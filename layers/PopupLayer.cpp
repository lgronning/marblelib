//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2012   Mohammed Nafees   <nafees.technocool@gmail.com>
// Copyright 2012   Dennis Nienhüser  <nienhueser@kde.org>
// Copyright 2012   Illya Kovalevskyy <illya.kovalevskyy@gmail.com>
// Copyright 2015   Imran Tatriev     <itatriev@gmail.com>
//

#include "PopupLayer.h"

#include "geodata/data/GeoDataCoordinates.h"
#include "geodata/data/GeoDataPlacemark.h"
#include "GeoPainter.h"
#include "MarbleWidget.h"
#include "PopupItem.h"
#include "ViewportParams.h"
#include "RenderPlugin.h"

#include <QSizeF>
#include <QDebug>

namespace Marble
{

class Q_DECL_HIDDEN PopupLayer::Private
{
public:
    Private( MarbleWidget *marbleWidget, PopupLayer *q );

    /**
     * @brief Sets size of the popup item, based on the requested size and viewport size
     * @param viewport required to compute the maximum dimensions
     */
    void setAppropriateSize( const ViewportParams *viewport );

    static QString filterEmptyShortDescription(const QString &description);
    void setupDialogSatellite( const GeoDataPlacemark *index );
    void setupDialogCity( const GeoDataPlacemark *index );
    void setupDialogNation( const GeoDataPlacemark *index );
    void setupDialogGeoPlaces( const GeoDataPlacemark *index );
    void setupDialogSkyPlaces( const GeoDataPlacemark *index );

    PopupItem *const m_popupItem;
    MarbleWidget *const m_widget;
    QSizeF m_requestedSize;
    bool m_hasCrosshairsPlugin;
    bool m_crosshairsVisible;
};

PopupLayer::Private::Private( MarbleWidget *marbleWidget, PopupLayer *q ) :
    m_popupItem( new PopupItem( q ) ),
    m_widget( marbleWidget ),
    m_hasCrosshairsPlugin( false ),
    m_crosshairsVisible( true )
{
}

PopupLayer::PopupLayer( MarbleWidget *marbleWidget, QObject *parent ) :
    QObject( parent ),
    d( new Private( marbleWidget, this ) )
{
    foreach (const RenderPlugin *renderPlugin, d->m_widget->renderPlugins()) {
        if( renderPlugin->nameId() == QLatin1String("crosshairs") ) {
            d->m_hasCrosshairsPlugin = true;
            break;
        }
    }

    connect( d->m_popupItem, SIGNAL(repaintNeeded()), this, SIGNAL(repaintNeeded()) );
}

PopupLayer::~PopupLayer()
{
    delete d;
}

QStringList PopupLayer::renderPosition() const
{
    return QStringList( QStringLiteral("ALWAYS_ON_TOP") );
}

bool PopupLayer::render( GeoPainter *painter, ViewportParams *viewport,
                                const QString&, GeoSceneLayer* )
{
    if ( visible() )
    {
        d->setAppropriateSize( viewport );
        d->m_popupItem->paintEvent( painter, viewport );
    }

    return true;
}

qreal PopupLayer::zValue() const
{
    return 4711.23;
}

bool PopupLayer::visible() const
{
    return d->m_popupItem->visible();
}

void PopupLayer::setVisible( bool visible )
{
    d->m_popupItem->setVisible( visible );
    if ( !visible ) {
        disconnect( d->m_popupItem, SIGNAL(repaintNeeded()), this, SIGNAL(repaintNeeded()) );
        emit repaintNeeded();
    }
    else
    {
        connect( d->m_popupItem, SIGNAL(repaintNeeded()), this, SIGNAL(repaintNeeded()) );
    }
}

void PopupLayer::popup()
{
    qDebug() << "PopupLayer::popup";

    if( d->m_hasCrosshairsPlugin ) {
        d->m_crosshairsVisible = d->m_widget->showCrosshairs();

        if( d->m_crosshairsVisible ) {
            d->m_widget->setShowCrosshairs( false );
        }
    }

    setVisible( true );
}

void PopupLayer::setPosition( const QPoint &pos)
{
    d->m_popupItem->setPosition( pos );
}



void PopupLayer::setPlacemark(const GeoDataPlacemark *placemark)
{
    d->m_popupItem->setContent( placemark->description());
}

void PopupLayer::setContent(const QString &html, const QPixmap &pixmap)
{
    qDebug() << "PopupLayer::setContent" << html;

    d->m_popupItem->setContent( html, pixmap );

}

void PopupLayer::Private::setAppropriateSize( const ViewportParams *viewport )
{
    qreal margin = 15.0;

    QSizeF maximumSize;
    maximumSize.setWidth( viewport->width() - margin );
    maximumSize.setHeight( viewport->height() - margin );

    QSizeF minimumSize( 20.0, 20.0 );

    m_popupItem->setSize( QSizeF(m_popupItem->sizeHint()).boundedTo( maximumSize ).expandedTo( minimumSize ) );
}

}

//#include "moc_PopupLayer.cpp"
