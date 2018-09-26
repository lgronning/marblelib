#define QT_NO_DEBUG_OUTPUT
//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011      Konstantin Oblaukhov <oblaukhov.konstantin@gmail.com>
//

#include "GeoPolygonGraphicsItem.h"
#include "GeoLabelPlaceHandler.h"
#include "GeoGraphicsItemHelper.h"

#include "geodata/data/GeoDataLinearRing.h"
#include "geodata/data/GeoDataPolygon.h"
#include "geodata/data/GeoDataPolyStyle.h"
#include "geodata/data/GeoDataLineStyle.h"
#include "geodata/data/GeoDataIconStyle.h"
#include "geodata/data/GeoDataLabelStyle.h"
#include "service/plot/GeometryHelper.h"

#include "GeoPainter.h"
#include "geodata/parser/GeoDataTypes.h"
#include "geodata/data/GeoDataPlacemark.h"
#include "ViewportParams.h"
#include "geodata/data/GeoDataStyle.h"
#include "MarbleDirs.h"

#include <QVector2D>
#include <QtCore/qmath.h>
#include <QtCore/QElapsedTimer>

namespace Marble
{

GeoPolygonGraphicsItem::GeoPolygonGraphicsItem( const GeoDataFeature *feature, const GeoDataPolygon* polygon )
        : GeoGraphicsItem( feature ),
          m_polygon( polygon ),
          m_ring( nullptr )
{
}

GeoPolygonGraphicsItem::GeoPolygonGraphicsItem( const GeoDataFeature *feature, const GeoDataLinearRing* ring )
        : GeoGraphicsItem( feature ),
          m_polygon( nullptr ),
          m_ring( ring )
{
}

const GeoDataLatLonAltBox& GeoPolygonGraphicsItem::latLonAltBox() const
{
    if( m_polygon ) {
        return m_polygon->latLonAltBox();
    } else if ( m_ring ) {
        return m_ring->latLonAltBox();
    } else {
        return GeoGraphicsItem::latLonAltBox();
    }
}

void GeoPolygonGraphicsItem::renderGeometry(GeoPainter *painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style)
{
    if ( (m_polygon && !viewport->resolves( m_polygon->latLonAltBox(), 0.1) ) ||
         (m_ring && !viewport->resolves( m_ring->latLonAltBox(), 0.1)))
    {
        return;
    }




    QPen currentPen = painter->pen();
    painter->save();

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

    if ( m_polygon ) {
        painter->drawPolygon( *m_polygon );
    } else if ( m_ring ) {
        painter->drawPolygon( *m_ring );
    }

    painter->restore();
}

void GeoPolygonGraphicsItem::renderIcons(GeoPainter *painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style)
{
    if(!style->iconStyle().hasIcon())
    {
        return;
    }

    GeoDataCoordinates center = m_ring ? m_ring->latLonAltBox().center() : m_polygon->latLonAltBox().center();

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
            painter->drawPixmap(latLonAltBox().center(), icon);
        }
    }
}

void GeoPolygonGraphicsItem::renderLabels(GeoPainter *painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style, GeoLabelPlaceHandler &placeHandler)
{
    if(!style->labelStyle().showLabel() || feature()->name().isEmpty())
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

    bool const hasLabel = style->labelStyle().showLabel() && !feature()->name().isEmpty();
    if (hasLabel)
    {
        GeoDataCoordinates center = m_ring ? m_ring->latLonAltBox().center() : m_polygon->latLonAltBox().center();

        qreal y( 0.0 );
        QVector<double> x;
        bool globeHidesPoint;
        bool visible = viewport->screenCoordinates( center, x, y, QSizeF(), globeHidesPoint );
        if ( visible )
        {
            for( int it = 0; it < x.size(); ++it )
            {
                QPointF centerPoint( x[it], y );

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

void
GeoPolygonGraphicsItem::getTiles(GeoSceneTextureTileDataset *tileDataset, const TileId &tile, int zoomLevel, TileMap &tiles, std::atomic<bool> &aCancel)
{ //tile - object, geometry - geojson object, Number
  //Convert tile to lon/lat
    const GeoDataLinearRing &outerBoundary = m_polygon->outerBoundary();
    int count = outerBoundary.size();

    QPolygonF tempPolygon(count);
    for(int i = 0; i < count; ++i)
    {
        double lon;
        double lat;
        outerBoundary.getLonLat(i, lon, lat, GeoDataCoordinates::Unit::Radian);

        tempPolygon[i] = QPointF(lon, lat);
    }

    getTilesImpl(tileDataset, tempPolygon, tile, zoomLevel, tiles, aCancel);
}

namespace
{

void
getTilesFromPainterPath(GeoSceneTextureTileDataset *tileDataset, const QPainterPath &subject, const TileId &tileId, int zoomLevel, TileMap &tiles, std::atomic<bool> &aCancel)
{
    if(aCancel)
    {
        return;
    }

    double radius = ( 1 << tileId.tileLevel() ) *tileDataset->levelZeroColumns()  / 2.0;

    double lonLeft   = ( tileId.x() - radius) / radius * M_PI;
    double lonRight  = ( tileId.x() - radius + 1) / radius * M_PI;

    radius = ( 1 << tileId.tileLevel() ) * tileDataset->levelZeroRows() / 2.0;

    double latBottom = 0;
    double latTop = 0;

    switch ( tileDataset->projection() ) {
    case GeoSceneTileDataset::Equirectangular:
        latTop = (radius - tileId.y() - 1) / radius * M_PI / 2.0;
        latBottom = (radius - tileId.y()) / radius * M_PI / 2.0;
        break;
    case GeoSceneTileDataset::Mercator:
        latTop = atan( sinh( ( radius - tileId.y() - 1) / radius * M_PI ) );
        latBottom = atan( sinh( ( radius - tileId.y() ) / radius * M_PI ) );
        break;
    }

    QRectF rect(lonLeft, latTop, lonRight - lonLeft, latBottom-latTop);

    QRectF boundingRect = subject.boundingRect();
    QPolygonF result;
    bool inside = rect.contains(boundingRect);
    if(inside)
    {
        result = subject.toFillPolygon();
    }
    else if(rect.intersects(boundingRect))
    {
        QPainterPath clip;
        clip.addPolygon(rect);

        result = subject.intersected(clip).toFillPolygon();
    }

    if(!result.isEmpty())
    {
        if (result.boundingRect() == rect &&
            result.size() == 5)
        {
            tiles[tileId.tileLevel()][tileId.x()][tileId.y()] = TileStatus::Full;
        }
        else
        {
            if (tileId.tileLevel()< zoomLevel)
            {
                QPainterPath tempSubject;
                tempSubject.addPolygon(result);

                TileId tile1(tileId.mapThemeIdHash(), tileId.tileLevel()+1, tileId.x()*2, tileId.y()*2);
                TileId tile2(tileId.mapThemeIdHash(), tileId.tileLevel()+1, (tileId.x()*2) + 1, tileId.y()*2);
                TileId tile3(tileId.mapThemeIdHash(), tileId.tileLevel()+1, tileId.x()*2, (tileId.y()*2)+1);
                TileId tile4(tileId.mapThemeIdHash(), tileId.tileLevel()+1, (tileId.x()*2) + 1, (tileId.y()*2)+1);

                getTilesFromPainterPath(tileDataset, tempSubject, tile1, zoomLevel, tiles, aCancel);
                getTilesFromPainterPath(tileDataset, tempSubject, tile2, zoomLevel, tiles, aCancel);
                getTilesFromPainterPath(tileDataset, tempSubject, tile3, zoomLevel, tiles, aCancel);
                getTilesFromPainterPath(tileDataset, tempSubject, tile4, zoomLevel, tiles, aCancel);
            }
            else
            {
                tiles[tileId.tileLevel()][tileId.x()][tileId.y()] = TileStatus::Partially;
            }
        }
    }

}

}

void
GeoPolygonGraphicsItem::getTilesImpl(GeoSceneTextureTileDataset *tileDataset, const QPolygonF &polygon, const TileId &tile, int zoomLevel, TileMap &tiles, std::atomic<bool> &aCancel)
{
    QPainterPath subject;
    subject.addPolygon(polygon);

    getTilesFromPainterPath(tileDataset, subject, tile, zoomLevel, tiles, aCancel);
};


}
