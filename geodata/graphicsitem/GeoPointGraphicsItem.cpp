#define QT_NO_DEBUG_OUTPUT
//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Andrew Manson <g.real.ate@gmail.com>
//

#include "GeoPointGraphicsItem.h"
#include "GeoLabelPlaceHandler.h"
#include "GeoPainter.h"
#include "geodata/data/GeoDataFeature.h"
#include "geodata/data/GeoDataStyle.h"
#include "geodata/data/GeoDataLabelStyle.h"
#include "geodata/data/GeoDataPointStyle.h"
#include "geodata/data/GeoDataIconStyle.h"
#include "ViewportParams.h"

#include <QtCore/QDebug>

namespace Marble
{

GeoPointGraphicsItem::GeoPointGraphicsItem( const GeoDataFeature *feature,  const GeoDataPoint* point)
        : GeoGraphicsItem( feature ),
          m_point(point)
{
}

void GeoPointGraphicsItem::setPoint( const GeoDataPoint* point )
{
    m_point = point;
}

const GeoDataPoint *
GeoPointGraphicsItem::point() const
{
    return m_point;
}

void GeoPointGraphicsItem::renderGeometry(GeoPainter *painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style)
{
    return;

    if(! viewport->viewLatLonAltBox().intersects( m_point->latLonAltBox() ))
    {
        return;
    }

    qDebug() << "GeoPointGraphicsItem::paint" << style->pointStyle().scaledIcon().isNull() << style->pointStyle().scaledIcon().size();

    painter->drawPoint( m_point->coordinates() );
}

void GeoPointGraphicsItem::renderIcons(GeoPainter *painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style)
{
    if(!style->iconStyle().hasIcon() && style->pointStyle().scaledIcon().isNull())
    {
        return;
    }

    qreal y( 0.0 );
    QVector<double> x;
    bool globeHidesPoint;

    const QPixmap &pointScaledPixmap = style->pointStyle().scaledIcon();
    const QPixmap &iconScaledPixmap = style->iconStyle().scaledIcon();
    QSize size(0,0);
    size.setHeight(qMax(iconScaledPixmap.height(), pointScaledPixmap.height()));
    size.setWidth(qMax(iconScaledPixmap.width(), pointScaledPixmap.width()));

    bool visible = viewport->screenCoordinates( m_point->coordinates(), x, y, size, globeHidesPoint );
    if ( visible )
    {
        for( int it = 0; it < x.size(); ++it )
        {
            QPointF point( x[it], y );
            if(!pointScaledPixmap.isNull())
            {
                painter->drawPixmap(point - QPointF(( pointScaledPixmap.width() / 2 ), ( pointScaledPixmap.height() / 2 ) ), pointScaledPixmap );
            }

            if(!iconScaledPixmap.isNull())
            {
                painter->drawPixmap(point - QPointF(( iconScaledPixmap.width() / 2 ), ( iconScaledPixmap.height() / 2 ) ), iconScaledPixmap);
                point.setY(point.y() + iconScaledPixmap.height()/2);
            }
        }
    }
}

void GeoPointGraphicsItem::renderLabels(GeoPainter *painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style, GeoLabelPlaceHandler &placeHandler)
{
    if(!viewport->viewLatLonAltBox().intersects( m_point->latLonAltBox() ) ||
       (!style->labelStyle().showLabel() || feature()->name().isEmpty()) )
    {
        return;
    }

    GeoPainter::LabelStyles labelStyles = GeoPainter::Normal;
    if(style->labelStyle().highlightText())
    {
        labelStyles |= GeoPainter::LabelStyle::Highlighted;
    }

    if(style->labelStyle().glow())
    {
        labelStyles |= GeoPainter::LabelStyle::Glow;
    }

    qreal y( 0.0 );
    QVector<double> x;
    bool globeHidesPoint;
    bool visible = viewport->screenCoordinates( m_point->coordinates(), x, y, QSizeF(), globeHidesPoint );
    if ( visible )
    {
        for( int it = 0; it < x.size(); ++it )
        {
            QPointF point( x[it], y );

            QString labelText = feature()->name();
            QRectF labelRect = painter->labelRect(labelText, style->labelStyle().font(), labelStyles);
            labelRect.moveCenter(point+QPointF(0,labelRect.height()/2));
            if(placeHandler.addLabelRect(labelRect, feature()))
            {
                painter->drawLabelText(labelRect, labelText, style->labelStyle().font(), labelStyles, style->labelStyle().color());
            }
        }
    }
}

const GeoDataLatLonAltBox& GeoPointGraphicsItem::latLonAltBox() const
{
    return m_point->latLonAltBox();
}

void GeoPointGraphicsItem::getTiles(GeoSceneTextureTileDataset *tileDataset, const TileId &tileId, int zoomLevel, TileMap &tiles, std::atomic<bool> &aCancel)
{
    getTilesImpl(tileDataset, m_point->coordinates(), tileId, zoomLevel, tiles, aCancel);
}

void GeoPointGraphicsItem::getTilesImpl(GeoSceneTextureTileDataset *tileDataset, const GeoDataCoordinates &point, const TileId &tile, int zoomLevel, TileMap &tiles, std::atomic<bool> &aCancel)
{
    if(aCancel)
    {
        return;
    }

    TileId tempTileId = TileId::fromCoordinates( tileDataset, point, zoomLevel );
    tiles[tempTileId.tileLevel()][tempTileId.x()][tempTileId.y()] = Marble::TileStatus::Partially;
}

}
