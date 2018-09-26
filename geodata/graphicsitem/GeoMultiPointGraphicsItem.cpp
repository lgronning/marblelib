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

#include "GeoMultiPointGraphicsItem.h"
#include "GeoLabelPlaceHandler.h"
#include "GeoPointGraphicsItem.h"
#include "GeoGraphicsItemHelper.h"
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

GeoMultiPointGraphicsItem::GeoMultiPointGraphicsItem( const GeoDataFeature *feature,  const GeoDataMultiPoint* points)
        : GeoGraphicsItem( feature ),
          m_points(points)
{
}

void GeoMultiPointGraphicsItem::setPoints( const GeoDataMultiPoint* points )
{
    m_points = points;
}

const GeoDataMultiPoint *
GeoMultiPointGraphicsItem::points() const
{
    return m_points;
}

void GeoMultiPointGraphicsItem::renderGeometry(GeoPainter *painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style)
{
    if(! viewport->viewLatLonAltBox().intersects( latLonAltBox() ))
    {
        return;
    }

    qDebug() << "GeoMultiPointGraphicsItem::paint" << style->pointStyle().scaledIcon().isNull() << style->pointStyle().scaledIcon().size();

    for(int i = 0; i < m_points->size(); ++i)
    {
        if(! viewport->viewLatLonAltBox().contains( m_points->at(i) ))
        {
            continue;
        }

        painter->drawPoint( m_points->at(i) );
    }
}

void GeoMultiPointGraphicsItem::renderIcons(GeoPainter *painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style)
{
    if(!style->iconStyle().hasIcon())
    {
        return;
    }

    QPixmap pixmap = style->iconStyle().scaledIcon();

    for(int i = 0; i < m_points->size(); ++i)
    {
        qreal y( 0.0 );
        QVector<double> x;
        bool globeHidesPoint;
        bool visible = viewport->screenCoordinates( m_points->at(i), x, y, pixmap.size(), globeHidesPoint );
        if ( visible )
        {
            for( int it = 0; it < x.size(); ++it )
            {
                QPointF point( x[it], y );
                painter->drawPixmap(point - QPointF( pixmap.width() / 2 , pixmap.height() / 2 ), pixmap);
            }
        }
    }
}

void GeoMultiPointGraphicsItem::renderLabels(GeoPainter *painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style, GeoLabelPlaceHandler &placeHandler)
{  
    if(! viewport->viewLatLonAltBox().intersects( latLonAltBox() ) ||
       (!style->labelStyle().showLabel() || feature()->name().isEmpty()))
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

    for(int i = 0; i < m_points->size(); ++i)
    {
        if(! viewport->viewLatLonAltBox().contains( m_points->at(i) ))
        {
            continue;
        }

        qreal y( 0.0 );
        QVector<double> x;
        bool globeHidesPoint;
        bool visible = viewport->screenCoordinates( m_points->at(i), x, y, QSizeF(), globeHidesPoint );
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
}

const GeoDataLatLonAltBox& GeoMultiPointGraphicsItem::latLonAltBox() const
{
    return m_points->latLonAltBox();
}

void GeoMultiPointGraphicsItem::getTiles(GeoSceneTextureTileDataset *tileDataset, const TileId &tile, int zoomLevel, TileMap &tiles, std::atomic<bool> &aCancel)
{
    int count = m_points->size();
    for(int i =0; i < count; ++i)
    {
        GeoPointGraphicsItem::getTilesImpl(tileDataset, m_points->at(i), tile, zoomLevel, tiles, aCancel);
    }
}

}
