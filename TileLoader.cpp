#define QT_NO_DEBUG_OUTPUT
/*
 * This file is part of the Marble Virtual Globe.
 *
 * Copyright 2010 Jens-Michael Hoffmann <jmho@c-xx.com>
 * Copyright 2010-2012 Bernhard Beschow <bbeschow@cs.tu-berlin.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#include "TileLoader.h"

#include <QDateTime>
#include <QFileInfo>
#include <QMetaType>
#include <QImage>

#include "geodata/scene/GeoSceneTextureTileDataset.h"
#include "geodata/scene/GeoSceneTileDataset.h"
#include "geodata/parser/GeoSceneTypes.h"
#include "geodata/scene/GeoSceneVectorTileDataset.h"
#include "geodata/data/GeoDataDocument.h"
#include "geodata/data/GeoDataContainer.h"
#include "HttpDownloadManager.h"
#include "MarbleDebug.h"
#include "MarbleDirs.h"
#include "TileLoaderHelper.h"
#include "ParseRunnerPlugin.h"
#include "ParsingRunner.h"

Q_DECLARE_METATYPE( Marble::DownloadUsage )

namespace Marble
{

TileLoader::TileLoader(HttpDownloadManager * const downloadManager, const PluginManager *pluginManager) :
    m_pluginManager(pluginManager)
{
    qRegisterMetaType<DownloadUsage>( "DownloadUsage" );
    connect( this, SIGNAL(createTile(QUrl,QString,QString,DownloadUsage)),
             downloadManager, SLOT(addJob(QUrl,QString,QString,DownloadUsage)));
    connect( downloadManager, SIGNAL(downloadComplete(QString,QString)),
             SLOT(updateTile(QString,QString)));
    connect( downloadManager, SIGNAL(downloadComplete(QByteArray,QString)),
             SLOT(updateTile(QByteArray,QString)));
}

TileLoader::~TileLoader()
{
    // nothing to do
}

// If the tile image file is locally available:
//     - if not expired: create ImageTile, set state to "uptodate", return it => done
//     - if expired: create TextureTile, state is set to Expired by default, trigger dl,
QImage TileLoader::loadTileImage( GeoSceneTextureTileDataset const *textureLayer, TileId const & tileId, DownloadUsage const usage )
{
    QString const fileName = tileFileName( textureLayer, tileId );

    TileStatus status = tileStatus( textureLayer, tileId );
    if ( status != Missing ) {
        // check if an update should be triggered

        if ( status == Available ) {
            mDebug() << Q_FUNC_INFO << tileId << "StateUptodate";
        } else {
            Q_ASSERT( status == Expired );
            mDebug() << Q_FUNC_INFO << tileId << "StateExpired";
            triggerDownload( textureLayer, tileId, usage );
        }

        QImage const image( fileName);
        if ( !image.isNull() && image.size() == textureLayer->tileSize())
        {
            // file is there, so create and return a tile object in any case
            return image;
        }
    }

    // tile was not locally available => trigger download and look for tiles in other levels
    // for scaling

    QImage replacementTile = scaledLowerLevelTile( textureLayer, tileId );
    Q_ASSERT( !replacementTile.isNull() );

    triggerDownload( textureLayer, tileId, usage );

    return replacementTile;
}


GeoDataDocument *TileLoader::loadTileVectorData( GeoSceneVectorTileDataset const *textureLayer, TileId const & tileId, DownloadUsage const usage )
{
    // FIXME: textureLayer->fileFormat() could be used in the future for use just that parser, instead of all available parsers

    QString const fileName = tileFileName( textureLayer, tileId );

    TileStatus status = tileStatus( textureLayer, tileId );
    if ( status != Missing ) {
        // check if an update should be triggered

        if ( status == Available ) {
            mDebug() << Q_FUNC_INFO << tileId << "StateUptodate";
        } else {
            Q_ASSERT( status == Expired );
            mDebug() << Q_FUNC_INFO << tileId << "StateExpired";
            triggerDownload( textureLayer, tileId, usage );
        }

        QFile file ( fileName );
        if ( file.exists() ) {

            // File is ready, so parse and return the vector data in any case
            GeoDataDocument* document = openVectorFile(fileName);
            if (document) {
                return document;
            }
        }
    }

    // tile was not locally available => trigger download
    triggerDownload( textureLayer, tileId, usage );
    return nullptr;
}

// This method triggers a download of the given tile (without checking
// expiration). It is called by upper layer (StackedTileLoader) when the tile
// that should be reloaded is currently loaded in memory.
//
// post condition
//     - download is triggered
void TileLoader::createTile(GeoSceneTileDataset const *tileData, TileId const &tileId, DownloadUsage usage )
{
    triggerDownload( tileData, tileId, usage );
}

bool
TileLoader::baseTilesAvailable( GeoSceneTileDataset const & tileData )
{
    const int  levelZeroColumns = tileData.levelZeroColumns();
    const int  levelZeroRows    = tileData.levelZeroRows();

    bool result = true;

    // Check whether the tiles from the lowest texture level are available
    //
    for ( int column = 0; result && column < levelZeroColumns; ++column ) {
        for ( int row = 0; result && row < levelZeroRows; ++row ) {
            const TileId id( 0, 0, column, row );
            const QString tilepath = tileFileName( &tileData, id );
            result &= QFile::exists( tilepath );
            if (!result) {
                qWarning() << "Base tile " << tileData.relativeTileFileName( id ) << " is missing for source dir " << tileData.sourceDir();
            }
        }
    }

    return result;
}

TileLoader::TileStatus TileLoader::tileStatus( GeoSceneTileDataset const *tileData, const TileId &tileId )
{
    QString const fileName = tileFileName( tileData, tileId );
    QFileInfo fileInfo( fileName );
    if ( !fileInfo.exists() ) {
        return Missing;
    }

    QImage const image( fileName );
    if ( image.isNull() || image.size() != tileData->tileSize())
    {
        return Missing;
    }

    const QDateTime lastModified = fileInfo.lastModified();
    const int expireSecs = tileData->expire();
    const bool isExpired = lastModified.secsTo( QDateTime::currentDateTime() ) >= expireSecs;
    return isExpired ? Expired : Available;
}

void
TileLoader::updateTile( QByteArray const & data, QString const & idStr )
{
    QStringList const components = idStr.split( ':', QString::SkipEmptyParts );
    Q_ASSERT( components.size() == 5 );

    QString const origin = components[0];
    QString const sourceDir = components[ 1 ];
    int const zoomLevel = components[ 2 ].toInt();
    int const tileX = components[ 3 ].toInt();
    int const tileY = components[ 4 ].toInt();

    TileId const id = TileId( sourceDir, zoomLevel, tileX, tileY );

    if (origin == GeoSceneTypes::GeoSceneTextureTileType) {
        QImage const tileImage = QImage::fromData( data );
        if ( tileImage.isNull() )
            return;

        emit AbstractTileLoader::tileCompleted( id, tileImage );
    }
}

void TileLoader::updateTile(const QString &fileName, const QString &idStr)
{
    QStringList const components = idStr.split( ':', QString::SkipEmptyParts );
    Q_ASSERT( components.size() == 5 );

    QString const origin = components[0];
    QString const sourceDir = components[ 1 ];
    int const zoomLevel = components[ 2 ].toInt();
    int const tileX = components[ 3 ].toInt();
    int const tileY = components[ 4 ].toInt();

    TileId const id = TileId( sourceDir, zoomLevel, tileX, tileY );
    if (origin == GeoSceneTypes::GeoSceneVectorTileType) {
        GeoDataDocument* document = openVectorFile(MarbleDirs::path(fileName));
        if (document) {
            emit tileCompleted(id,  document);
        }
    }
}

QString TileLoader::tileFileName( GeoSceneTileDataset const * tileData, TileId const & tileId )
{
    QString const fileName = tileData->relativeTileFileName( tileId );
    QFileInfo const dirInfo( fileName );
    return dirInfo.isAbsolute() ? fileName : MarbleDirs::path( fileName );
}

void TileLoader::triggerDownload( GeoSceneTileDataset const *tileData, TileId const &id, DownloadUsage const usage )
{
    qDebug() << "TileLoader::triggerDownload";

    if (id.tileLevel() > 0) {
        int minValue = tileData->maximumTileLevel() == -1 ? id.tileLevel() : qMin( id.tileLevel(), tileData->maximumTileLevel() );
        if (id.tileLevel() != qMax(tileData->minimumTileLevel(), minValue) ) {
            // Download only level 0 tiles and tiles between minimum and maximum tile level
            return;
        }
    }

    QUrl const sourceUrl = tileData->downloadUrl( id );
    QString const destFileName = tileData->relativeTileFileName( id );
    QString const idStr = QStringLiteral( "%1:%2:%3:%4:%5" ).arg( tileData->nodeType()).arg( tileData->sourceDir() ).arg( id.tileLevel() ).arg( id.x() ).arg( id.y() );

    qDebug() << "TileLoader::triggerDownload id" << sourceUrl<< destFileName << idStr;

    emit createTile( sourceUrl, destFileName, idStr, usage );
}

GeoDataDocument *TileLoader::openVectorFile(const QString &fileName) const
{
    QList<const ParseRunnerPlugin*> plugins = m_pluginManager->parsingRunnerPlugins();
    const QFileInfo fileInfo( fileName );
    const QString suffix = fileInfo.suffix().toLower();
    const QString completeSuffix = fileInfo.completeSuffix().toLower();

    foreach( const ParseRunnerPlugin *plugin, plugins ) {
        QStringList const extensions = plugin->fileExtensions();
        if ( extensions.contains( suffix ) || extensions.contains( completeSuffix ) ) {
            ParsingRunner* runner = plugin->newRunner();
            QString error;
            GeoDataDocument* document = runner->parseFile(fileName, UserDocument, error);
            if (!document && !error.isEmpty()) {
                mDebug() << QStringLiteral("Failed to open vector tile %1: %2").arg(fileName).arg(error);
            }
            delete runner;
            return document;
        }
    }

    mDebug() << "Unable to open vector tile " << fileName << ": No suitable plugin registered to parse this file format";
    return nullptr;
}

}

//#include "moc_TileLoader.cpp"
