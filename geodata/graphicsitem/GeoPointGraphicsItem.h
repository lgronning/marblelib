//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Andrew Manson <g.real.ate@gmail.com>
//

#ifndef MARBLE_GEOPOINTGRAPHICSITEM_H
#define MARBLE_GEOPOINTGRAPHICSITEM_H

#include "geodata/data/GeoDataPoint.h"
#include "graphicsview/GeoGraphicsItem.h"
#include "geodata/data/GeoDataLatLonAltBox.h"
#include "marble_export.h"

namespace Marble
{

class MARBLE_EXPORT GeoPointGraphicsItem : public GeoGraphicsItem
{
public:
    explicit GeoPointGraphicsItem(const GeoDataFeature *feature , const GeoDataPoint *point);

    void
    setPoint( const GeoDataPoint* point );

    const GeoDataPoint *
    point() const;
    
    
    void
    renderGeometry( GeoPainter* painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style) override;

    
    void
    renderIcons( GeoPainter* painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style ) override;

    
    void
    renderLabels( GeoPainter* painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style, GeoLabelPlaceHandler &placeHandler  ) override;

    
    const
    GeoDataLatLonAltBox& latLonAltBox() const override;

    
    void
    getTiles(GeoSceneTextureTileDataset *tileDataset, const TileId &tileId, int zoomLevel, TileMap &tiles, std::atomic<bool> &aCancel) override;

protected:
    friend class GeoMultiPointGraphicsItem;

    static
    void
    getTilesImpl(GeoSceneTextureTileDataset *tileDataset, const GeoDataCoordinates &point, const TileId &tile, int zoomLevel, TileMap &tiles, std::atomic<bool> &aCancel);

    const GeoDataPoint *m_point;
};

}

#endif
