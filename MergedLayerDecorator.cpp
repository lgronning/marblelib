#define QT_NO_DEBUG_OUTPUT
// Copyright 2008 David Roberts <dvdr18@gmail.com>
// Copyright 2009 Jens-Michael Hoffmann <jensmh@gmx.de>
// Copyright 2011 Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library.  If not, see <http://www.gnu.org/licenses/>.


#include "MergedLayerDecorator.h"

#include "blendings/Blending.h"
#include "blendings/BlendingFactory.h"
#include "SunLocator.h"
#include "MarbleMath.h"
#include "MarbleDebug.h"
#include "geodata/scene/GeoSceneTextureTileDataset.h"
#include "ImageF.h"
#include "StackedTile.h"
#include "TileLoaderHelper.h"
#include "TextureTile.h"
#include "AbstractTileLoader.h"
#include "VectorTileLoader.h"

#include "geodata/data/GeoDataCoordinates.h"

#include <QMutexLocker>
#include <QPointer>
#include <QtGui/QPainter>

using namespace Marble;

class Q_DECL_HIDDEN MergedLayerDecorator::Private
{
public:
    Private( AbstractTileLoader *tileLoader, const SunLocator *sunLocator );

    static int maxDivisor( int maximum, int fullLength );

    StackedTile *createTile( const QVector<QSharedPointer<TextureTile> > &tiles ) const;

    void paintSunShading( QImage *tileImage, const TileId &id ) const;
    void paintTileId( QImage *tileImage, const TileId &id ) const;

    void detectMaxTileLevel();
    QVector<const GeoSceneTextureTileDataset *> findRelevantTextureLayers( const TileId &stackedTileId ) const;

    AbstractTileLoader *const m_tileLoader;
    const SunLocator *const m_sunLocator;
    BlendingFactory m_blendingFactory;
    QVector<const GeoSceneTextureTileDataset *> m_textureLayers;
    int m_maxTileLevel;
    QString m_themeId;
    int m_levelZeroColumns;
    int m_levelZeroRows;
    bool m_showSunShading;
    bool m_showCityLights;
    bool m_showTileId;
};

MergedLayerDecorator::Private::Private(AbstractTileLoader *tileLoader, const SunLocator *sunLocator ) :
    m_tileLoader( tileLoader ),
    m_sunLocator( sunLocator ),
    m_blendingFactory( sunLocator ),
    m_textureLayers(),
    m_maxTileLevel( 0 ),
    m_themeId(),
    m_levelZeroColumns( 0 ),
    m_levelZeroRows( 0 ),
    m_showSunShading( false ),
    m_showCityLights( false ),
    m_showTileId( false )
{
}

MergedLayerDecorator::MergedLayerDecorator(AbstractTileLoader * const tileLoader,
                                            const SunLocator* sunLocator )
    : d( new Private( tileLoader, sunLocator ) )
{
}

MergedLayerDecorator::~MergedLayerDecorator()
{
    delete d;
}

void MergedLayerDecorator::setTextureLayers( const QVector<const GeoSceneTextureTileDataset *> &textureLayers )
{
    mDebug() << Q_FUNC_INFO;

    if ( textureLayers.count() > 0 ) {
        const GeoSceneTileDataset *const firstTexture = textureLayers.at( 0 );
        d->m_levelZeroColumns = firstTexture->levelZeroColumns();
        d->m_levelZeroRows = firstTexture->levelZeroRows();
        d->m_blendingFactory.setLevelZeroLayout( d->m_levelZeroColumns, d->m_levelZeroRows );
        d->m_themeId = "maps/" + firstTexture->sourceDir();
    }

    d->m_textureLayers = textureLayers;

    d->detectMaxTileLevel();
}

const QVector<const GeoSceneTextureTileDataset *> &
MergedLayerDecorator::textureLayers()
{
    return d->m_textureLayers;
}

int MergedLayerDecorator::textureLayersSize() const
{
    return d->m_textureLayers.size();
}

int MergedLayerDecorator::maximumTileLevel() const
{
    return d->m_maxTileLevel;
}

int MergedLayerDecorator::tileColumnCount( int level ) const
{
    Q_ASSERT( !d->m_textureLayers.isEmpty() );

    const int levelZeroColumns = d->m_textureLayers.at( 0 )->levelZeroColumns();

    return TileLoaderHelper::levelToColumn( levelZeroColumns, level );
}

int MergedLayerDecorator::tileRowCount( int level ) const
{
    Q_ASSERT( !d->m_textureLayers.isEmpty() );

    const int levelZeroRows = d->m_textureLayers.at( 0 )->levelZeroRows();

    return TileLoaderHelper::levelToRow( levelZeroRows, level );
}

GeoSceneTileDataset::Projection MergedLayerDecorator::tileProjection() const
{
    Q_ASSERT( !d->m_textureLayers.isEmpty() );

    return d->m_textureLayers.at( 0 )->projection();
}

QSize MergedLayerDecorator::tileSize() const
{
    Q_ASSERT( !d->m_textureLayers.isEmpty() );

    return d->m_textureLayers.at( 0 )->tileSize();
}

StackedTile *MergedLayerDecorator::Private::createTile( const QVector<QSharedPointer<TextureTile> > &tiles ) const
{
    Q_ASSERT( !tiles.isEmpty() );

    const TileId firstId = tiles.first()->id();
    const TileId id( 0, firstId.tileLevel(), firstId.x(), firstId.y() );

    // Image for blending all the texture tiles on it
    QImage resultImage;

    // if there are more than one active texture layers, we have to convert the
    // result tile into QImage::Format_ARGB32_Premultiplied to make blending possible
    const bool withConversion = tiles.count() > 1 || m_showSunShading || m_showTileId;
    foreach ( const QSharedPointer<TextureTile> &tile, tiles ) {

        // Image blending. If there are several images in the same tile (like clouds
        // or hillshading images over the map) blend them all into only one image

        const Blending *const blending =  tile->blending();
        if ( blending ) {

            mDebug() << Q_FUNC_INFO << "blending";

            if ( resultImage.isNull() ) {
                resultImage = QImage( tile->image().size(), QImage::Format_ARGB32_Premultiplied );
                resultImage.fill(Qt::transparent);
            }

            blending->blend( &resultImage, tile.data() );
        }
        else {
            mDebug() << Q_FUNC_INFO << "no blending defined => copying top over bottom image";
            if ( withConversion ) {
                resultImage = tile->image().convertToFormat( QImage::Format_ARGB32_Premultiplied );
            } else {
                resultImage = tile->image();
            }
        }
    }

    if ( m_showSunShading && !m_showCityLights ) {
        paintSunShading( &resultImage, id );
    }

    if ( m_showTileId ) {
        paintTileId( &resultImage, id );
    }

    return new StackedTile( id, resultImage, tiles );
}

StackedTile *MergedLayerDecorator::loadTile( const TileId &stackedTileId )
{
    const QVector<const GeoSceneTextureTileDataset *> textureLayers = d->findRelevantTextureLayers( stackedTileId );
    QVector<QSharedPointer<TextureTile> > tiles;

    foreach ( const GeoSceneTextureTileDataset *layer, textureLayers ) {
        const TileId tileId( layer->sourceDir(), stackedTileId.tileLevel(),
                             stackedTileId.x(), stackedTileId.y() );

        mDebug() << Q_FUNC_INFO << layer->sourceDir() << tileId << layer->tileSize() << layer->fileFormat();

        // Blending (how to merge the images into an only image)
        const Blending *blending = d->m_blendingFactory.findBlending( layer->blending() );
        if ( blending == nullptr && !layer->blending().isEmpty() ) {
            mDebug() << Q_FUNC_INFO << "could not find blending" << layer->blending();
        }

        const GeoSceneTextureTileDataset *const textureLayer = static_cast<const GeoSceneTextureTileDataset *>( layer );
        const QImage tileImage = d->m_tileLoader->loadTileImage( textureLayer, tileId, DownloadBrowse );

        QSharedPointer<TextureTile> tile( new TextureTile( tileId, tileImage, blending ) );
        tiles.append( tile );
    }

    Q_ASSERT( !tiles.isEmpty() );

    return d->createTile( tiles );
}

bool MergedLayerDecorator::hasTextureLayer() const
{
    return !d->m_textureLayers.isEmpty();
}

StackedTile *MergedLayerDecorator::updateTile( const StackedTile &stackedTile, const TileId &tileId, const QImage &tileImage )
{
    Q_ASSERT( !tileImage.isNull() );

    d->detectMaxTileLevel();

    QVector<QSharedPointer<TextureTile> > tiles = stackedTile.tiles();

    for ( int i = 0; i < tiles.count(); ++ i) {
        if ( tiles[i]->id() == tileId ) {
            const Blending *blending = tiles[i]->blending();

            tiles[i] = QSharedPointer<TextureTile>( new TextureTile( tileId, tileImage, blending ) );
        }
    }

    return d->createTile( tiles );
}

void MergedLayerDecorator::downloadStackedTile( const TileId &id, DownloadUsage usage )
{
    const QVector<const GeoSceneTextureTileDataset *> textureLayers = d->findRelevantTextureLayers( id );

    foreach ( const GeoSceneTextureTileDataset *textureLayer, textureLayers ) {
        if ( d->m_tileLoader->tileStatus( textureLayer, id ) != AbstractTileLoader::Available || usage == DownloadBrowse ) {
            d->m_tileLoader->createTile( textureLayer, id, usage );
        }
    }
}

void MergedLayerDecorator::setShowSunShading( bool show )
{
    d->m_showSunShading = show;
}

bool MergedLayerDecorator::showSunShading() const
{
    return d->m_showSunShading;
}

void MergedLayerDecorator::setShowCityLights( bool show )
{
    d->m_showCityLights = show;
}

bool MergedLayerDecorator::showCityLights() const
{
    return d->m_showCityLights;
}

void MergedLayerDecorator::setShowTileId( bool visible )
{
    d->m_showTileId = visible;
}

void MergedLayerDecorator::Private::paintSunShading( QImage *tileImage, const TileId &id ) const
{
    if ( tileImage->depth() != 32 )
        return;

    // TODO add support for 8-bit maps?
    // add sun shading
    const qreal  global_width  = tileImage->width()
            * TileLoaderHelper::levelToColumn( m_levelZeroColumns, id.tileLevel() );
    const qreal  global_height = tileImage->height()
            * TileLoaderHelper::levelToRow( m_levelZeroRows, id.tileLevel() );
    const qreal lon_scale = 2*M_PI / global_width;
    const qreal lat_scale = -M_PI / global_height;
    const int tileHeight = tileImage->height();
    const int tileWidth = tileImage->width();

    // First we determine the supporting point interval for the interpolation.
    const int n = maxDivisor( 30, tileWidth );
    const int ipRight = n * (int)( tileWidth / n );

    for ( int cur_y = 0; cur_y < tileHeight; ++cur_y ) {
        const qreal lat = lat_scale * ( id.y() * tileHeight + cur_y ) - 0.5*M_PI;
        const qreal a = sin( (lat+DEG2RAD * m_sunLocator->getLat() )/2.0 );
        const qreal c = cos(lat)*cos( -DEG2RAD * m_sunLocator->getLat() );

        QRgb* scanline = (QRgb*)tileImage->scanLine( cur_y );

        qreal lastShade = -10.0;

        int cur_x = 0;

        while ( cur_x < tileWidth ) {

            const bool interpolate = ( cur_x != 0 && cur_x < ipRight && cur_x + n < tileWidth );

            qreal shade = 0;

            if ( interpolate ) {
                const int check = cur_x + n;
                const qreal checklon   = lon_scale * ( id.x() * tileWidth + check );
                shade = m_sunLocator->shading( checklon, a, c );

                // if the shading didn't change across the interpolation
                // interval move on and don't change anything.
                if ( shade == lastShade && shade == 1.0 ) {
                    scanline += n;
                    cur_x += n;
                    continue;
                }
                if ( shade == lastShade && shade == 0.0 ) {
                    for ( int t = 0; t < n; ++t ) {
                        m_sunLocator->shadePixel( *scanline, shade );
                        ++scanline;
                    }
                    cur_x += n;
                    continue;
                }
                for ( int t = 0; t < n ; ++t ) {
                    const qreal lon   = lon_scale * ( id.x() * tileWidth + cur_x );
                    shade = m_sunLocator->shading( lon, a, c );
                    m_sunLocator->shadePixel( *scanline, shade );
                    ++scanline;
                    ++cur_x;
                }
            }

            else {
                // Make sure we don't exceed the image memory
                if ( cur_x < tileWidth ) {
                    const qreal lon   = lon_scale * ( id.x() * tileWidth + cur_x );
                    shade = m_sunLocator->shading( lon, a, c );
                    m_sunLocator->shadePixel( *scanline, shade );
                    ++scanline;
                    ++cur_x;
                }
            }
            lastShade = shade;
        }
    }
}

void MergedLayerDecorator::Private::paintTileId( QImage *tileImage, const TileId &id ) const
{
    QString filename = QStringLiteral( "%1_%2.jpg" )
            .arg( id.x(), tileDigits, 10, QChar('0') )
            .arg( id.y(), tileDigits, 10, QChar('0') );

    QPainter painter( tileImage );

    QColor foreground;
    QColor background;

    if ( ( (qreal)(id.x())/2 == id.x()/2 && (qreal)(id.y())/2 == id.y()/2 )
         || ( (qreal)(id.x())/2 != id.x()/2 && (qreal)(id.y())/2 != id.y()/2 )
         )
    {
        foreground.setNamedColor( QStringLiteral("#FFFFFF") );
        background.setNamedColor( QStringLiteral("#000000") );
    }
    else {
        foreground.setNamedColor( QStringLiteral("#000000") );
        background.setNamedColor( QStringLiteral("#FFFFFF") );
    }

    int   strokeWidth = 10;
    QPen  testPen( foreground );
    testPen.setWidth( strokeWidth );
    testPen.setJoinStyle( Qt::MiterJoin );

    painter.setPen( testPen );
    painter.drawRect( strokeWidth / 2, strokeWidth / 2,
                      tileImage->width()  - strokeWidth,
                      tileImage->height() - strokeWidth );
    QFont testFont( QStringLiteral("Sans"), 20 );
    QFontMetrics testFm( testFont );
    painter.setFont( testFont );

    QPen outlinepen( foreground );
    outlinepen.setWidthF( 6 );

    painter.setPen( outlinepen );
    painter.setBrush( background );

    QPainterPath   outlinepath;

    QPointF  baseline1( ( tileImage->width() - testFm.boundingRect(filename).width() ) / 2,
                        ( tileImage->height() * 0.25) );
    outlinepath.addText( baseline1, testFont, QStringLiteral( "level: %1" ).arg(id.tileLevel()) );

    QPointF  baseline2( ( tileImage->width() - testFm.boundingRect(filename).width() ) / 2,
                        tileImage->height() * 0.50 );
    outlinepath.addText( baseline2, testFont, filename );

    QPointF  baseline3( ( tileImage->width() - testFm.boundingRect(filename).width() ) / 2,
                        tileImage->height() * 0.75 );
    outlinepath.addText( baseline3, testFont, m_themeId );

    painter.drawPath( outlinepath );

    painter.setPen( Qt::NoPen );
    painter.drawPath( outlinepath );
}

void MergedLayerDecorator::Private::detectMaxTileLevel()
{
    if ( m_textureLayers.isEmpty() ) {
        m_maxTileLevel = -1;
        return;
    }

    m_maxTileLevel = m_tileLoader->maximumTileLevel( *m_textureLayers.at( 0 ) );
}

QVector<const GeoSceneTextureTileDataset *> MergedLayerDecorator::Private::findRelevantTextureLayers( const TileId &stackedTileId ) const
{
    QVector<const GeoSceneTextureTileDataset *> result;

    foreach ( const GeoSceneTextureTileDataset *candidate, m_textureLayers ) {
        Q_ASSERT( candidate );
        // check, if layer provides tiles for the current level
        if ( !candidate->hasMaximumTileLevel() ||
             candidate->maximumTileLevel() >= stackedTileId.tileLevel() ) {
            //check if the tile intersects with texture bounds
            if ( candidate->latLonBox().isNull()
                || candidate->latLonBox().intersects( stackedTileId.toLatLonBox( candidate ) ) )
            {
                result.append( candidate );
            }
        }
    }

    return result;
}

// TODO: This should likely go into a math class in the future ...

int MergedLayerDecorator::Private::maxDivisor( int maximum, int fullLength )
{
    // Find the optimal interpolation interval n for the
    // current image canvas width
    int best = 2;

    int  nEvalMin = fullLength;
    for ( int it = 1; it <= maximum; ++it ) {
        // The optimum is the interval which results in the least amount
        // supporting points taking into account the rest which can't
        // get used for interpolation.
        int nEval = fullLength / it + fullLength % it;
        if ( nEval < nEvalMin ) {
            nEvalMin = nEval;
            best = it;
        }
    }
    return best;
}
