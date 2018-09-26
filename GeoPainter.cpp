#define QT_NO_DEBUG_OUTPUT
//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2006-2009 Torsten Rahn <tackat@kde.org>


#include "GeoPainter.h"
#include "GeoPainter_p.h"

#include <QtCore/QList>
#include <QtGui/QPainterPath>
#include <QtGui/QGuiApplication>
#include <QtGui/QPalette>
#include <QtGui/QRegion>
#include <QtCore/qmath.h>

#include "MarbleDebug.h"

#include "geodata/data/GeoDataCoordinates.h"
#include "geodata/data/GeoDataLineString.h"
#include "geodata/data/GeoDataLinearRing.h"
#include "geodata/data/GeoDataPoint.h"
#include "geodata/data/GeoDataPolygon.h"

#include "MarbleGlobal.h"
#include "ViewportParams.h"
#include "projections/AbstractProjection.h"

// #define MARBLE_DEBUG

using namespace Marble;

GeoPainterPrivate::GeoPainterPrivate( const ViewportParams *viewport )
        : m_viewport( viewport ),
        m_mapQuality( LowQuality ),
        m_x( new qreal[100] )
{
}

GeoPainterPrivate::~GeoPainterPrivate()
{
    delete[] m_x;
}

void GeoPainterPrivate::createAnnotationLayout (  qreal x, qreal y,
                                                  QSizeF bubbleSize,
                                                  qreal bubbleOffsetX, qreal bubbleOffsetY,
                                                  qreal xRnd, qreal yRnd,
                                                  QPainterPath& path, QRectF& rect )
{
    // TODO: MOVE this into an own Annotation class
    qreal arrowPosition = 0.3;
    qreal arrowWidth = 12.0;

    qreal width =  bubbleSize.width();
    qreal height = bubbleSize.height();

    qreal dx =  ( bubbleOffsetX > 0 ) ? 1.0 : -1.0; // x-Mirror
    qreal dy =  ( bubbleOffsetY < 0 ) ? 1.0 : -1.0; // y-Mirror

    qreal x0 =  ( x + bubbleOffsetX ) - dx * ( 1.0 - arrowPosition ) * ( width - 2.0 * xRnd ) - xRnd *dx;
    qreal x1 =  ( x + bubbleOffsetX ) - dx * ( 1.0 - arrowPosition ) * ( width - 2.0 * xRnd );
    qreal x2 =  ( x + bubbleOffsetX ) - dx * ( 1.0 - arrowPosition ) * ( width - 2.0 * xRnd ) + xRnd * dx;
    qreal x3 =  ( x + bubbleOffsetX ) - dx * arrowWidth / 2.0;
    qreal x4 =  ( x + bubbleOffsetX ) + dx * arrowWidth / 2.0;
    qreal x5 =  ( x + bubbleOffsetX ) + dx * arrowPosition * ( width - 2.0 * xRnd )- xRnd * dx;
    qreal x6 =  ( x + bubbleOffsetX ) + dx * arrowPosition * ( width - 2.0 * xRnd );
    qreal x7 =  ( x + bubbleOffsetX ) + dx * arrowPosition * ( width - 2.0 * xRnd ) + xRnd * dx;

    qreal y0 =  ( y + bubbleOffsetY );
    qreal y1 =  ( y + bubbleOffsetY ) - dy * yRnd;
    qreal y2 =  ( y + bubbleOffsetY ) - dy * 2 * yRnd;
    qreal y5 =  ( y + bubbleOffsetY ) - dy * ( height - 2 * yRnd );
    qreal y6 =  ( y + bubbleOffsetY ) - dy * ( height - yRnd );
    qreal y7 =  ( y + bubbleOffsetY ) - dy * height;

    QPointF p1 ( x, y ); // pointing point
    QPointF p2 ( x4, y0 );
    QPointF p3 ( x6, y0 );
    QPointF p4 ( x7, y1 );
    QPointF p5 ( x7, y6 );
    QPointF p6 ( x6, y7 );
    QPointF p7 ( x1, y7 );
    QPointF p8 ( x0, y6 );
    QPointF p9 ( x0, y1 );
    QPointF p10( x1, y0 );
    QPointF p11( x3, y0 );

    QRectF bubbleBoundingBox(  QPointF( x0, y7 ), QPointF( x7, y0 ) );

    path.moveTo( p1 );
    path.lineTo( p2 );

    path.lineTo( p3 );
    QRectF bottomRight( QPointF( x5, y2 ), QPointF( x7, y0 ) );
    path.arcTo( bottomRight, 270.0, 90.0 );

    path.lineTo( p5 );
    QRectF topRight( QPointF( x5, y7 ), QPointF( x7, y5 ) );
    path.arcTo( topRight, 0.0, 90.0 );

    path.lineTo( p7 );
    QRectF topLeft( QPointF( x0, y7 ), QPointF( x2, y5 ) );
    path.arcTo( topLeft, 90.0, 90.0 );

    path.lineTo( p9 );
    QRectF bottomLeft( QPointF( x0, y2 ), QPointF( x2, y0 ) );
    path.arcTo( bottomLeft, 180.0, 90.0 );

    path.lineTo( p10 );
    path.lineTo( p11 );
    path.lineTo( p1 );

    qreal left   = ( dx > 0 ) ? x1 : x6;
    qreal right  = ( dx > 0 ) ? x6 : x1;
    qreal top    = ( dy > 0 ) ? y6 : y1;
    qreal bottom = ( dy > 0 ) ? y1 : y6;

    rect.setTopLeft( QPointF( left, top ) );
    rect.setBottomRight( QPointF( right, bottom ) );
}

GeoDataLinearRing GeoPainterPrivate::createLinearRingFromGeoRect( const GeoDataCoordinates & centerCoordinates,
                                                                  qreal width, qreal height )
{
    qreal lon = 0.0;
    qreal lat = 0.0;
    qreal altitude = centerCoordinates.altitude();
    centerCoordinates.geoCoordinates( lon, lat, GeoDataCoordinates::Degree );

    lon = GeoDataCoordinates::normalizeLon( lon, GeoDataCoordinates::Degree );
    lat = GeoDataCoordinates::normalizeLat( lat, GeoDataCoordinates::Degree );

    qreal west = GeoDataCoordinates::normalizeLon( lon - width * 0.5, GeoDataCoordinates::Degree );
    qreal east =  GeoDataCoordinates::normalizeLon( lon + width * 0.5, GeoDataCoordinates::Degree );

    qreal north = GeoDataCoordinates::normalizeLat( lat + height * 0.5, GeoDataCoordinates::Degree );
    qreal south = GeoDataCoordinates::normalizeLat( lat - height * 0.5, GeoDataCoordinates::Degree );

    GeoDataCoordinates southWest( west, south,
                                  altitude, GeoDataCoordinates::Degree );
    GeoDataCoordinates southEast( east, south,
                                  altitude, GeoDataCoordinates::Degree );
    GeoDataCoordinates northEast( east, north,
                                  altitude, GeoDataCoordinates::Degree );
    GeoDataCoordinates northWest( west, north,
                                  altitude, GeoDataCoordinates::Degree );

    // If the width of the rect is larger as 180 degree, we have to enforce the long way.
    if ( width >= 180 ) {
        qreal center = lon;
        GeoDataCoordinates southCenter( center, south, altitude, GeoDataCoordinates::Degree );
        GeoDataCoordinates northCenter( center, north, altitude, GeoDataCoordinates::Degree );

        return GeoDataLinearRing(QVector<GeoDataCoordinates>() << southWest << southCenter << southEast << northEast << northCenter << northWest, Tessellate | RespectLatitudeCircle);
    }

    return GeoDataLinearRing(QVector<GeoDataCoordinates>() << southWest << southEast << northEast << northWest, Tessellate | RespectLatitudeCircle);
}

qreal GeoPainterPrivate::normalizeAngle(qreal angle)
{
    angle = fmodf(angle, 360);
    return angle < 0 ? angle + 360 : angle;
}

// -------------------------------------------------------------------------------------------------

GeoPainter::GeoPainter( QPaintDevice* pd, const ViewportParams *viewport, MapQuality mapQuality )
    : ClipPainter( pd ),
      d( new GeoPainterPrivate( viewport) )
{
    setMapQuality(mapQuality);
    const bool antialiased = mapQuality == HighQuality || mapQuality == PrintQuality;
    setRenderHint( QPainter::Antialiasing, antialiased );
}


GeoPainter::~GeoPainter()
{
    delete d;
}


MapQuality GeoPainter::mapQuality() const
{
    return d->m_mapQuality;
}

void GeoPainter::setMapQuality(MapQuality quality)
{
    if(d->m_mapQuality == quality)
    {
        return;
    }

    d->m_mapQuality = quality;
    setRenderHint( QPainter::Antialiasing, (quality == HighQuality || quality == PrintQuality) );
}


void GeoPainter::drawAnnotation( const GeoDataCoordinates & position,
                                 const QString & text, QSizeF bubbleSize,
                                 qreal bubbleOffsetX, qreal bubbleOffsetY,
                                 qreal xRnd, qreal yRnd )
{
    qreal y;
    bool globeHidesPoint;

    if ( bubbleSize.height() <= 0 ) {
        QRectF rect = QRectF( QPointF( 0.0, 0.0 ), bubbleSize - QSizeF( 2 * xRnd, 0.0 ) );
        qreal idealTextHeight = boundingRect( rect, Qt::TextWordWrap, text ).height();
        bubbleSize.setHeight( 2 * yRnd + idealTextHeight );
    }

    QVector<double> m_x;
    bool visible = d->m_viewport->screenCoordinates( position, m_x, y, QSizeF(), globeHidesPoint );

    if ( visible ) {
        // Draw all the x-repeat-instances of the point on the screen
        for( int it = 0; it < m_x.size(); ++it ) {
            QPainterPath path;
            QRectF rect;
            d->createAnnotationLayout( d->m_x[it], y, bubbleSize, bubbleOffsetX, bubbleOffsetY,  xRnd, yRnd, path, rect );
            QPainter::drawPath( path );
            QPainter::drawText( rect, Qt::TextWordWrap, text, &rect );
        }
    }
}


void GeoPainter::drawPoint (  const GeoDataCoordinates & position )
{
    qreal y;
    bool globeHidesPoint;

    QVector<double> m_x;

    bool visible = d->m_viewport->screenCoordinates( position, m_x, y, QSizeF(), globeHidesPoint );

    if ( visible ) {
        // Draw all the x-repeat-instances of the point on the screen
        for( int it = 0; it < m_x.size(); ++it ) {
            QPainter::drawPoint( QPointF(d->m_x[it], y) );
        }
    }
}



void GeoPainter::drawText ( const GeoDataCoordinates & position,
                            const QString & text,
                            int xOffset, int yOffset,
                            int width, int height,
                            const QTextOption & option )
{
    // Of course in theory we could have the "isGeoProjected" parameter used
    // for drawText as well. However this would require us to convert all
    // glyphs to PainterPaths / QPolygons. From QPolygons we could create
    // GeoDataPolygons which could get painted on screen. Any patches appreciated ;-)

    qreal y;
    bool globeHidesPoint;

    QSizeF textSize( fontMetrics().width( text ), fontMetrics().height() );  

    QVector<double> m_x;

    bool visible = d->m_viewport->screenCoordinates( position, m_x, y, textSize, globeHidesPoint );

    if ( visible ) {
        // Draw all the x-repeat-instances of the point on the screen
        for( int it = 0; it < m_x.size(); ++it ) {
            if (width == 0 && height == 0) {
                QPainter::drawText( m_x[it] + xOffset, y + yOffset,  text );
            }
            else {
                QRectF boundingRect(d->m_x[it] + xOffset, y + yOffset, width, height);
                QPainter::drawText( boundingRect, text, option );
            }
        }
    }
}


void GeoPainter::drawEllipse ( const GeoDataCoordinates & centerPosition,
                               qreal width, qreal height,
                               bool isGeoProjected )
{
    if ( !isGeoProjected ) {
        qreal y;
        bool globeHidesPoint;

        QVector<double> m_x;

        bool visible = d->m_viewport->screenCoordinates( centerPosition, m_x, y, QSizeF( width, height ), globeHidesPoint );

        if ( visible ) {
            // Draw all the x-repeat-instances of the point on the screen
            for( int it = 0; it < m_x.size(); ++it ) {
                QPainter::drawEllipse(  m_x[it] - width / 2.0,
                                        y - height / 2.0, width, height  );
            }
        }
    }
    else {
        // Initialize variables
        const qreal centerLon = centerPosition.longitude( GeoDataCoordinates::Degree );
        const qreal centerLat = centerPosition.latitude( GeoDataCoordinates::Degree );
        const qreal altitude = centerPosition.altitude();

        // Ensure a valid latitude range: 
        if ( centerLat + 0.5 * height > 90.0 || centerLat - 0.5 * height < -90.0 ) {
            return;
        }

        // Don't show the ellipse if it's too small:
        GeoDataLatLonBox ellipseBox( centerLat + 0.5 * height, centerLat - 0.5 * height,
                                     centerLon + 0.5 * width,  centerLon - 0.5 * width, 
                                     GeoDataCoordinates::Degree );
        if ( !d->m_viewport->viewLatLonAltBox().intersects( ellipseBox ) ||
             !d->m_viewport->resolves( ellipseBox ) ) return;

        QVector<GeoDataCoordinates> coordinates;

        // Optimizing the precision by determining the size which the 
        // ellipse covers on the screen:
        const qreal degreeResolution = d->m_viewport->angularResolution() * RAD2DEG;
        // To create a circle shape even for very small precision we require uneven numbers:
        const int precision = qMin<qreal>( width / degreeResolution / 8 + 1, 81 );

        // Calculate the shape of the upper half of the ellipse:
        for ( int i = 0; i <= precision; ++i ) {
            const qreal t = 1.0 - 2.0 * (qreal)(i) / (qreal)(precision);
            const qreal lat = centerLat + 0.5 * height * sqrt( 1.0 - t * t );
            const qreal lon = centerLon + 0.5 * width * t;
            coordinates << GeoDataCoordinates( lon, lat, altitude, GeoDataCoordinates::Degree );
        }
        // Calculate the shape of the lower half of the ellipse:
        for ( int i = 0; i <= precision; ++i ) {
            const qreal t = 2.0 * (qreal)(i) / (qreal)(precision) -  1.0;
            const qreal lat = centerLat - 0.5 * height * sqrt( 1.0 - t * t );
            const qreal lon = centerLon + 0.5 * width * t;
            coordinates << GeoDataCoordinates( lon, lat, altitude, GeoDataCoordinates::Degree );
        }
        
        drawPolygon( GeoDataLinearRing(coordinates) );

    }

}


void GeoPainter::drawImage ( const GeoDataCoordinates & centerPosition,
                             const QImage & image /*, bool isGeoProjected */ )
{
    // isGeoProjected = true would project the image/pixmap onto the globe. This
    // requires to deal with the TextureMapping classes -> should get
    // implemented later on

    qreal y;
    bool globeHidesPoint;
    QVector<double> m_x;
    bool visible = d->m_viewport->screenCoordinates( centerPosition, m_x, y, image.size(), globeHidesPoint );

    if(visible)
    {

       // Draw all the x-repeat-instances of the point on the screen
        for( int it = 0; it < m_x.size(); ++it ) {
            QPainter::drawImage(QPointF(m_x[it] - ( image.width() / 2 ), y - ( image.height() / 2 ) ), image );
        }
    }
}


void GeoPainter::drawPixmap ( const GeoDataCoordinates & centerPosition,
                              const QPixmap & pixmap /* , bool isGeoProjected */ )
{
    qreal y;
    bool globeHidesPoint;

    QVector<double> m_x;
    bool visible = d->m_viewport->screenCoordinates( centerPosition, m_x, y, pixmap.size(), globeHidesPoint );

    if ( visible ) {
        // Draw all the x-repeat-instances of the point on the screen
        for( int it = 0; it < m_x.size(); ++it ) {
            QPainter::drawPixmap( QPointF(m_x[it] - ( pixmap.width() / 2 ), y - ( pixmap.height() / 2 ) ), pixmap );
        }
    }
}


void GeoPainter::drawPolygon ( const GeoDataLinearRing & linearRing,
                               Qt::FillRule fillRule )
{
    // Immediately leave this method now if:
    // - the object is not visible in the viewport or if
    // - the size of the object is below the resolution of the viewport
    if ( ! d->m_viewport->viewLatLonAltBox().intersects( linearRing.latLonAltBox() ) ||
         ! d->m_viewport->resolves( linearRing.latLonAltBox() )
        )
    {
        // mDebug() << "Polygon doesn't get displayed on the viewport";
        return;
    }

    QVector<QPolygonF> polygons;
    d->m_viewport->screenCoordinates( linearRing, polygons );

    foreach( QPolygonF itPolygon, polygons ) {
        ClipPainter::drawPolygon( itPolygon, fillRule );
    }
}



void GeoPainter::drawPolygon ( const GeoDataPolygon & polygon,
                               Qt::FillRule fillRule )
{
    // If the object is not visible in the viewport return 
    if ( ! d->m_viewport->viewLatLonAltBox().intersects( polygon.outerBoundary().latLonAltBox() ) ||
    // If the size of the object is below the resolution of the viewport then return
         ! d->m_viewport->resolves( polygon.outerBoundary().latLonAltBox() )
        )
    {
        // mDebug() << "Polygon doesn't get displayed on the viewport";
        return;
    }
    // mDebug() << "Drawing Polygon";

    QVector<QPolygonF> outerPolygons;
    QVector<QPolygonF> innerPolygons;
    d->m_viewport->screenCoordinates( polygon.outerBoundary(), outerPolygons );

    QPen const oldPen = pen();

    // When inner boundaries exist, the outline of the polygon must be painted
    // separately to avoid connections between the outer and inner boundaries
    // To avoid performance penalties the separate painting is only done when
    // it's really needed. See review 105019 for details.
    bool const hasInnerBoundaries = !polygon.innerBoundaries().isEmpty();
    bool innerBoundariesOnScreen = false;

    if ( hasInnerBoundaries ) {
        QVector<GeoDataLinearRing> innerBoundaries = polygon.innerBoundaries();

        const GeoDataLatLonAltBox & viewLatLonAltBox = d->m_viewport->viewLatLonAltBox();
        foreach( const GeoDataLinearRing& itInnerBoundary, innerBoundaries ) {
            if ( viewLatLonAltBox.intersects(itInnerBoundary.latLonAltBox())
                 && d->m_viewport->resolves(itInnerBoundary.latLonAltBox()) )  {
                innerBoundariesOnScreen = true;
                break;
            }
        }

        if (innerBoundariesOnScreen) {
            // Cut the outer polygons to the viewport
            QVector<QPointF> viewportPolygon = QPolygonF(QRectF(0, 0, d->m_viewport->width(), d->m_viewport->height()));
            foreach(QPolygonF outerPolygon, outerPolygons) {
                outerPolygon = outerPolygon.intersected(QPolygonF(viewportPolygon));
            }

            setPen( QPen( Qt::NoPen ) );

            // Create the inner screen polygons
            foreach( const GeoDataLinearRing& itInnerBoundary, innerBoundaries ) {
                QVector<QPolygonF> innerPolygonsPerBoundary;

                d->m_viewport->screenCoordinates( itInnerBoundary, innerPolygonsPerBoundary );

                foreach( QPolygonF innerPolygonPerBoundary, innerPolygonsPerBoundary ) {
                    innerPolygons << innerPolygonPerBoundary;
                }
            }
        }
    }

    foreach( QPolygonF outerPolygon, outerPolygons ) {
        if (hasInnerBoundaries && innerBoundariesOnScreen) {
            QRegion clip(outerPolygon.toPolygon());

            foreach(QPolygonF innerPolygon, innerPolygons) {
                clip-=QRegion(innerPolygon.toPolygon());
            }
            ClipPainter::setClipRegion(clip);
        }
        ClipPainter::drawPolygon(outerPolygon, fillRule );
    }

    if ( hasInnerBoundaries && innerBoundariesOnScreen ) {
        setPen( oldPen );
        foreach( const QPolygonF outerPolygon, outerPolygons ) {
            ClipPainter::drawPolyline( outerPolygon );
        }
        foreach( const QPolygonF innerPolygon, innerPolygons ) {
            ClipPainter::drawPolyline( innerPolygon );
        }
    }
}


void GeoPainter::drawRect ( const GeoDataCoordinates & centerCoordinates,
                            qreal width, qreal height,
                            bool isGeoProjected )
{
    if ( !isGeoProjected ) {
        qreal y;
        bool globeHidesPoint;

        QVector<double> m_x;
        bool visible = d->m_viewport->screenCoordinates( centerCoordinates, m_x, y, QSizeF( width, height ), globeHidesPoint );

        if ( visible ) {
            // Draw all the x-repeat-instances of the point on the screen
            for( int it = 0; it < m_x.size(); ++it ) {
                QPainter::drawRect(QRectF( m_x[it] - ( width / 2.0 ), y - ( height / 2.0 ), width, height) );
            }
        }
    }
    else {
        drawPolygon( d->createLinearRingFromGeoRect( centerCoordinates, width, height ),
                     Qt::OddEvenFill );
    }
}


void GeoPainter::drawRoundRect ( const GeoDataCoordinates &centerPosition,
                                 int width, int height,
                                 int xRnd, int yRnd )
{
    qreal y;
    bool globeHidesPoint;

    // FIXME: Better visibility detection that takes the circle geometry into account
    QVector<double> m_x;
    bool visible = d->m_viewport->screenCoordinates( centerPosition, m_x, y, QSizeF( width, height ), globeHidesPoint );

    if ( visible ) {
        // Draw all the x-repeat-instances of the point on the screen
        for( int it = 0; it < m_x.size(); ++it ) {
            QPainter::drawRoundRect( QRectF( m_x[it] - ( width / 2 ), y - ( height / 2 ), width, height), xRnd, yRnd);
        }
    }
}

QRectF
GeoPainter::labelRect(const QString &text, const QFont &labelFont, LabelStyles labelStyle)
{
    QRectF textRect;
    if(labelStyle.testFlag(Glow))
    {
        QFont font = labelFont;
        font.setWeight( 75 );

        QFontMetricsF metrics = QFontMetricsF( font);
        return QRectF( 0, 0, metrics.width( text ), metrics.height() );
    }

    QFontMetricsF metrics = QFontMetricsF( labelFont);
    return QRectF( 0, 0, metrics.width( text ), metrics.height() );
}

void
GeoPainter::drawLabelText(const QRectF &textRect, const QString &text,
                          const QFont &labelFont, LabelStyles labelStyles, const QColor &color )
{
    qDebug() << "labelRect" << textRect << text;
    static const qreal s_labelOutlineWidth = 2.5;

    QFont font = labelFont;

    QFont oldFont = this->font();

    if(labelStyles.testFlag(Highlighted))
    {
        QPen oldPen = pen();
        setPen( color );

        fillRect( textRect, QGuiApplication::palette().highlight() );
        setPen( QPen( QGuiApplication::palette().highlightedText(), 1 ) );
    }

    if(labelStyles.testFlag(Glow))
    {
        font.setWeight( 75 );
        double fontAscent = QFontMetricsF( font ).ascent();

        QPen outlinepen( color == QColor( Qt::white ) ? Qt::black : Qt::white );
        outlinepen.setWidthF( s_labelOutlineWidth );
        QBrush  outlinebrush( color );

        QPainterPath outlinepath;

        const QPointF  baseline = QPointF( s_labelOutlineWidth / 2.0, fontAscent ) + textRect.topLeft();
        outlinepath.addText( baseline, font, text );

        QPen oldPen = pen();
        QBrush oldBrush = brush();

        setPen( outlinepen );
        setBrush( outlinebrush );
        drawPath( outlinepath );
        setPen( Qt::NoPen );
        drawPath( outlinepath );

        setPen(oldPen);
        setBrush(oldBrush);
    }
    else
    {
        QPen oldPen = pen();
        setPen( color );

        drawText(textRect, text);

        setPen(oldPen);
    }

    setFont( oldFont );
}
