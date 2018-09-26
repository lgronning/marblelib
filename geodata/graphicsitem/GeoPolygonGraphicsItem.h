//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011      Konstantin Oblaukhov <oblaukhov.konstantin@gmail.com>
//

#ifndef MARBLE_GEOPOLYGONGRAPHICSITEM_H
#define MARBLE_GEOPOLYGONGRAPHICSITEM_H

#include "graphicsview/GeoGraphicsItem.h"
#include "marble_export.h"
#include <QImage>
#include <QtGui/QColor>

class QPointF;

namespace Marble
{

class GeoDataLinearRing;
class GeoDataPolygon;

class MARBLE_EXPORT GeoPolygonGraphicsItem : public GeoGraphicsItem
{
public:
    explicit GeoPolygonGraphicsItem( const GeoDataFeature *feature, const GeoDataPolygon* polygon );
    explicit GeoPolygonGraphicsItem( const GeoDataFeature *feature, const GeoDataLinearRing* ring );

    const GeoDataLatLonAltBox& latLonAltBox() const override;

    
    void
    renderGeometry( GeoPainter* painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style ) override;


    
    void
    renderIcons( GeoPainter* painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style ) override;

    
    void
    renderLabels( GeoPainter* painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style, GeoLabelPlaceHandler &placeHandler  ) override;

    
    void
    getTiles(GeoSceneTextureTileDataset *tileDataset, const TileId &tile, int zoomLevel, TileMap &tiles, std::atomic<bool> &aCancel) override;

private:
    friend class GeoMultiPolygonGraphicsItem;
    friend class GeoGroundGraphicsItem;
    friend class GeoPointGraphicsItem;

    static
    void
    getTilesImpl(GeoSceneTextureTileDataset *tileDataset, const QPolygonF &polygon, const TileId &tile, int zoomLevel, TileMap &tiles, std::atomic<bool> &aCancel);

    static
    QVector<QPolygonF>
    getPolygonsImpl(const ViewportParams *viewport, QVector<GeoDataLinearRing> &linearRings, bool fill);

    static
    QVector<QPolygonF>
    getGeoPolygons(const ViewportParams *viewport, const GeoDataLinearRing &lineString, const GeoDataLatLonAltBox &latLongAltBox);

    const GeoDataPolygon *const m_polygon;
    const GeoDataLinearRing *const m_ring;
    QString m_cachedTexturePath;
    QColor m_cachedTextureColor;
    QImage m_cachedTexture;
};

}

#endif
