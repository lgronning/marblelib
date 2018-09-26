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

#include "GeoMultiPolygonGraphicsItem.h"
#include "GeoLineStringGraphicsItem.h"
#include "GeoLabelPlaceHandler.h"
#include "GeoPolygonGraphicsItem.h"
#include "GeoGraphicsItemHelper.h"

#include "geodata/data/GeoDataFeature.h"
#include "geodata/data/GeoDataMultiPolygon.h"
#include "geodata/data/GeoDataPolygon.h"
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

GeoMultiPolygonGraphicsItem::GeoMultiPolygonGraphicsItem( const GeoDataFeature *feature,
                                                      const GeoDataMultiPolygon * polygons)
        : GeoGraphicsItem( feature ),
          m_polygons( polygons )
{
}

GeoMultiPolygonGraphicsItem::~GeoMultiPolygonGraphicsItem()
{
}

const GeoDataLatLonAltBox& GeoMultiPolygonGraphicsItem::latLonAltBox() const
{
    return m_polygons->latLonAltBox();
}

void GeoMultiPolygonGraphicsItem::renderGeometry(GeoPainter *painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style)
{
    if ( ! viewport->viewLatLonAltBox().intersects( latLonAltBox() ) || ( !viewport->resolves( m_polygons->latLonAltBox()) ) )
    {
        return;
    }

    painter->save();
    QPen currentPen = painter->pen();

    if ( !style ) {
        painter->setPen( QPen() );
    }
    else
    {
        if ( !style->polyStyle().outline() )
        {
            currentPen.setColor( Qt::transparent );
        }
        else {
            if ( currentPen.color() != style->lineStyle().paintedColor() ||
                 currentPen.widthF() != style->lineStyle().width() ) {
                currentPen.setColor( style->lineStyle().paintedColor() );
                currentPen.setWidthF( style->lineStyle().width() );
            }

            if ( currentPen.capStyle() != style->lineStyle().capStyle() )
                currentPen.setCapStyle( style->lineStyle().capStyle() );

            if ( currentPen.style() != style->lineStyle().penStyle() )
                currentPen.setStyle( style->lineStyle().penStyle() );
        }

        if ( painter->pen() != currentPen )
            painter->setPen( currentPen );

        if ( !style->polyStyle().fill() ) {
            if ( painter->brush().color() != Qt::transparent )
                painter->setBrush( QColor( Qt::transparent ) );
        }
        else
        {
            if ( painter->brush().color() != style->polyStyle().paintedColor() )
            {
                QImage textureImage = style->polyStyle().textureImage();
                if( !textureImage.isNull()){
                    GeoDataCoordinates coords = latLonAltBox().center();
                    qreal x, y;
                    viewport->screenCoordinates(coords, x, y);
                    if (m_cachedTexturePath != style->polyStyle().texturePath() || m_cachedTextureColor != style->polyStyle().paintedColor() ) {
                        if (textureImage.hasAlphaChannel()) {
                            m_cachedTexture = QImage ( textureImage.size(), QImage::Format_ARGB32_Premultiplied );
                            m_cachedTexture.fill(style->polyStyle().paintedColor());
                            QPainter imagePainter(&m_cachedTexture );
                            imagePainter.drawImage(0, 0, textureImage);
                        }
                        else {
                            m_cachedTexture = textureImage;
                        }
                        m_cachedTexturePath = style->polyStyle().texturePath();
                        m_cachedTextureColor = style->polyStyle().paintedColor();
                    }
                    QBrush brush(m_cachedTexture);
                    painter->setBrush(brush);
                    painter->setBrushOrigin(QPoint(x,y));
                }
                else
                {
                    painter->setBrush( style->polyStyle().paintedColor() );
                }
            }
        }
    }

    foreach(const GeoDataPolygon &polygon, m_polygons->polygons())
    {
        if ( ! viewport->viewLatLonAltBox().intersects( polygon.latLonAltBox() ) || ( !viewport->resolves( polygon.latLonAltBox()) ) )
        {
            continue;
        }

        painter->drawPolygon( polygon );
    }

    painter->restore();
}

void GeoMultiPolygonGraphicsItem::renderLabels(GeoPainter *painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style, GeoLabelPlaceHandler &placeHandler)
{
    if(!viewport->viewLatLonAltBox().intersects( latLonAltBox() ) ||
       ((!style->labelStyle().showLabel() || feature()->name().isEmpty()) && !style->iconStyle().hasIcon()))
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


    QString labelText = feature()->name();
    QFont labelFont = style->labelStyle().font();

    bool const hasIcon = style->iconStyle().hasIcon();
    bool const hasLabel = style->labelStyle().showLabel() && !feature()->name().isEmpty();
    foreach(const GeoDataPolygon &polygon, m_polygons->polygons())
    {
        if(! viewport->viewLatLonAltBox().intersects( polygon.latLonAltBox() ))
        {
            continue;
        }

        GeoDataCoordinates center = GeoDataCoordinates(style->iconStyle().hotSpot().x(), style->iconStyle().hotSpot().y(), 0, Marble::GeoDataCoordinates::Degree);
        if(!center.isValid())
        {
            center =  polygon.latLonAltBox().center();
        }

        qreal y( 0.0 );
        QVector<double> x;
        bool globeHidesPoint;
        bool visible = viewport->screenCoordinates( center, x, y, QSizeF(), globeHidesPoint );
        if ( visible )
        {
            for( int it = 0; it < x.size(); ++it )
            {
                QPointF centerPoint( x[it], y );

                if(hasIcon)
                {
                    const QPixmap &icon = style->iconStyle().scaledIcon();
                    painter->drawPixmap(centerPoint, icon);
                }
                else if(hasLabel)
                {
                    QRectF labelRect = painter->labelRect(labelText, labelFont, labelStyles);
                    switch(style->labelStyle().alignment())
                    {
                        case GeoDataLabelStyle::Alignment::Center:
                            labelRect.moveCenter(centerPoint);
                        break;
                        case GeoDataLabelStyle::Alignment::Right:
                        case GeoDataLabelStyle::Alignment::Corner:
                            labelRect.moveTo(centerPoint);
                        break;
                    }

                    if(placeHandler.addLabelRect(labelRect, feature()))
                    {
                        painter->drawLabelText(labelRect, labelText, labelFont, labelStyles, style->labelStyle().color());
                    }

                }
            }
        }
    }
}

void GeoMultiPolygonGraphicsItem::renderIcons(GeoPainter *painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style)
{
    if(!style->iconStyle().hasIcon())
    {
        return;
    }

    foreach(const GeoDataPolygon &polygon, m_polygons->polygons())
    {
        if(! viewport->viewLatLonAltBox().intersects( polygon.latLonAltBox() ))
        {
            continue;
        }

        GeoDataCoordinates center = GeoDataCoordinates(style->iconStyle().hotSpot().x(), style->iconStyle().hotSpot().y(), 0, Marble::GeoDataCoordinates::Degree);
        if(!center.isValid())
        {
            center =  polygon.latLonAltBox().center();
        }

        const QPixmap &icon = style->iconStyle().scaledIcon();

        qreal y( 0.0 );
        QVector<double> x;
        bool globeHidesPoint;
        bool visible = viewport->screenCoordinates( center, x, y, icon.size(), globeHidesPoint );
        if ( visible )
        {
            for( int it = 0; it < x.size(); ++it )
            {
                QPointF centerPoint( x[it], y );

                painter->drawPixmap(centerPoint, icon);
            }
        }
    }
}

void GeoMultiPolygonGraphicsItem::getTiles(GeoSceneTextureTileDataset *tileDataset, const TileId &tile, int zoomLevel, TileMap &tiles, std::atomic<bool> &aCancel)
{

    foreach(const GeoDataPolygon &polygon, m_polygons->polygons())
    {
        const GeoDataLinearRing &outerBoundary = polygon.outerBoundary();
        int count = polygon.outerBoundary().size();

        QPolygonF tempPolygon(count);
        for(int i = 0; i < count; ++i)
        {
            double lon;
            double lat;
            outerBoundary.getLonLat(i, lon, lat, GeoDataCoordinates::Unit::Radian);

            tempPolygon[i] = QPointF(lon, lat);
        }

        GeoPolygonGraphicsItem::getTilesImpl(tileDataset, tempPolygon, tile, zoomLevel, tiles, aCancel);
    }
}

}
