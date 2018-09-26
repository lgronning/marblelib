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

#include "GeoMultiLineStringGraphicsItem.h"
#include "GeoLineStringGraphicsItem.h"
#include "GeoLabelPlaceHandler.h"
#include "GeoGraphicsItemHelper.h"

#include "geodata/data/GeoDataFeature.h"
#include "geodata/data/GeoDataLineString.h"
#include "geodata/data/GeoDataLineStyle.h"
#include "geodata/data/GeoDataPolyStyle.h"
#include "geodata/data/GeoDataLabelStyle.h"
#include "geodata/data/GeoDataIconStyle.h"
#include "GeoPainter.h"
#include "ViewportParams.h"
#include "geodata/data/GeoDataStyle.h"
#include "geodata/data/GeoDataMultiLineString.h"
#include <QtCore/QDebug>

namespace Marble
{

GeoMultiLineStringGraphicsItem::GeoMultiLineStringGraphicsItem( const GeoDataFeature *feature,
                                                      const GeoDataMultiLineString* lineStrings )
        : GeoGraphicsItem( feature ),
          m_lineStrings( lineStrings )
{
}

GeoMultiLineStringGraphicsItem::~GeoMultiLineStringGraphicsItem()
{
}

const GeoDataLatLonAltBox& GeoMultiLineStringGraphicsItem::latLonAltBox() const
{
    return m_lineStrings->latLonAltBox();
}

void GeoMultiLineStringGraphicsItem::renderGeometry(GeoPainter *painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style)
{
    QPixmap pointPixmap = style->lineStyle().scaledPointPixmap();

    // Immediately leave this method now if:
    // - the object is not visible in the viewport or if
    // - the size of the object is below the resolution of the viewport
    if ( ! viewport->viewLatLonAltBox().intersects( latLonAltBox() ) ||
         (pointPixmap.isNull() && !viewport->resolves( latLonAltBox())) )
    {
        // qDebug() << "LineString doesn't get displayed on the viewport";
        return;
    }

    QPen currentPen = painter->pen();
    QPen oldPen = currentPen;
    QBrush oldBrush = painter->background();
    Qt::BGMode oldBackgroundMode = painter->backgroundMode();

    if ( !style ) {
        painter->setPen( QPen() );
    }
    else
    {
        if ( currentPen.color() != style->lineStyle().paintedColor() )
            currentPen.setColor( style->lineStyle().paintedColor() );

        if ( !qFuzzyCompare(currentPen.widthF(), style->lineStyle().width()) ||
                style->lineStyle().physicalWidth() != 0.0 ) {
            if ( double( viewport->radius() ) / EARTH_RADIUS * style->lineStyle().physicalWidth() < style->lineStyle().width() )
                currentPen.setWidthF( style->lineStyle().width() );
            else
                currentPen.setWidthF( double( viewport->radius() ) / EARTH_RADIUS * style->lineStyle().physicalWidth() );
        }
        else if ( style->lineStyle().width() != 0.0 ) {
            currentPen.setWidthF( style->lineStyle().width() );
        }

        if ( currentPen.capStyle() != style->lineStyle().capStyle() )
            currentPen.setCapStyle( style->lineStyle().capStyle() );

        if ( currentPen.style() != style->lineStyle().penStyle() )
            currentPen.setStyle( style->lineStyle().penStyle() );

        if ( style->lineStyle().penStyle() == Qt::CustomDashLine )
            currentPen.setDashPattern( style->lineStyle().dashPattern() );

        if ( painter->pen() != currentPen )
            painter->setPen( currentPen );

        if ( style->lineStyle().background() ) {
            QBrush brush = painter->background();
            brush.setColor( style->polyStyle().paintedColor() );
            painter->setBackground( brush );

            painter->setBackgroundMode( Qt::OpaqueMode );
        }
    }

    qDebug() << "GeoMultiLineStringGraphicsItem::paint" << currentPen.widthF() << style->lineStyle().scaledPointPixmap().isNull();
    if( style->lineStyle().cosmeticOutline() &&
        style->lineStyle().penStyle() == Qt::SolidLine )
    {
        if ( currentPen.widthF() > 2.5f )
        {
            currentPen.setWidthF( currentPen.widthF() - 2.0f );
        }
        currentPen.setColor( style->polyStyle().paintedColor() );
        painter->setPen( currentPen );
    }


    foreach(const GeoDataLineString &lineString, m_lineStrings->lineStrings())
    {
        // Immediately leave this method now if:
        // - the object is not visible in the viewport or if
        // - the size of the object is below the resolution of the viewport
        if ( ! viewport->viewLatLonAltBox().intersects( lineString.latLonAltBox() ) ||
             (pointPixmap.isNull() && !viewport->resolves( lineString.latLonAltBox())) )
        {
            // qDebug() << "LineString doesn't get displayed on the viewport";
            continue;
        }

        QVector<QPolygonF> polygons = GeoLineStringGraphicsItem::getPolygonsImpl(viewport, &lineString);
        QPixmap tempPixmap;
        if(style->lineStyle().alwaysUsePointPixmap() || (!style->lineStyle().alwaysUsePointPixmap() && lineString.size() > 2))
        {
            tempPixmap = pointPixmap;
        }

        foreach( const QPolygonF &itPolygon, polygons )
        {
            painter->drawPolyline( itPolygon, pointPixmap );
        }
    }

    painter->setPen(oldPen);
    painter->setBackground(oldBrush);
    painter->setBackgroundMode(oldBackgroundMode);
}

void GeoMultiLineStringGraphicsItem::renderLabels(GeoPainter *painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style, GeoLabelPlaceHandler &placeHandler)
{
    if(!style ||
       !style->labelStyle().showLabel() ||
       feature()->name().isEmpty() ||
       !viewport->viewLatLonAltBox().intersects( latLonAltBox()))
    {
        return;
    }

    LabelPositionFlags labelPositionFlags = NoLabel;
    if(style->labelStyle().showLabel())
    {
        // label styles
        switch ( style->labelStyle().alignment() ) {
        case GeoDataLabelStyle::Corner:
        case GeoDataLabelStyle::Right:
            labelPositionFlags |= LineStart;
            break;
        case GeoDataLabelStyle::Center:
            labelPositionFlags |= LineCenter;
            break;
        }
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

    const QVector<GeoDataLineString> & lineStrings = m_lineStrings->lineStrings();
    for(int i = 0; i < lineStrings.size(); ++i)
    {
        const GeoDataLineString &lineString = lineStrings[i];
        QVector<QPolygonF> polygons = GeoLineStringGraphicsItem::getPolygonsImpl(viewport, &lineString);
        bool ok = GeoLineStringGraphicsItem::drawPolylineLabel(painter, polygons, viewport, feature()->name(), labelPositionFlags, style->labelStyle().color(), style->labelStyle().font(), labelStyles, placeHandler, feature());
        if(ok)
        {
            break;
        }
    }
}

void GeoMultiLineStringGraphicsItem::renderIcons(GeoPainter *painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style)
{
    if(!style ||
       !style->iconStyle().hasIcon())
    {
        return;
    }

    GeoDataCoordinates coordinate = GeoDataCoordinates(style->iconStyle().hotSpot().x(), style->iconStyle().hotSpot().y(), 0, Marble::GeoDataCoordinates::Degree);
    if(!coordinate.isValid())
    {
        const QVector<GeoDataLineString> & lineStrings = m_lineStrings->lineStrings();
        if(lineStrings.size() > 0 && lineStrings.last().size() > 0)
        {
            coordinate = lineStrings.last().at(lineStrings.last().size()-1);
        }
    }

    const QPixmap &pixmap = style->iconStyle().scaledIcon();
    qreal y( 0.0 );
    QVector<double> x;
    bool globeHidesPoint;
    bool visible = viewport->screenCoordinates( coordinate, x, y, pixmap.size(), globeHidesPoint );
    if ( visible )
    {
        for( int it = 0; it < x.size(); ++it )
        {
            QPointF point( x[it], y );
            painter->drawPixmap(point - QPointF( pixmap.width() / 2 , pixmap.height() / 2 ), pixmap);
        }
    }
}

void
GeoMultiLineStringGraphicsItem::getTiles(GeoSceneTextureTileDataset *tileDataset, const TileId &tileId, int zoomLevel, TileMap &tiles, std::atomic<bool> &aCancel)
{   
    foreach(const GeoDataLineString &lineString, m_lineStrings->lineStrings())
    {
        int count = lineString.size();

        QPolygonF tempPolygon(count);
        for(int i = 0; i < count; ++i)
        {
            double lon;
            double lat;
            lineString.getLonLat(i, lon, lat, GeoDataCoordinates::Unit::Radian);

            tempPolygon[i] = QPointF(lon, lat);
        }

        GeoLineStringGraphicsItem::getTilesImpl(tileDataset, QVector<QPolygonF>() << tempPolygon, tileId, zoomLevel, tiles, aCancel);
    }
}

}
