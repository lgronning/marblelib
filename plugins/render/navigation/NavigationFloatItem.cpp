//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008      Dennis Nienhüser <nienhueser@kde.org>
// Copyright 2010      Bastian Holst <bastianholst@gmx.de>
// Copyright 2013      Mohammed Nafees <nafees.technocool@gmail.com>
//

#include "NavigationFloatItem.h"

#include <qmath.h>
#include <QContextMenuEvent>
#include <QRect>
#include <QPixmap>
#include <QToolButton>
#include <QSlider>
#include <QWidget>
#include <QPainter>
#include <QPixmapCache>

#include "ui_navigation.h"
#include "ViewportParams.h"
#include "MarbleDebug.h"
#include "MarbleWidget.h"
#include "MarbleModel.h"
#include "graphicsview/WidgetGraphicsItem.h"
#include "graphicsview/MarbleGraphicsGridLayout.h"

using namespace Marble;
/* TRANSLATOR Marble::NavigationFloatItem */

NavigationFloatItem::NavigationFloatItem( const MarbleModel *marbleModel )
    : AbstractFloatItem( marbleModel, QPointF( -10, -30 ) ),
      m_marbleWidget( nullptr ),
      m_widgetItem( nullptr ),
      m_navigationWidget( nullptr ),
      m_oldViewportRadius( 0 ),
      m_contextMenu( nullptr )
{
    // Plugin is visible by default on desktop systems
    const bool smallScreen = MarbleGlobal::getInstance()->profiles() & MarbleGlobal::SmallScreen;
    setEnabled( !smallScreen );
    setVisible( true );

    setCacheMode( NoCache );
    setBackground( QBrush( QColor( Qt::transparent ) ) );
    setFrame( NoFrame );
}

NavigationFloatItem::~NavigationFloatItem()
{
    QPixmapCache::remove( QStringLiteral("marble/navigation/navigational_backdrop_top") );
    QPixmapCache::remove( QStringLiteral("marble/navigation/navigational_backdrop_center") );
    QPixmapCache::remove( QStringLiteral("marble/navigation/navigational_backdrop_bottom") );
    QPixmapCache::remove( QStringLiteral("marble/navigation/navigational_currentlocation") );
    QPixmapCache::remove( QStringLiteral("marble/navigation/navigational_currentlocation_hover") );
    QPixmapCache::remove( QStringLiteral("marble/navigation/navigational_currentlocation_pressed") );

    delete m_navigationWidget;
}

QStringList NavigationFloatItem::backendTypes() const
{
    return QStringList(QStringLiteral("navigation"));
}

QString NavigationFloatItem::name() const
{
    return tr("Navigation");
}

QString NavigationFloatItem::guiString() const
{
    return tr("&Navigation");
}

QString NavigationFloatItem::nameId() const
{
    return QStringLiteral("navigation");
}

QString NavigationFloatItem::version() const
{
    return QStringLiteral("1.0");
}

QString NavigationFloatItem::description() const
{
    return tr("A mouse control to zoom and move the map");
}

QString NavigationFloatItem::copyrightYears() const
{
    return QStringLiteral("2008, 2010, 2013");
}

QList<PluginAuthor> NavigationFloatItem::pluginAuthors() const
{
    return QList<PluginAuthor>()
            << PluginAuthor( QString::fromUtf8( "Dennis Nienhüser" ), QStringLiteral("nienhueser@kde.org") )
            << PluginAuthor( QStringLiteral("Bastian Holst"), QStringLiteral("bastianholst@gmx.de") )
            << PluginAuthor( QStringLiteral("Mohammed Nafees"), QStringLiteral("nafees.technocool@gmail.com") );
}

QIcon NavigationFloatItem::icon() const
{
    return QIcon(":/icons/navigation.png");
}

void NavigationFloatItem::initialize()
{
    QWidget *navigationParent = new QWidget( nullptr );
    navigationParent->setAttribute( Qt::WA_NoSystemBackground, true );

    m_navigationWidget = new Ui::Navigation;
    m_navigationWidget->setupUi( navigationParent );

    m_widgetItem = new WidgetGraphicsItem( this );
    m_widgetItem->setWidget( navigationParent );

    MarbleGraphicsGridLayout *layout = new MarbleGraphicsGridLayout( 1, 1 );
    layout->addItem( m_widgetItem, 0, 0 );

    setLayout( layout );
}

bool NavigationFloatItem::isInitialized() const
{
    return m_widgetItem;
}

void NavigationFloatItem::setProjection( const ViewportParams *viewport )
{
    if ( viewport->radius() != m_oldViewportRadius ) {
        m_oldViewportRadius = viewport->radius();
        // The slider depends on the map state (zoom factor)
        update();
    }

    AbstractFloatItem::setProjection( viewport );
}

bool NavigationFloatItem::eventFilter( QObject *object, QEvent *e )
{
    if ( !enabled() || !visible() ) {
        return false;
    }

    MarbleWidget *widget = dynamic_cast<MarbleWidget*> (object);
    if ( !widget ) {
        return AbstractFloatItem::eventFilter( object, e );
    }

    if ( m_marbleWidget != widget ) {
        // Delayed initialization
        m_marbleWidget = widget;

        m_maxZoom = m_marbleWidget->maximumZoom();
        m_minZoom = m_marbleWidget->minimumZoom();

        m_navigationWidget->arrowDisc->setMarbleWidget( m_marbleWidget );
        connect( m_navigationWidget->arrowDisc, SIGNAL(repaintNeeded()), SIGNAL(repaintNeeded()) );

        connect( m_navigationWidget->zoomInButton, SIGNAL(repaintNeeded()), SIGNAL(repaintNeeded()) );
        connect( m_navigationWidget->zoomInButton, SIGNAL(clicked()),
                 m_marbleWidget, SLOT(zoomIn()) );

        m_navigationWidget->zoomSlider->setMaximum( m_maxZoom );
        m_navigationWidget->zoomSlider->setMinimum( m_minZoom );
        connect( m_navigationWidget->zoomSlider, SIGNAL(repaintNeeded()), SIGNAL(repaintNeeded()) );
        connect( m_navigationWidget->zoomSlider, SIGNAL(valueChanged(int)),
                 m_marbleWidget, SLOT(setZoom(int)) );

        connect( m_navigationWidget->zoomOutButton, SIGNAL(repaintNeeded()), SIGNAL(repaintNeeded()) );
        connect( m_navigationWidget->zoomOutButton, SIGNAL(clicked()),
                 m_marbleWidget, SLOT(zoomOut()) );

        connect( m_marbleWidget, SIGNAL(zoomChanged(int)), SLOT(updateButtons(int)) );
        updateButtons( m_marbleWidget->zoom() );
        connect( m_marbleWidget, SIGNAL(themeChanged(QString)), this, SLOT(selectTheme(QString)) );
     }

    return AbstractFloatItem::eventFilter(object, e);
}

void NavigationFloatItem::selectTheme( QString )
{
    if ( m_marbleWidget ) {
        m_maxZoom = m_marbleWidget->maximumZoom();
        m_minZoom = m_marbleWidget->minimumZoom();
        m_navigationWidget->zoomSlider->setMaximum( m_maxZoom );
        m_navigationWidget->zoomSlider->setMinimum( m_minZoom );
        updateButtons( m_marbleWidget->zoom() );
    }
}

void NavigationFloatItem::updateButtons( int zoomValue )
{
    bool const zoomInEnabled = m_navigationWidget->zoomInButton->isEnabled();
    bool const zoomOutEnabled = m_navigationWidget->zoomOutButton->isEnabled();
    int const oldZoomValue = m_navigationWidget->zoomSlider->value();
    m_navigationWidget->zoomInButton->setEnabled( zoomValue < m_maxZoom );
    m_navigationWidget->zoomOutButton->setEnabled( zoomValue > m_minZoom );
    m_navigationWidget->zoomSlider->setValue( zoomValue );
    if ( zoomInEnabled != m_navigationWidget->zoomInButton->isEnabled() ||
         zoomOutEnabled != m_navigationWidget->zoomOutButton->isEnabled() ||
         oldZoomValue != zoomValue ) {
        update();
    }
}

QPixmap NavigationFloatItem::pixmap( const QString &id )
{
    QPixmap result;
    if ( !QPixmapCache::find( id, result ) ) {
        result = QPixmap( QStringLiteral( ":/%1.png" ).arg( id ) );
        QPixmapCache::insert( id, result );
    }
    return result;
}

void NavigationFloatItem::paintContent( QPainter *painter )
{
    painter->drawPixmap( 0, 0, pixmap( QStringLiteral("marble/navigation/navigational_backdrop_top") ) );
    painter->drawPixmap( 0, 70, pixmap( QStringLiteral("marble/navigation/navigational_backdrop_center") ) );
    painter->drawPixmap( 0, 311, pixmap( QStringLiteral("marble/navigation/navigational_backdrop_bottom") ) );
}

void NavigationFloatItem::contextMenuEvent( QWidget *w, QContextMenuEvent *e )
{
    if ( !m_contextMenu ) {
        m_contextMenu = contextMenu();
    }

    Q_ASSERT( m_contextMenu );
    m_contextMenu->move(e->globalPos());
    m_contextMenu->show( );
}


QHash<QString,QVariant> NavigationFloatItem::settings() const
{
    return AbstractFloatItem::settings();
}

void NavigationFloatItem::setSettings( const QHash<QString, QVariant> &settings )
{
    AbstractFloatItem::setSettings( settings );
}

Q_EXPORT_PLUGIN2( NavigationFloatItem, Marble::NavigationFloatItem )
