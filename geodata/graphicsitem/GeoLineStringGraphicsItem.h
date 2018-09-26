//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Andrew Manson <g.real.ate@gmail.com>
//

#ifndef MARBLE_GEOLINESTRINGGRAPHICSITEM_H
#define MARBLE_GEOLINESTRINGGRAPHICSITEM_H

#include "graphicsview/GeoGraphicsItem.h"
#include "MarbleGlobal.h"

namespace Marble
{

class GeoDataLineString;
class GeoDataLineStyle;

class MARBLE_EXPORT GeoLineStringGraphicsItem : public GeoGraphicsItem
{
public:
    explicit GeoLineStringGraphicsItem( const GeoDataFeature *feature, const GeoDataLineString *lineString );

    
    ~GeoLineStringGraphicsItem() override;

    void setLineString( const GeoDataLineString* lineString );

    
    const GeoDataLatLonAltBox&
    latLonAltBox() const override;

    
    void
    renderGeometry( GeoPainter* painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style ) override;

    
    void
    renderLabels( GeoPainter* painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style, GeoLabelPlaceHandler &placeHandler  ) override;

    
    void
    renderIcons( GeoPainter* painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style ) override;

    
    void
    getTiles(GeoSceneTextureTileDataset *tileDataset, const TileId &tile, int zoomLevel, TileMap &tiles, std::atomic<bool> &aCancel) override;

protected:
    const GeoDataLineString *m_lineString;

private:
    friend class GraticulePlugin;
    friend class GeoMultiLineStringGraphicsItem;

    static
    bool drawPolylineLabel(GeoPainter* painter, const QVector<QPolygonF> &polygons, const ViewportParams *viewport, const QString& labelText, LabelPositionFlags labelPositionFlags, const QColor& labelColor, const QFont& labelFont, int labelStyles, GeoLabelPlaceHandler &placeHandler, const GeoDataFeature* feature);

    static
    QVector<QPolygonF>
    getPolygonsImpl(const ViewportParams *viewport, const GeoDataLineString *lineString) ;

    static
    QVector<QPointF>
    labelPosition(const QPolygonF & polygon, LabelPositionFlags labelPositionFlags, const ViewportParams *viewport);

    static
    bool
    pointAllowsLabel(const QPointF &point, const ViewportParams *viewport);

    static
    QPointF
    interpolateLabelPoint(const QPointF &previousPoint, const QPointF &currentPoint, LabelPositionFlags labelPositionFlags, const ViewportParams *viewport);

    static
    void
    getTilesImpl(GeoSceneTextureTileDataset *tileDataset, const QVector<QPolygonF> &tempPolygon, const TileId &tile, int zoomLevel,  TileMap &tiles, std::atomic<bool> &aCancel);

    static
    QSizeF
    getSize(GeoDataStyle::ConstPtr style, double radius, const GeoDataLineString *lineString);
};

}

#endif
