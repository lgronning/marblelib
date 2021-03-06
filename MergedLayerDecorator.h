// Copyright 2008 David Roberts <dvdr18@gmail.com>
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

#ifndef MARBLE_MERGEDLAYERDECORATOR_H
#define MARBLE_MERGEDLAYERDECORATOR_H

#include <QSize>
#include <QVector>
#include <QList>

#include "geodata/scene/GeoSceneTextureTileDataset.h"
#include "RenderState.h"

class QImage;
class QString;

namespace Marble
{

class SunLocator;
class StackedTile;
class Tile;
class TileId;
class AbstractTileLoader;

class MergedLayerDecorator
{
 public:
    MergedLayerDecorator( AbstractTileLoader * const tileLoader, const SunLocator* sunLocator );
    virtual ~MergedLayerDecorator();

    void
    setTextureLayers( const QVector<const GeoSceneTextureTileDataset *> &textureLayers );

    const QVector<const GeoSceneTextureTileDataset *> &
    textureLayers( );


    int textureLayersSize() const;

    /**
     * Returns the highest level in which some tiles are theoretically
     * available for the current texture layers.
     */
    int maximumTileLevel() const;

    int tileColumnCount( int level ) const;

    int tileRowCount( int level ) const;

    GeoSceneTextureTileDataset::Projection tileProjection() const;

    QSize tileSize() const;

    StackedTile *loadTile( const TileId &id );

    StackedTile *updateTile( const StackedTile &stackedTile, const TileId &tileId, const QImage &tileImage );

    void downloadStackedTile( const TileId &id, DownloadUsage usage );

    void setShowSunShading( bool show );
    bool showSunShading() const;

    void setShowCityLights( bool show );
    bool showCityLights() const;

    void setShowTileId(bool show);

    bool hasTextureLayer() const;

 protected:
    Q_DISABLE_COPY( MergedLayerDecorator )

    class Private;
    Private *const d;
};

}

#endif
