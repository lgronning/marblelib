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

#include "GeoLineStringGraphicsItem.h"
#include "GeoLabelPlaceHandler.h"
#include "GeoGraphicsItemHelper.h"

#include "geodata/data/GeoDataFeature.h"
#include "geodata/data/GeoDataLineString.h"
#include "geodata/data/GeoDataLineStyle.h"
#include "geodata/data/GeoDataPolyStyle.h"
#include "geodata/data/GeoDataLabelStyle.h"
#include "geodata/data/GeoDataIconStyle.h"
#include "geodata/data/GeoDataSpectroStyle.h"
#include "GeoPainter.h"
#include "ViewportParams.h"
#include "geodata/data/GeoDataStyle.h"
#include <QtCore/QDebug>
#include <QtCore/QElapsedTimer>
#include <QtCore/QThread>
#include <geos_c.h>
#include <iostream>

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/adapted/c_array.hpp>
#include <boost/geometry/multi/geometries/multi_polygon.hpp>


using namespace Marble;

BOOST_GEOMETRY_REGISTER_C_ARRAY_CS(cs::cartesian)

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/register/linestring.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/adapted/boost_polygon.hpp>

BOOST_GEOMETRY_REGISTER_POINT_2D_GET_SET(QPointF, double, boost::geometry::cs::cartesian, x, y, setX, setY)
BOOST_GEOMETRY_REGISTER_LINESTRING(QPolygonF)


namespace Marble
{

static const double m_labelAreaMargin = 10.0;

GeoLineStringGraphicsItem::GeoLineStringGraphicsItem( const GeoDataFeature *feature,
                                                      const GeoDataLineString* lineString )
        : GeoGraphicsItem( feature ),
          m_lineString( lineString )
{
}

GeoLineStringGraphicsItem::~GeoLineStringGraphicsItem()
{
}



void GeoLineStringGraphicsItem::setLineString( const GeoDataLineString* lineString )
{
    m_lineString = lineString;
}

const GeoDataLatLonAltBox& GeoLineStringGraphicsItem::latLonAltBox() const
{
    return m_lineString->latLonAltBox();
}

namespace
{

QColor
getColor(const Marble::ColorMapPtr &colorMap, const QVector<QRgb> &colorTable, const Marble::ColorMapInterval &colorRange, Marble::ColorMap::Format format, double messure)
{
    if(format == Marble::ColorMap::RGB)
    {
        const QRgb rgb = colorMap->rgb(colorRange, messure );
        return QColor( rgb );
    }

    const unsigned char index = colorMap->colorIndex(colorRange, messure);
    return QColor( colorTable[index]);
}

}


void GeoLineStringGraphicsItem::renderGeometry(GeoPainter *painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style)
{
    // Immediately leave this method now if:
    // - the object is not visible in the viewport or if
    // - the size of the object is below the resolution of the viewport
    if ( ! viewport->viewLatLonAltBox().intersects( latLonAltBox() ) ||
         !viewport->resolves( latLonAltBox()))
    {
        // qDebug() << "LineString doesn't get displayed on the viewport";
        return;
    }


    QPen currentPen = painter->pen();
    QPen oldPen = currentPen;
    QBrush oldBrush = painter->background();
    Qt::BGMode oldBackgroundMode = painter->backgroundMode();

    if ( !style )
    {
        painter->setPen( QPen() );

        QVector<QPolygonF> polygons = getPolygonsImpl(viewport, m_lineString);
        foreach( const QPolygonF &itPolygon, polygons )
        {
            painter->drawPolyline( itPolygon );
        }
    }
    else
    {
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

        if ( style->lineStyle().background() ) {
            QBrush brush = painter->background();
            brush.setColor( style->polyStyle().paintedColor() );
            painter->setBackground( brush );

            painter->setBackgroundMode( Qt::OpaqueMode );
        }

        if( style->lineStyle().cosmeticOutline() &&
            style->lineStyle().penStyle() == Qt::SolidLine )
        {
            if ( currentPen.widthF() > 2.5f )
            {
                currentPen.setWidthF( currentPen.widthF() - 2.0f );
            }
        }

        if(m_lineString->hasMessure() && style->spectroStyle().colorMap())
        {
            const ColorMap::Format format = style->spectroStyle().colorMap()->format();
            QVector<QRgb> colorTable;
            if ( format == ColorMap::Indexed )
                colorTable = style->spectroStyle().colorMap()->colorTable( style->spectroStyle().colorInterval() );

            QPointF prevPoint;
            bool firstPoint = true;
            QPolygonF polyline;
            QColor previousColor;

            int to = m_lineString->size();
            for ( int i = 0; i < to; i++ )
            {

                double lon1;
                double lat1;

                m_lineString->getLonLat(i, lon1, lat1, GeoDataCoordinates::Degree);
                QPointF point( lon1, lat1 );

                const double mesure = m_lineString->messure(i);
                QColor color = getColor(style->spectroStyle().colorMap(), colorTable, style->spectroStyle().colorInterval(), format, mesure);

                polyline.append(point);

                if(firstPoint)
                {
                    firstPoint = false;
                    prevPoint = point;
                    previousColor = color;
                }
                else if(previousColor != color)
                {
                    if ( currentPen.color() != previousColor )
                        currentPen.setColor( previousColor );

                    Marble::GeoDataLineString tempLineString(polyline, m_lineString->tessellationFlags());
                    QVector<QPolygonF> tempPolygons = getPolygonsImpl(viewport, &tempLineString);

                    foreach( const QPolygonF &polygon, tempPolygons )
                    {
                        QPen tempPen = currentPen;

                        if(tempPen.widthF() >= 3.0)
                        {
                            QPen outlinePen = tempPen;

                            const double width = tempPen.widthF();
                            const QColor originalColor = tempPen.color();

                            const QColor cosmeticColor = originalColor.darker(200);

                            outlinePen.setColor(cosmeticColor);
                            outlinePen.setWidthF(width);

                            painter->setPen(outlinePen);

                            painter->drawPolyline( polygon );

                            tempPen.setWidthF(width - qMin(2, qMax(1, qRound(width/5))));
                            tempPen.setColor(originalColor);
                        }

                        if ( painter->pen() != tempPen )
                            painter->setPen( tempPen );

                        painter->drawPolyline( polygon );
                    }

                    polyline.clear();
                    polyline.append(point);

                    previousColor = color;
                }
            }

            if(!polyline.isEmpty())
            {
                if ( currentPen.color() != previousColor )
                    currentPen.setColor( previousColor );

                Marble::GeoDataLineString tempLineString(polyline, m_lineString->tessellationFlags());
                QVector<QPolygonF> tempPolygons = getPolygonsImpl(viewport, &tempLineString);

                foreach( const QPolygonF &polygon, tempPolygons )
                {
                    QPen tempPen = currentPen;

                    if(tempPen.widthF() >= 3.0)
                    {
                        QPen outlinePen = tempPen;

                        const double width = tempPen.widthF();
                        const QColor originalColor = tempPen.color();

                        const QColor cosmeticColor = originalColor.darker(200);

                        outlinePen.setColor(cosmeticColor);
                        outlinePen.setWidthF(width);

                        painter->setPen(outlinePen);

                        painter->drawPolyline( polygon );

                        tempPen.setWidthF(width - qMin(2, qMax(1, qRound(width/5))));
                        tempPen.setColor(originalColor);
                    }

                    if ( painter->pen() != tempPen )
                        painter->setPen( tempPen );

                    painter->drawPolyline( polygon );
                }
            }
        }
        else
        {
            if ( currentPen.color() != style->lineStyle().paintedColor() )
                currentPen.setColor( style->lineStyle().paintedColor() );

            QVector<QPolygonF> polygons = getPolygonsImpl(viewport, m_lineString);
        //    qWarning() << "GeoLineStringGraphicsItem::renderGeometry get polygons" << timer.nsecsElapsed();

            foreach( const QPolygonF &polygon, polygons )
            {
                QPen tempPen = currentPen;

                if(tempPen.widthF() >= 3.0)
                {
                    QPen outlinePen = tempPen;
                    double width = tempPen.widthF();
                    QColor originalColor = tempPen.color();

                    double luma = (0.2126 * originalColor.red()) + (0.7152 * originalColor.green()) + (0.0722 * originalColor.blue());
                    bool darkColor = (luma < 40);

                    QColor cosmeticColor = darkColor ? originalColor.lighter(150) : originalColor.darker(200);

                    outlinePen.setColor(cosmeticColor);
                    outlinePen.setWidthF(width);

                    painter->setPen(outlinePen);

                    painter->drawPolyline( polygon );

                    tempPen.setWidthF(width - qMin(2, qMax(1, qRound(width/5))));
                    tempPen.setColor(originalColor);
                }

                if ( painter->pen() != tempPen )
                    painter->setPen( tempPen );

                painter->drawPolyline( polygon );
            }
        }
    }

    painter->setPen(oldPen);
    painter->setBackground(oldBrush);
    painter->setBackgroundMode(oldBackgroundMode);
}

void GeoLineStringGraphicsItem::renderLabels(GeoPainter *painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style, GeoLabelPlaceHandler &placeHandler)
{
    if(!style || !viewport->viewLatLonAltBox().intersects( latLonAltBox()))
    {
        return;
    }

    if((style->labelStyle().showLabel() && !feature()->name().isEmpty()) || style->iconStyle().hasIcon())
    {
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

        drawPolylineLabel(painter, getPolygonsImpl(viewport, m_lineString), viewport, feature()->name(), labelPositionFlags, style->labelStyle().color(), style->labelStyle().font(), labelStyles, placeHandler, feature());

        if(style->iconStyle().hasIcon())
        {
            GeoDataCoordinates coordinate = GeoDataCoordinates(style->iconStyle().hotSpot().x(), style->iconStyle().hotSpot().y(), 0, Marble::GeoDataCoordinates::Degree);
            if(!coordinate.isValid() && m_lineString->size() > 0)
            {
                coordinate = m_lineString->at(m_lineString->size()-1);
            }

            if(coordinate.isValid())
            {
                painter->drawPixmap(coordinate, style->iconStyle().scaledIcon());
            }
        }
    }



}

void GeoLineStringGraphicsItem::renderIcons(GeoPainter *painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style)
{
    if(!style ||
       style->lineStyle().scaledPointPixmap().isNull() ||
       !(style->lineStyle().alwaysUsePointPixmap() && (!style->lineStyle().alwaysUsePointPixmap() || m_lineString->size() > 2)))
    {
        return;
    }

    QPixmap pointPixmap = style->lineStyle().scaledPointPixmap();

    QVector<QPolygonF> polygons;

//        qWarning() << "Geos Line String:" << GEOSGeomToWKT_r(handler, geosLineString);

    qreal north, south, east, west;
    viewport->viewLatLonAltBox().boundaries( north, south, east, west, GeoDataCoordinates::Unit::Degree );

    /* Need to add a margin so we dont have anomilies at the borders */
    double yMargin = (north - south)/20;
    north+=yMargin;
    south-=yMargin;

    double xMargin = (east- west)/20;
    east += xMargin;
    west -= xMargin;

//        qWarning() << "GeoLineStringGraphicsItem::getPolygonsImpl rect" << west << east << north << south;

    QElapsedTimer timer;

    timer.start();

    QPolygonF points = m_lineString->rawData();

    typedef boost::geometry::model::d2::point_xy<double> point_type;
    typedef boost::geometry::model::box<point_type> box_type;

    box_type box = boost::geometry::make<box_type>(west,
                                                   south,
                                                   east,
                                                   north);

    QVector<QPolygonF> output;
    boost::geometry::intersection(points, box, output);

    for (auto const& l : output)
    {
        foreach(const QPointF &point, l)
        {
            GeoDataCoordinates coordinate = GeoDataCoordinates(point.x(), point.y(), 0, GeoDataCoordinates::Unit::Degree);

            painter->drawPixmap(coordinate, pointPixmap);
        }
    }
}

QSizeF
GeoLineStringGraphicsItem::getSize(GeoDataStyle::ConstPtr style, double radius, const GeoDataLineString *lineString)
{
    QSizeF size(0,0);
    double width = style->lineStyle().width();
    size = size.expandedTo(QSizeF(width, width));

    if(!qFuzzyIsNull(style->lineStyle().physicalWidth()))
    {
        double widthPhysical = (radius / EARTH_RADIUS * style->lineStyle().physicalWidth()) ;
        size = size.expandedTo(QSizeF(widthPhysical, widthPhysical));
    }

    QPixmap pointPixmap = style->lineStyle().scaledPointPixmap();
    if(!pointPixmap.isNull())
    {
        if(style->lineStyle().alwaysUsePointPixmap() || (!style->lineStyle().alwaysUsePointPixmap() && lineString->size() <= 2))
        {
            double width_2 = static_cast<double>(pointPixmap.width());
            double height_2 = static_cast<double>(pointPixmap.height());
            size = size.expandedTo(QSizeF(width_2, height_2));
        }
    }

    return size;
}

void GeoLineStringGraphicsItem::getTiles(GeoSceneTextureTileDataset *tileDataset, const TileId &tileId, int zoomLevel, TileMap &tiles, std::atomic<bool> &aCancel)
{
    int count = m_lineString->size();

    QPolygonF tempPolygon(count);
    for(int i = 0; i < count; ++i)
    {
        double lon;
        double lat;
        m_lineString->getLonLat(i, lon, lat, GeoDataCoordinates::Unit::Radian);

        tempPolygon[i] = QPointF(lon, lat);
    }

    getTilesImpl(tileDataset, QVector<QPolygonF>() << tempPolygon, tileId, zoomLevel, tiles, aCancel);
}

namespace
{
qreal normalizeAngle(qreal angle)
{
    angle = fmodf(angle, 360);
    return angle < 0 ? angle + 360 : angle;
}

}

bool
GeoLineStringGraphicsItem::drawPolylineLabel(GeoPainter *painter, const QVector<QPolygonF> &polygons, const ViewportParams *viewport, const QString &labelText, LabelPositionFlags labelPositionFlags, const QColor &labelColor, const QFont &labelFont, int labelStyles, GeoLabelPlaceHandler &placeHandler, const GeoDataFeature*feature)
{    
    if ( labelText.isEmpty() || labelPositionFlags.testFlag( NoLabel ) || polygons.isEmpty() || labelColor == Qt::transparent || !feature)
    {
        return false;
    }

    QFont font = labelFont;
    QFontMetricsF fontMatrix(font);
    QRectF viewportRect = QRectF(QPointF(0, 0), viewport->size());

    if ( labelPositionFlags.testFlag( FollowLine ) )
    {
        foreach( const QPolygonF& itPolygon, polygons )
        {
            if (!itPolygon.boundingRect().intersects(viewportRect)) {
                continue;
            }

            double labelWidth = fontMatrix.width( labelText );

            QPainterPath path;
            path.addPolygon(itPolygon);
            qreal pathLength = path.length();
            if (qFuzzyIsNull(pathLength))
            {
                continue;
            }

            int maxNumLabels = static_cast<int>(pathLength / labelWidth);

            if (maxNumLabels > 0) {
                qreal textRelativeLength = labelWidth / pathLength;
                int numLabels = 1;
                if (maxNumLabels > 1) {
                    numLabels = maxNumLabels/2;
                }
                qreal offset = (1.0 - numLabels*textRelativeLength)/numLabels;
                qreal startPercent = offset/2.0;

                for (int k = 0; k < numLabels; ++k, startPercent += textRelativeLength + offset) {
                    QPointF point = path.pointAtPercent(startPercent);
                    QPointF endPoint = path.pointAtPercent(startPercent + textRelativeLength);

                    if ( viewportRect.contains(point.toPoint()) || viewportRect.contains(endPoint.toPoint()) ) {
                        qreal angle = -path.angleAtPercent(startPercent);
                        qreal angle2 = -path.angleAtPercent(startPercent + textRelativeLength);
                        angle = normalizeAngle(angle);
                        angle2 = normalizeAngle(angle2);
                        bool upsideDown = angle > 90.0 && angle < 270.0;

                        if ( qAbs(angle - angle2) < 3.0 )
                        {
                            if ( upsideDown ) {
                                angle += 180.0;
                                point = path.pointAtPercent(startPercent + textRelativeLength);
                            }

                            QTransform transform;
                            transform.translate( point.x(), point.y() );
                            transform.rotate( angle );

                            QRectF textRect = painter->labelRect(labelText, font, GeoPainter::LabelStyle(labelStyles));
                            QRectF transformedTextRect = transform.mapRect( textRect );

                            if(placeHandler.addLabelRect(transformedTextRect, feature))
                            {
                                QTransform const oldTransform = painter->transform();
                                painter->setTransform(transform, true);
                                painter->drawLabelText( textRect, labelText, font, GeoPainter::LabelStyle(labelStyles), labelColor);
                                painter->setTransform(oldTransform);
                            }
                        }
                        else
                        {
                            for (int i = 0; i < labelText.length(); ++i) {
                                qreal currentGlyphTextLength = fontMatrix.width(labelText.left(i)) / pathLength;

                                if ( !upsideDown ) {
                                    angle = -path.angleAtPercent(startPercent + currentGlyphTextLength);
                                    point = path.pointAtPercent(startPercent + currentGlyphTextLength);
                                }
                                else {
                                    angle = -path.angleAtPercent(startPercent + textRelativeLength - currentGlyphTextLength) + 180;
                                    point = path.pointAtPercent(startPercent + textRelativeLength - currentGlyphTextLength);
                                }

                                QTransform transform;
                                transform.translate( point.x(), point.y() );
                                transform.rotate( angle );

                                QRectF textRect = painter->labelRect(labelText, font, GeoPainter::LabelStyles(labelStyles));
                                QRectF transformedTextRect = transform.mapRect( textRect );

                                if(placeHandler.addLabelRect(transformedTextRect, feature))
                                {
                                    QTransform const oldTransform = painter->transform();
                                    painter->setTransform(transform, true);
                                    painter->drawLabelText( textRect, labelText, font, GeoPainter::LabelStyles(labelStyles), labelColor);
                                    painter->setTransform(oldTransform);
                                }
                            }
                        }
                    }
                }
            }
        }

        return true;
    }
    else
    {
        QRectF labelRect = painter->labelRect(labelText, font, static_cast<GeoPainter::LabelStyle>(labelStyles));

        foreach( const QPolygonF& itPolygon, polygons )
        {
            QVector<QPointF> labelNodes = labelPosition(itPolygon, labelPositionFlags, viewport);
            foreach ( const QPointF& labelNode, labelNodes )
            {
                QPointF labelPosition = labelNode + QPointF( 3.0, -2.0 );

                // FIXME: This is a Q&D fix.
                qreal xmax = viewportRect.width() - 10.0 - labelRect.width();
                if ( labelPosition.x() > xmax )
                {
                    labelPosition.setX( xmax );
                }

                qreal ymin = 10.0 + labelRect.height();

                if ( labelPosition.y() < ymin )
                {
                    labelPosition.setY( ymin );
                }

                qreal ymax = viewportRect.height() - 10.0 - labelRect.height();
                if ( labelPosition.y() > ymax )
                {
                    labelPosition.setY( ymax );
                }


                labelRect.moveTopLeft(labelPosition + labelRect.topLeft());
                qDebug() << "labelRect" << labelRect.topLeft() << labelRect.width() << labelRect.height() << labelText;
                if(placeHandler.addLabelRect(labelRect, feature))
                {
                    painter->drawLabelText( labelRect, labelText, font, static_cast<GeoPainter::LabelStyle>(labelStyles), labelColor );
                    return true;
                }
            }
        }
    }

    return false;
}

namespace
{

QVector<QPolygonF>
getGeoPolygons(const ViewportParams *viewport, const GeoDataLineString *lineString, const GeoDataLatLonAltBox& latLongAltBox)
{
    QVector<QPolygonF> polygons;

//        qWarning() << "Geos Line String:" << GEOSGeomToWKT_r(handler, geosLineString);

    qreal north, south, east, west;
    latLongAltBox.boundaries( north, south, east, west, GeoDataCoordinates::Unit::Degree );

    /* Need to add a margin so we dont have anomilies at the borders */
    double yMargin = (north - south)/100;
    north+=yMargin;
    south-=yMargin;

    double xMargin = (east- west)/100;
    east += xMargin;
    west -= xMargin;

//        qWarning() << "GeoLineStringGraphicsItem::getPolygonsImpl rect" << west << east << north << south;

    QElapsedTimer timer;

    timer.start();

    QPolygonF points = lineString->rawData();

    typedef boost::geometry::model::d2::point_xy<double> point_type;
    typedef boost::geometry::model::box<point_type> box_type;

    box_type box = boost::geometry::make<box_type>(west,
                                                   south,
                                                   east,
                                                   north);

    QVector<QPolygonF> output;
    boost::geometry::intersection(points, box, output);

    for (auto const& l : output)
    {
        QVector<QPolygonF> tempPolygons;
        viewport->screenCoordinates( GeoDataLineString(l, lineString->tessellationFlags()), tempPolygons );
        polygons << tempPolygons;
    }

    return polygons;
}

}

QVector<QPolygonF>
GeoLineStringGraphicsItem::getPolygonsImpl(const ViewportParams *viewport, const GeoDataLineString *lineString)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPolygonF> polygons;
    int size = lineString->size();

    if(viewport->viewLatLonAltBox().united(lineString->latLonAltBox()) == viewport->viewLatLonAltBox() || size <= 1)
    {
        viewport->screenCoordinates( *lineString, polygons);
    }
    else
    {
        if ( viewport->viewLatLonAltBox().west() > viewport->viewLatLonAltBox().east() )
        {
            // Handle boxes crossing the IDL by splitting it into two separate boxes
            GeoDataLatLonAltBox left;
            left.setWest( -M_PI );
            left.setEast( viewport->viewLatLonAltBox().east() );
            left.setNorth( viewport->viewLatLonAltBox().north() );
            left.setSouth( viewport->viewLatLonAltBox().south() );
            left.setMinAltitude(viewport->viewLatLonAltBox().minAltitude());
            left.setMaxAltitude(viewport->viewLatLonAltBox().maxAltitude());

            GeoDataLatLonAltBox right;
            right.setWest( viewport->viewLatLonAltBox().west() );
            right.setEast( M_PI );
            right.setNorth( viewport->viewLatLonAltBox().north() );
            right.setSouth( viewport->viewLatLonAltBox().south() );
            right.setMinAltitude(viewport->viewLatLonAltBox().minAltitude());
            right.setMaxAltitude(viewport->viewLatLonAltBox().maxAltitude());

            polygons = getGeoPolygons( viewport, lineString, left) + getGeoPolygons( viewport, lineString, right);
        }
        else
        {
            polygons = getGeoPolygons(viewport, lineString, viewport->viewLatLonAltBox());
        }
    }

    qDebug() << "GeoLineStringGraphicsItem::getPolygonsImpl finished" << timer.nsecsElapsed();


    return polygons;
}



QVector<QPointF> GeoLineStringGraphicsItem::labelPosition(const QPolygonF & polygon, LabelPositionFlags labelPositionFlags, const ViewportParams *viewport)
{
    QVector<QPointF> labelNodes;

    QRectF viewportRect(QPointF(0,0), viewport->size());
    viewportRect.adjust(m_labelAreaMargin, m_labelAreaMargin, -m_labelAreaMargin, -m_labelAreaMargin);

    if ( labelPositionFlags.testFlag( LineCenter ) ) {
        // The Label at the center of the polyline:
        int labelPosition = static_cast<int>( polygon.size() / 2.0 );
        if ( polygon.size() > 0 ) {
            if ( labelPosition >= polygon.size() ) {
                labelPosition = polygon.size() - 1;
            }
            labelNodes << polygon.at( labelPosition );
        }
    }

    QLineF leftSide(viewportRect.topLeft(), viewportRect.bottomLeft());
    QLineF rightSide(viewportRect.topRight(), viewportRect.bottomRight());
    QLineF topSide(viewportRect.topLeft(), viewportRect.topRight());
    QLineF bottomSide(viewportRect.bottomLeft(), viewportRect.bottomRight());

    if ( polygon.size() > 0 && labelPositionFlags.testFlag( LineStart ) ) {
        if ( pointAllowsLabel( polygon.first(), viewport ) )
        {
            labelNodes << polygon.first();
        }
        else
        {
            // The Label at the start of the polyline:
            for ( int it = 1; it < polygon.size(); ++it )
            {
                QPointF tempPrevPoint = polygon.at( it -1 );
                QPointF tempPoint = polygon.at( it );

                if(viewportRect.contains(tempPrevPoint) ||
                   viewportRect.contains(tempPoint))
                {
                    if ( pointAllowsLabel( polygon.at( it ), viewport ) )
                    {
                        QPointF node = interpolateLabelPoint( polygon.at( it -1 ), polygon.at( it ),
                                                            labelPositionFlags, viewport );
                        if ( node != QPointF( -1.0, -1.0 ) && viewportRect.contains(node))
                        {
                            labelNodes << node;
                            break;
                        }
                    }
                }

                if(tempPrevPoint.x() >= viewportRect.right() && tempPoint.x() >= viewportRect.right())
                {
                    continue;
                }

                if(tempPrevPoint.x() <= viewportRect.left() && tempPoint.x() <= viewportRect.left())
                {
                    continue;
                }

                if(tempPrevPoint.y() >= viewportRect.bottom() && tempPoint.y() >= viewportRect.bottom())
                {
                    continue;
                }

                if(tempPrevPoint.y() <= viewportRect.top() && tempPoint.y() <= viewportRect.top())
                {
                    continue;
                }

                QLineF line(tempPrevPoint, tempPoint);
                QPointF intersectPoint;

                if(tempPoint.x() > tempPrevPoint.x())
                {
                    if(leftSide.intersect(line, &intersectPoint) == QLineF::BoundedIntersection)
                    {
                        labelNodes << intersectPoint;
                        break;
                    }

                    if(rightSide.intersect(line, &intersectPoint) == QLineF::BoundedIntersection)
                    {
                        labelNodes << intersectPoint;
                        break;
                    }
                }
                else
                {
                    if(rightSide.intersect(line, &intersectPoint) == QLineF::BoundedIntersection)
                    {
                        labelNodes << intersectPoint;
                        break;
                    }

                    if(leftSide.intersect(line, &intersectPoint) == QLineF::BoundedIntersection)
                    {
                        labelNodes << intersectPoint;
                        break;
                    }
                }

                if(tempPoint.y() > tempPrevPoint.y())
                {
                    if(topSide.intersect(line, &intersectPoint) == QLineF::BoundedIntersection)
                    {
                        labelNodes << intersectPoint;
                        break;
                    }

                    if(bottomSide.intersect(line, &intersectPoint) == QLineF::BoundedIntersection)
                    {
                        labelNodes << intersectPoint;
                        break;
                    }
                }
                else
                {
                    if(bottomSide.intersect(line, &intersectPoint) == QLineF::BoundedIntersection)
                    {
                        labelNodes << intersectPoint;
                        break;
                    }

                    if(topSide.intersect(line, &intersectPoint) == QLineF::BoundedIntersection)
                    {
                        labelNodes << intersectPoint;
                        break;
                    }
                }
            }
        }
    }

    if ( polygon.size() > 1 && labelPositionFlags.testFlag( LineEnd ) ) {
        if ( pointAllowsLabel( polygon.at( polygon.size() - 1 ), viewport ) ) {
            labelNodes << polygon.at( polygon.size() - 1 );
        }

        // The Label at the end of the polyline:
        for ( int it = polygon.size() - 2; it > 0; --it ) {
            QPointF node = interpolateLabelPoint( polygon.at( it + 1 ), polygon.at( it ),
                                                labelPositionFlags, viewport );
            if ( node != QPointF( -1.0, -1.0 ) && viewportRect.contains(node)) {
                labelNodes << node;
                break;
            }
        }
    }

    return labelNodes;
}

bool GeoLineStringGraphicsItem::pointAllowsLabel( const QPointF& point, const ViewportParams *viewport)
{

    if ( point.x() > m_labelAreaMargin && point.x() < viewport->width() - m_labelAreaMargin
         && point.y() > m_labelAreaMargin && point.y() < viewport->height() - m_labelAreaMargin ) {
        return true;
    }
    return false;
}

namespace
{
double
_m( const QPointF & start, const QPointF & end )
{
    qreal  divisor = end.x() - start.x();
    if ( std::fabs( divisor ) < 0.000001 ) {
        // this is in screencoordinates so the difference
        // between 0, 0.000001 and -0.000001 isn't visible at all
        divisor = 0.000001;
    }

    return ( end.y() - start.y() )
         / divisor;

}
}

QPointF GeoLineStringGraphicsItem::interpolateLabelPoint(const QPointF& previousPoint,
                                                          const QPointF& currentPoint,
                                                          LabelPositionFlags labelPositionFlags,
                                                          const ViewportParams *viewport )
{
    qreal m = _m( previousPoint, currentPoint );
    if ( previousPoint.x() <= m_labelAreaMargin ) {
        if ( labelPositionFlags.testFlag( IgnoreXMargin ) ) {
            return QPointF( -1.0, -1.0 );
        }
        return QPointF( m_labelAreaMargin,
                        previousPoint.y() + ( m_labelAreaMargin - previousPoint.x() ) * m );
    }
    else if ( previousPoint.x() >= viewport->width() - m_labelAreaMargin  ) {
        if ( labelPositionFlags.testFlag( IgnoreXMargin ) ) {
            return QPointF( -1.0, -1.0 );
        }
        return QPointF( viewport->width() - m_labelAreaMargin,
                        previousPoint.y() -
                        ( previousPoint.x() - viewport->width() + m_labelAreaMargin ) * m );
    }

    if ( previousPoint.y() <= m_labelAreaMargin ) {
        if ( labelPositionFlags.testFlag( IgnoreYMargin ) ) {
            return QPointF( -1.0, -1.0 );
        }
        return QPointF( previousPoint.x() + ( m_labelAreaMargin - previousPoint.y() ) / m,
                        m_labelAreaMargin );
    }
    else if ( previousPoint.y() >= viewport->height() - m_labelAreaMargin  ) {
        if ( labelPositionFlags.testFlag( IgnoreYMargin ) ) {
            return QPointF( -1.0, -1.0 );
        }
        return QPointF(   previousPoint.x() -
                        ( previousPoint.y() - viewport->height() + m_labelAreaMargin ) / m,
                          viewport->height() - m_labelAreaMargin );
    }

//    mDebug() << Q_FUNC_INFO << "Previous and current node position are allowed!";

    return QPointF( -1.0, -1.0 );
}

void GeoLineStringGraphicsItem::getTilesImpl(GeoSceneTextureTileDataset *tileDataset, const QVector<QPolygonF> &tempPolygon, const TileId &tileId, int zoomLevel, TileMap & tiles, std::atomic<bool> &aCancel)
{
    if(aCancel)
    {
        return;
    }

    double radius = ( 1 << tileId.tileLevel() ) *tileDataset->levelZeroColumns()  / 2.0;

    double lonLeft   = ( tileId.x() - radius ) / radius * M_PI;
    double lonRight  = ( tileId.x() - radius + 1 ) / radius * M_PI;

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
        latBottom = atan( sinh( ( radius - tileId.y()) / radius * M_PI ) );
        break;
    }

    typedef boost::geometry::model::d2::point_xy<double> point_type;
    typedef boost::geometry::model::box<point_type> box_type;

    box_type box = boost::geometry::make<box_type>(lonLeft,
                                                   latTop,
                                                   lonRight,
                                                   latBottom);

    QVector<QPolygonF> output;
    foreach(const QPolygonF &line, tempPolygon)
    {
        QVector<QPolygonF> tempOutput;
        bool intersect = boost::geometry::intersection(line, box, tempOutput);
        if(intersect)
        {
            output << tempOutput;
        }
    }

    if(!output.isEmpty())
    {
        if (tileId.tileLevel()< zoomLevel)
        {
            TileId tile1(tileId.mapThemeIdHash(), tileId.tileLevel()+1, tileId.x()*2, tileId.y()*2);
            TileId tile2(tileId.mapThemeIdHash(), tileId.tileLevel()+1, (tileId.x()*2) + 1, tileId.y()*2);
            TileId tile3(tileId.mapThemeIdHash(), tileId.tileLevel()+1, tileId.x()*2, (tileId.y()*2)+1);
            TileId tile4(tileId.mapThemeIdHash(), tileId.tileLevel()+1, (tileId.x()*2) + 1, (tileId.y()*2)+1);

            getTilesImpl(tileDataset, output, tile1, zoomLevel, tiles, aCancel);
            getTilesImpl(tileDataset, output, tile2, zoomLevel, tiles, aCancel);
            getTilesImpl(tileDataset, output, tile3, zoomLevel, tiles, aCancel);
            getTilesImpl(tileDataset, output, tile4, zoomLevel, tiles, aCancel);
        }
        else
        {
            tiles[tileId.tileLevel()][tileId.x()][tileId.y()] = Marble::TileStatus::Partially;
        }
    }
}

} // namespace
