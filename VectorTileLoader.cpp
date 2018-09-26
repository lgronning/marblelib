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

#include "VectorTileLoader.h"
#include "TextureTile.h"

#include <QtCore/QDateTime>
#include <QtCore/QFileInfo>
#include <QtCore/QMetaType>
#include <QtCore/QtMath>
#include <QtCore/QRunnable>
#include <QtCore/QThreadPool>
#include <QtCore/QFuture>
#include <QtCore/QMetaType>
#include <QtCore/QElapsedTimer>
#include <QtConcurrent/QtConcurrentRun>

#include <QtGui/QImage>
#include <QtGui/QGuiApplication>
#include <QtGui/QOpenGLPaintDevice>
#include <QtGui/QOpenGLFramebufferObjectFormat>
#include <QtGui/QWindow>

#include "geodata/scene/GeoSceneTextureTileDataset.h"
#include "geodata/scene/GeoSceneTileDataset.h"
#include "geodata/parser/GeoSceneTypes.h"
#include "geodata/scene/GeoSceneVectorTileDataset.h"
#include "geodata/data/GeoDataDocument.h"
#include "geodata/data/GeoDataContainer.h"
#include "HttpDownloadManager.h"
#include "MarbleDebug.h"
#include "MarbleDirs.h"
#include "MarbleMath.h"
#include "TileLoaderHelper.h"
#include "ParseRunnerPlugin.h"
#include "ParsingRunner.h"
#include "ParsingRunner.h"
#include "GeoGraphicsScene.h"
#include "GeoPainter.h"
#include "ViewportParams.h"
#include "graphicsview/GeoGraphicsItem.h"

Q_DECLARE_METATYPE( Marble::DownloadUsage )

static qint64 MaxTileBackingStoreSize = 30 * 1024 * 1024;

namespace
{


QImage
getEmptyImage(const QSize &size, double pixelRatio)
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);

    static QHash<int, QHash<int, QHash<double, QImage> > > imageCache;

    if(!imageCache.contains(size.width()) || !imageCache[size.width()].contains(size.height()) || !imageCache[size.width()][size.height()].contains(pixelRatio))
    {
        QImage image = QImage( size * pixelRatio , QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        image.setDevicePixelRatio( pixelRatio );

        imageCache[size.width()][size.height()].insert(pixelRatio, image);
    }

    return imageCache[size.width()][size.height()][pixelRatio];
}

}
namespace Marble
{

class GeometryTile : public Tile
{
 public:
    GeometryTile(Marble::TileId const & tileId, QImage const & image )
        :   Tile(tileId),
            aImage(image),
            aIsExpired(false),
            aIsUsed(false)
    {

    }

    
    ~GeometryTile() override;

    const QImage &
    image() const
    {
        return aImage;
    }

    int
    byteCount() const
    {
        return aImage.byteCount();
    }

    bool
    isExpired() const
    {
        return aIsExpired;
    }

    void
    setIsExpired(bool expired)
    {
        aIsExpired = expired;
    }

    void
    setUsed( bool used )
    {
        aIsUsed = used;
    }

    bool
    used() const
    {
        return aIsUsed;
    }


 private:
    QImage const aImage;
    bool aIsExpired;
    bool aIsUsed;
};

GeometryTile::~GeometryTile()
= default;


RenderJob::RenderJob(Projection projection, qreal centerLongitude, qreal centerLatitude, int radius,  const QSize &size, QList< GeoGraphicsItemPtr > items , const TileId &tileId, QString tileDir)
    :   aCancel(false),
        aProjection(projection),
        aCenterLongitude(centerLongitude),
        aCenterLatitude(centerLatitude),
        aRadius(radius),
        aSize(size),
        aItems(std::move(items)),
        aTileId(tileId),
        aTileDir(std::move(tileDir))
{
}

RenderJob::~RenderJob()
{
    qDebug() << "Deleted Render Job";
}

void RenderJob::run()
{
    QTime timer;
    timer.start();

    qDebug() << "VectorTileLoader::RenderJob::run" << aItems.size();
    qreal pixelRatio = 1.0;

    if ( qApp )
        pixelRatio = qApp->devicePixelRatio();

    MapQuality mapQuality = MapQuality::HighQuality;
    const double ScaleFactor = 1; //(mapQuality == MapQuality::HighQuality || mapQuality == MapQuality::PrintQuality) ? 1.5 : 1;

    ViewportParams viewport(aProjection, aCenterLongitude, aCenterLatitude, qFloor(aRadius), aSize);

    QImage pm = QImage( aSize * pixelRatio  * ScaleFactor, QImage::Format_ARGB32_Premultiplied);
    pm.fill(Qt::transparent);
    pm.setDevicePixelRatio( pixelRatio );

    GeoPainter tempGeoPainter(&pm, &viewport, MapQuality::HighQuality);
//    tempGeoPainter.setClipRect(QRect(QPoint(0,0), pm.size()+QSize(1,1)));
    tempGeoPainter.scale(ScaleFactor, ScaleFactor);

    qDebug() << "VectorTileLoader::RenderJob::run before render" << timer.elapsed();

    foreach( const GeoGraphicsItemPtr& item, aItems )
    {
        if(aCancel)
        {
            return;
        }

        MapQuality tempMapQuality = mapQuality;
        if((tempMapQuality == MapQuality::HighQuality || tempMapQuality == MapQuality::PrintQuality) && item->style() && !item->style()->useAntialising())
        {
            tempMapQuality = MapQuality::LowQuality;
        }

        tempGeoPainter.setMapQuality(tempMapQuality);

        item->renderGeometry(&tempGeoPainter, &viewport, item->style() );
    }


    qDebug() << "VectorTileLoader::RenderJob::run after render" << timer.elapsed();

    if(qFuzzyCompare(ScaleFactor, 1.0))
    {
        aTile = pm;
    }
    else
    {
        aTile = pm.scaled(aSize * pixelRatio, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    if(QDir(aTileDir).exists())
    {
        bool ok = aTile.save(aTileDir + QString("/tile_%1_%2_%3.png").arg(aTileId.tileLevel()).arg(aTileId.x()).arg(aTileId.y()), "PNG");
        qDebug() << "VectorTileLoader::RenderJob::run file saved" << ok << aTileDir << aTileDir + QString("/tile_%1_%2_%3.png").arg(aTileId.tileLevel()).arg(aTileId.x()).arg(aTileId.y());
    }


    qDebug() << "VectorTileLoader::RenderJob::run finished" << timer.elapsed();
}


VectorTileLoader::VectorTileLoader(GeoGraphicsScene *scene)
    :   m_scene(scene),
        m_cacheSize(0),
        dir(new QTemporaryDir()),
        mutex(new QMutex(QMutex::Recursive))
{
    qRegisterMetaType<TileId>( "TileId" );
    connect(this, SIGNAL(startRenderTile(GeoSceneTileDataset  const *, TileId)), SLOT(renderTile(GeoSceneTileDataset const *, TileId)), Qt::QueuedConnection);
}

VectorTileLoader::~VectorTileLoader()
{
    qDeleteAll( m_tilesOnDisplay );
    qDeleteAll( m_tileCache );
    delete mutex;
    delete dir;
}

// If the tile image file is locally available:
//     - if not expired: create ImageTile, set state to "uptodate", return it => done
//     - if expired: create GeometryTile, state is set to Expired by default, trigger dl,
QImage
VectorTileLoader::loadTileImage( GeoSceneTextureTileDataset const *textureLayer, TileId const & tileId, DownloadUsage const usage )
{
    QMutexLocker locker(mutex);

    TileStatus status = tileStatus( textureLayer, tileId );
    if ( status != Missing ) {
        // check if an update should be triggered

        if ( status == Available )
        {
            mDebug() << Q_FUNC_INFO << tileId << "StateUptodate";
        }
        else
        {
            Q_ASSERT( status == Expired );
            mDebug() << Q_FUNC_INFO << tileId << "StateExpired";

            if(!tileRenderJobMap.contains(tileId))
            {
                triggerCreate( textureLayer, tileId, usage );
            }
        }

        GeometryTile * stackedTile = m_tilesOnDisplay.value( tileId, nullptr );
        if ( stackedTile )
        {
            stackedTile->setUsed( true );
            return stackedTile->image();
        }

        stackedTile = m_tileCache.take( tileId );
        if(!stackedTile && m_emptyTiles.contains(tileId))
        {
            qreal pixelRatio = 1.0;

            if ( qApp )
                pixelRatio = qApp->devicePixelRatio();

            stackedTile = new GeometryTile(tileId, getEmptyImage(textureLayer->tileSize(), pixelRatio));
        }

        if ( stackedTile ) {
            stackedTile->setUsed( true );
            m_cacheSize -= stackedTile->byteCount();
            m_cacheTiles.removeOne(tileId);
            m_tilesOnDisplay[ tileId ] = stackedTile;
            return stackedTile->image();
        }

        QString tileFileName = dir->path() + QString("/tile_%1_%2_%3.png").arg(tileId.tileLevel()).arg(tileId.x()).arg(tileId.y());
        if(QFileInfo::exists(tileFileName))
        {
            QFile file;
            QImage image = QImage(tileFileName, "PNG");
            if(!image.isNull() && image.size() == textureLayer->tileSize())
            {
                image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);

                auto stackedTile = new GeometryTile(tileId, image);

                stackedTile->setUsed(true);
                m_tilesOnDisplay[ tileId ] = stackedTile;
                return stackedTile->image();
            }
        }
    }

    if(!tileRenderJobMap.contains(tileId))
    {
        triggerCreate( textureLayer, tileId, usage );
    }

    QImage replacementTile = scaledLowerLevelTile( textureLayer, tileId );
    if(!replacementTile.isNull())
    {
        return replacementTile;
    }

    QSize tileSize = textureLayer->tileSize();
    QImage emptyImage = QImage( tileSize, QImage::Format_ARGB32_Premultiplied );
    emptyImage.fill(Qt::transparent );

    return emptyImage;

}

// This method triggers a download of the given tile (without checking
// expiration). It is called by upper layer (StackedTileLoader) when the tile
// that should be reloaded is currently loaded in memory.
//
// post condition
//     - download is triggered
void VectorTileLoader::createTile(GeoSceneTileDataset const *tileData, TileId const &tileId, DownloadUsage usage )
{
    QMutexLocker locker(mutex);

    triggerCreate( tileData, tileId, usage );
}


VectorTileLoader::TileStatus VectorTileLoader::tileStatus( GeoSceneTileDataset const *tileData, const TileId &tileId )
{
    QMutexLocker locker(mutex);

    if(!m_tilesOnDisplay.contains(tileId) && !m_tileCache.contains(tileId) && !m_emptyTiles.contains(tileId))
    {
        if(QFileInfo::exists(dir->path() + QString("/tile_%1_%2_%3.png").arg(tileId.tileLevel()).arg(tileId.x()).arg(tileId.y())))
        {
            return Available;
        }

        return Missing;
    }

    if(m_emptyTiles.contains(tileId))
    {
        return Available;
    }

    GeometryTile * stackedTile = m_tilesOnDisplay.value( tileId, nullptr );
    if ( !stackedTile )
    {
        stackedTile = m_tileCache.value( tileId );
    }

    if(stackedTile && stackedTile->isExpired())
    {
        return Expired;
    }

    return Available;
}

void VectorTileLoader::clear()
{
    QMutexLocker locker(mutex);

    qDeleteAll(tileRenderJobMap);
    tileRenderJobMap.clear();

    qDeleteAll(m_tilesOnDisplay);
    m_tilesOnDisplay.clear();

    qDeleteAll(m_tileCache);
    m_tileCache.clear();
    m_cacheTiles.clear();
    m_emptyTiles.clear();

    m_cacheSize = 0;

    delete dir;
    dir = new QTemporaryDir();
}

void VectorTileLoader::resetTilehash()
{
    QMutexLocker locker(mutex);

    QHash<TileId, GeometryTile*>::const_iterator it = m_tilesOnDisplay.constBegin();
    QHash<TileId, GeometryTile*>::const_iterator const end = m_tilesOnDisplay.constEnd();
    for (; it != end; ++it )
    {
        it.value()->setUsed( false );
    }
}

void VectorTileLoader::setTilesUsed(const QList<TileId> &tiles)
{
    QMutexLocker locker(mutex);

    foreach(const TileId &tileId, tiles)
    {
        GeometryTile * stackedTile = m_tilesOnDisplay.value( tileId, nullptr );
        if ( stackedTile )
        {
            stackedTile->setUsed( true );
        }

        stackedTile = m_tileCache.take( tileId);
        if ( stackedTile )
        {
            stackedTile->setUsed( true );
            m_cacheSize -= stackedTile->byteCount();
            m_cacheTiles.removeOne(tileId);

            m_tilesOnDisplay[ tileId ] = stackedTile;
        }
    }
}

void VectorTileLoader::cleanupTilehash()
{
    QMutexLocker locker(mutex);

    // Make sure that tiles which haven't been used during the last
    // rendering of the map at all get removed from the tile hash.

    QHashIterator<TileId, GeometryTile*> it( m_tilesOnDisplay );
    while ( it.hasNext() )
    {
        it.next();
        if ( !it.value()->used() )
        {
            // If insert call result is false then the cache is too small to store the tile
            // but the item will get deleted nevertheless and the pointer we have
            // doesn't get set to zero (so don't delete it in this case or it will crash!)
            m_tileCache.insert( it.key(), it.value() );
            m_cacheSize += it.value()->byteCount();
            m_cacheTiles.enqueue(it.key());

            m_tilesOnDisplay.remove( it.key() );
        }
    }
}

void VectorTileLoader::handleFinished(const TileId &tileId, const QImage &tile)
{
    QMutexLocker locker(mutex);

    qDebug() << "VectorTileLoader::handleFinished";
    if(m_tileCache.contains(tileId))
    {
        GeometryTile *geometryTile= m_tileCache.value(tileId);
        m_cacheSize -= geometryTile->byteCount();
        m_tileCache.remove(tileId);
        m_cacheTiles.removeOne(tileId);

        delete geometryTile;
    }

    if(m_tilesOnDisplay.contains(tileId))
    {
        delete m_tilesOnDisplay.value(tileId);
        m_tilesOnDisplay.remove(tileId);
    }

    if(tileRenderJobMap.contains(tileId))
    {
        bool isCancelled = tileRenderJobMap[tileId]->isCancelled();
        tileRenderJobMap[tileId]->deleteLater();
        tileRenderJobMap.remove(tileId);

        if(isCancelled)
        {
            qWarning() << "tile render job was cancelled" << tileId;
            return;
        }
    }
    else
    {
        qDebug() << "Not in tile render job map" << tileId;
    }

    GeometryTile *geometryTile = new GeometryTile(tileId, tile);

    while(m_cacheSize+geometryTile->byteCount() > MaxTileBackingStoreSize && !m_tileCache.isEmpty())
    {
        TileId tileId = m_cacheTiles.dequeue();

        int tempByteCount = m_tileCache[tileId]->byteCount();

        delete m_tileCache[tileId];
        m_tileCache.remove(tileId);
        m_cacheSize -= tempByteCount;
    }


    m_cacheSize += geometryTile->byteCount();
    m_tileCache.insert(tileId, geometryTile);
    m_cacheTiles.enqueue(tileId);

    emit tileCompleted(tileId, tile);
}

void VectorTileLoader::renderTile(const GeoSceneTileDataset *tileData, const TileId &tileId)
{
    QMutexLocker locker(mutex);

    if(tileRenderJobMap.contains(tileId))
    {
        return;
    }

    qDebug() << "VectorTileLoader::renderTile" << tileId.tileLevel() << tileId.x() << tileId.y();

    qreal count = ( 1 << tileId.tileLevel() ) * tileData->levelZeroColumns() / 2.0;
    qreal lon   = ( tileId.x() - count + 0.5) / count * M_PI;

    count = ( 1 << tileId.tileLevel() ) * tileData->levelZeroRows() / 2.0;
    double lat = 0;
    switch ( tileData->projection() ) {
    case GeoSceneTileDataset::Equirectangular:
        lat = count - tileId.y() - 0.5 / count * M_PI / 2.0;
        break;
    case GeoSceneTileDataset::Mercator:
        lat = atan( sinh( ( count - tileId.y() - 0.5 ) / count * M_PI ) );
        break;
    }

    QList< GeoGraphicsItemPtr > items = m_scene->items(tileData, tileId);

    if(items.isEmpty())
    {
        qreal pixelRatio = 1.0;

        if ( qApp )
            pixelRatio = qApp->devicePixelRatio();
        QImage emptyImage = getEmptyImage(tileData->tileSize(), pixelRatio);

        m_emptyTiles.insert(tileId);

        handleFinished(tileId, emptyImage);
    }
    else
    {
        Projection projection = Projection::Mercator;
        if(tileData->projection() == GeoSceneTileDataset::Equirectangular)
        {
            projection = Projection::Equirectangular;
        }

        // choose the smaller dimension for selecting the tile level, leading to higher-resolution results
        const int levelZeroWidth = tileData->tileSize().width() * tileData->levelZeroColumns();
        const int levelZeroHight = tileData->tileSize().height() * tileData->levelZeroRows();
        const int levelZeroMinDimension = qMin( levelZeroWidth, levelZeroHight );

        double tileLevelF = tileId.tileLevel();
        double linearLevel = qExp(tileLevelF * qLn( 2.0 ) );

        if ( linearLevel < 1.0 )
            linearLevel = 1.0; // Dirty fix for invalid entry linearLevel

        qreal radius =  (static_cast<double>( levelZeroMinDimension ) * linearLevel) /4.0 ;

        qDebug() << "Radius" << radius;


        TileRenderJob *job = new TileRenderJob(projection, lon, lat, qRound(radius), tileData->tileSize(), items, tileId, dir->path(), this);
        tileRenderJobMap.insert(tileId, job);

        connect(job, SIGNAL(finished(const TileId &, const QImage &)), this, SLOT(handleFinished(const TileId &, const QImage &)));
        job->run();
    }

}

bool
checkTileId(const TileId& tileId, const GeoSceneTileDataset *tileData, TileMap tileMap)
{
    GeoDataLatLonBox box = tileId.toLatLonBox(tileData);
    qreal north, south, east, west;
    box.boundaries( north, south, east, west );

    auto itZLevel = tileMap.begin();
    auto itZLevelEnd = tileMap.end();

    for(; itZLevel != itZLevelEnd; ++itZLevel)
    {
        int tempTileLevel = itZLevel.key();
        TileId topLeftKey = TileId::fromCoordinates(tileData, GeoDataCoordinates(west, north, 0), tempTileLevel );
        TileId bottomRightKey = TileId::fromCoordinates(tileData, GeoDataCoordinates(east, south, 0), tempTileLevel );

        QRect rect(topLeftKey.x(), topLeftKey.y(), bottomRightKey.x() - topLeftKey.x() + 1, bottomRightKey.y() - topLeftKey.y() + 1);

        const auto & hash = itZLevel.value();

        auto itX = hash.lowerBound(rect.left());
        auto itEnd = hash.constEnd();

        for(; itX != itEnd && itX.key() <= rect.right(); ++itX)
        {
            if(itX.key() < rect.left())
            {
                continue;
            }


            auto itY = itX.value().lowerBound(rect.top());
            auto itYEnd = itX.value().constEnd();

            for(; itY != itYEnd && itY.key() <= rect.bottom(); ++itY)
            {
                if(itY.key() < rect.top())
                {
                    continue;
                }

                return true;
            }
        }
    }

    return false;
}

void
VectorTileLoader::setTileExpired(const GeoSceneTileDataset *tileData, TileMap tileMap)
{
    QMutexLocker locker(mutex);

    QMutableMapIterator<TileId, TileRenderJob*> iRenderJob(tileRenderJobMap);
    while (iRenderJob.hasNext())
    {
        iRenderJob.next();
        TileId tileId = iRenderJob.key();

        bool found = checkTileId(tileId, tileData, tileMap);

        if(found)
        {
            delete iRenderJob.value();
            iRenderJob.remove();
        }
    }

    QMutableHashIterator<TileId, GeometryTile*> iTileCache(m_tileCache);
    while (iTileCache.hasNext())
    {
        iTileCache.next();

        TileId tileId = iTileCache.key();

        bool found = checkTileId(tileId, tileData, tileMap);

        if(found)
        {
            m_cacheSize -= iTileCache.value()->byteCount();
            m_cacheTiles.removeOne(tileId);

            delete iTileCache.value();
            iTileCache.remove();
        }
    }

    QMutableSetIterator<TileId> iTileEmptyCache(m_emptyTiles);
    while (iTileEmptyCache.hasNext())
    {
        iTileEmptyCache.next();

        TileId tileId = iTileEmptyCache.value();

        bool found = checkTileId(tileId, tileData, tileMap);

        if(found)
        {
            iTileEmptyCache.remove();
        }
    }

    foreach(const QString &fileName, QDir(dir->path()).entryList(QStringList("tile_*"), QDir::Files | QDir::NoSymLinks))
    {
        QStringList values = fileName.split("_");
        if(values.size() != 4)
        {
            continue;
        }

        TileId tileId(0, values[1].toInt(), values[2].toInt(), values[3].remove(".png").toInt());
        bool found = checkTileId(tileId, tileData, tileMap);

        if(found)
        {
            QFile::remove(dir->path() + "/" + fileName);
        }

    }

    QHashIterator<TileId, GeometryTile*> iTilesOnDisplay(m_tilesOnDisplay);
    while (iTilesOnDisplay.hasNext())
    {
        iTilesOnDisplay.next();
        TileId tileId = iTilesOnDisplay.key();

        bool found = checkTileId(tileId, tileData, tileMap);

        if(found)
        {
            iTilesOnDisplay.value()->setIsExpired(true);

            triggerCreate(tileData, tileId, DownloadUsage::DownloadBulk);
        }
    }



}

void VectorTileLoader::triggerCreate( GeoSceneTileDataset const *tileData, TileId const &id, DownloadUsage const usage )
{
    qDebug() << "VectorTileLoader::triggerDownload";

    emit startRenderTile(tileData, id);

}

TileRenderJob::TileRenderJob(Projection projection, qreal centerLongitude, qreal centerLatitude, int radius, const QSize &size, QList<GeoGraphicsItemPtr> items, const TileId &tileId, const QString &tileDir, QObject *parent)
    :   QObject(parent),
        job(new RenderJob(projection, centerLongitude, centerLatitude, radius, size, items, tileId, tileDir))
{
    qDebug() << "TileRenderJob::TileRenderJob" << job << this;

    connect(&watcher, SIGNAL(finished()), this, SLOT(handleFinished()));
}

TileRenderJob::~TileRenderJob()
{
    qDebug() << "TileRenderJob::~TileRenderJob" << job << job->isCancelled() << watcher.isCanceled() << watcher.isFinished() << watcher.isRunning() << watcher.isStarted() << this;

    disconnect(&watcher, SIGNAL(finished()), this, SLOT(handleFinished()));

    job->cancel();
    watcher.cancel();
    watcher.waitForFinished();

    delete job;

    qDebug() << "TileRenderJob::~TileRenderJob finished" << job << this;
}

void TileRenderJob::run()
{
    qDebug() << "TileRenderJob::run" << job << this;

    QFuture<void> future = QtConcurrent::run(job, &RenderJob::run);
    watcher.setFuture(future);
}

void TileRenderJob::handleFinished()
{
    qDebug() << "TileRenderJob::TileRenderJob::handleFinished" << job << job->tileId() << job->isCancelled();
    if(!job->isCancelled())
    {
        emit finished(job->tileId(), job->tile());
    }

}


} // namespace




