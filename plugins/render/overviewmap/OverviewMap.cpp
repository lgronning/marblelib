//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008 Torsten Rahn <tackat@kde.org>
//

#include "OverviewMap.h"

#include <QRect>
#include <QStringList>
#include <QCursor>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QtSvg/QSvgRenderer>
#include <QColorDialog>

#include "MarbleDirs.h"
#include "MarbleDebug.h"
#include "MarbleModel.h"

#include "geodata/data/GeoDataPoint.h"
#include "ViewportParams.h"
#include "MarbleWidget.h"
#include "Planet.h"
#include "PlanetFactory.h"

namespace Marble
{

OverviewMap::OverviewMap()
    : AbstractFloatItem( nullptr ),
      m_mapChanged( false )
{
}

OverviewMap::OverviewMap( const MarbleModel *marbleModel )
    : AbstractFloatItem( marbleModel, QPointF( 15.5, 40.5 ), QSizeF( 166.0, 86.0 ) ),
      m_target(),
      m_posColor(Qt::red),
      m_defaultSize( AbstractFloatItem::size() ),
      m_mapChanged( false )
{
    // cache is no needed because:
    // (1) the SVG overview map is already rendered and stored in m_worldmap pixmap
    // (2) bounding box and location dot keep changing during navigation
    setCacheMode( NoCache );

    restoreDefaultSettings();
}

OverviewMap::~OverviewMap()
= default;

QStringList OverviewMap::backendTypes() const
{
    return QStringList( QStringLiteral("overviewmap") );
}

QString OverviewMap::name() const
{
    return tr("Overview Map");
}

QString OverviewMap::guiString() const
{
    return tr("&Overview Map");
}

QString OverviewMap::nameId() const
{
    return QStringLiteral( "overviewmap" );
}

QString OverviewMap::version() const
{
    return QStringLiteral("1.0");
}

QString OverviewMap::description() const
{
    return tr("This is a float item that provides an overview map.");
}

QString OverviewMap::copyrightYears() const
{
    return QStringLiteral("2008");
}

QList<PluginAuthor> OverviewMap::pluginAuthors() const
{
    return QList<PluginAuthor>()
            << PluginAuthor( QStringLiteral("Torsten Rahn"), QStringLiteral("tackat@kde.org") );
}

QIcon OverviewMap::icon () const
{
    return QIcon(":/icons/worldmap.png");
}

void OverviewMap::initialize ()
{
}

bool OverviewMap::isInitialized () const
{
    return true;
}

void OverviewMap::setProjection( const ViewportParams *viewport )
{
    GeoDataLatLonAltBox latLonAltBox = viewport->latLonAltBox( QRect( QPoint( 0, 0 ), viewport->size() ) );
    const qreal centerLon = viewport->centerLongitude();
    const qreal centerLat = viewport->centerLatitude();
    QString target = marbleModel()->planetId();

    if ( target != m_target ) {
        changeBackground( );
        m_target = target;
        update();
    }

    if ( !( m_latLonAltBox == latLonAltBox
            && m_centerLon == centerLon
            && m_centerLat == centerLat ) )
    {
        m_latLonAltBox = latLonAltBox;
        m_centerLon = centerLon;
        m_centerLat = centerLat;
        update();
    }

    AbstractFloatItem::setProjection( viewport );
}

void OverviewMap::paintContent( QPainter *painter )
{
    painter->save();

    QRectF mapRect( contentRect() );

    // Rerender worldmap pixmap if the size or map has changed
    if ( m_worldmap.size() != mapRect.size().toSize() || m_mapChanged ) {
        m_mapChanged = false;
        m_worldmap = QPixmap( mapRect.size().toSize() );
        m_worldmap.fill( Qt::transparent );
        QPainter mapPainter;
        mapPainter.begin( &m_worldmap );
        mapPainter.setViewport( m_worldmap.rect() );
        QFile file(QStringLiteral(":marble/svg/worldmap.svg"));
        if(file.open(QIODevice::ReadOnly))
        {
            QSvgRenderer renderer(file.readAll());

            renderer.render( &mapPainter );
        }

        mapPainter.end();
    }

    painter->drawPixmap( QPoint( 0, 0 ), m_worldmap );

    // Now draw the latitude longitude bounding box
    qreal xWest = mapRect.width() / 2.0 
                    + mapRect.width() / ( 2.0 * M_PI ) * m_latLonAltBox.west();
    qreal xEast = mapRect.width() / 2.0
                    + mapRect.width() / ( 2.0 * M_PI ) * m_latLonAltBox.east();
    qreal xNorth = mapRect.height() / 2.0 
                    - mapRect.height() / M_PI * m_latLonAltBox.north();
    qreal xSouth = mapRect.height() / 2.0
                    - mapRect.height() / M_PI * m_latLonAltBox.south();

    qreal lon = m_centerLon;
    qreal lat = m_centerLat;
    GeoDataCoordinates::normalizeLonLat( lon, lat );
    qreal x = mapRect.width() / 2.0 + mapRect.width() / ( 2.0 * M_PI ) * lon;
    qreal y = mapRect.height() / 2.0 - mapRect.height() / M_PI * lat;

    painter->setPen( QPen( Qt::white ) );
    painter->setBrush( QBrush( Qt::transparent ) );
    painter->setRenderHint( QPainter::Antialiasing, false );

    qreal boxWidth  = xEast  - xWest;
    qreal boxHeight = xSouth - xNorth;

    qreal minBoxSize = 2.0;
    if ( boxHeight < minBoxSize ) boxHeight = minBoxSize;

    if ( m_latLonAltBox.west() <= m_latLonAltBox.east() ) {
        // Make sure the latLonBox is still visible
        if ( boxWidth  < minBoxSize ) boxWidth  = minBoxSize;

        painter->drawRect( QRectF( xWest, xNorth, boxWidth, boxHeight ) );
    }
    else {
        // If the dateline is shown in the viewport  and if the poles are not 
        // then there are two boxes that represent the latLonBox of the view.

        boxWidth = xEast;

        // Make sure the latLonBox is still visible
        if ( boxWidth  < minBoxSize ) boxWidth  = minBoxSize;

        painter->drawRect( QRectF( 0, xNorth, boxWidth, boxHeight ) );

        boxWidth = mapRect.width() - xWest;

        // Make sure the latLonBox is still visible
        if ( boxWidth  < minBoxSize ) boxWidth  = minBoxSize;

        painter->drawRect( QRectF( xWest, xNorth, boxWidth, boxHeight ) );
    }

    painter->setPen( QPen( m_posColor ) );
    painter->setBrush( QBrush( m_posColor ) );

    qreal circleRadius = 2.5;

    painter->setRenderHint( QPainter::Antialiasing, true );
    painter->drawEllipse( QRectF( x - circleRadius, y - circleRadius , 2 * circleRadius, 2 * circleRadius ) );

    painter->restore();
}

QHash<QString,QVariant> OverviewMap::settings() const
{
    QHash<QString, QVariant> result = AbstractFloatItem::settings();

    typedef QHash<QString, QVariant>::ConstIterator Iterator;
    Iterator end = m_settings.constEnd();
    for ( Iterator iter = m_settings.constBegin(); iter != end; ++iter ) {
        result.insert( iter.key(), iter.value() );
    }

    return result;
}

void OverviewMap::setSettings( const QHash<QString,QVariant> &settings )
{
    AbstractFloatItem::setSettings( settings );

    m_settings.insert( QStringLiteral("width"), settings.value( QStringLiteral("width"), m_defaultSize.toSize().width() ) );
    m_settings.insert( QStringLiteral("height"), settings.value( QStringLiteral("height"), m_defaultSize.toSize().height() ) );
    m_settings.insert( QStringLiteral("posColor"), settings.value( QStringLiteral("posColor"), QColor( Qt::white ).name() ) );

    m_target.clear(); // FIXME: forces execution of changeBackground() in changeViewport()

    emit settingsChanged( nameId() );
}

bool OverviewMap::eventFilter( QObject *object, QEvent *e )
{
    if ( !enabled() || !visible() ) {
        return false;
    }

    MarbleWidget *widget = dynamic_cast<MarbleWidget*>(object);
    if ( !widget ) {
        return AbstractFloatItem::eventFilter(object,e);
    }

    if ( e->type() == QEvent::MouseButtonDblClick || e->type() == QEvent::MouseMove ) {
        QMouseEvent *event = static_cast<QMouseEvent*>(e);
        QRectF floatItemRect = QRectF( positivePosition(), size() );

        bool cursorAboveFloatItem(false);
        if ( floatItemRect.contains(event->pos()) ) {
            cursorAboveFloatItem = true;

            // Double click triggers recentering the map at the specified position
            if ( e->type() == QEvent::MouseButtonDblClick ) {
                QRectF mapRect( contentRect() );
                QPointF pos = event->pos() - floatItemRect.topLeft() 
                    - QPointF(padding(),padding());

                qreal lon = ( pos.x() - mapRect.width() / 2.0 ) / mapRect.width() * 360.0 ;
                qreal lat = ( mapRect.height() / 2.0 - pos.y() ) / mapRect.height() * 180.0;
                widget->centerOn(lon,lat,true);

                return true;
            }
        }

        if ( cursorAboveFloatItem && e->type() == QEvent::MouseMove 
                && !(event->buttons() & Qt::LeftButton) )
        {
            // Cross hair cursor when moving above the float item without pressing a button
            widget->setCursor(QCursor(Qt::CrossCursor));
            return true;
        }
    }

    return AbstractFloatItem::eventFilter(object,e);
}

void OverviewMap::changeBackground()
{
    m_mapChanged = true;
}


}

Q_EXPORT_PLUGIN2( OverviewMap, Marble::OverviewMap )
