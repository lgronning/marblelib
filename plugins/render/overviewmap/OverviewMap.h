//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008 Torsten Rahn <tackat@kde.org>
//

#ifndef MARBLEOVERVIEWMAP_H
#define MARBLEOVERVIEWMAP_H

#include <QObject>
#include <QHash>
#include <QColor>
#include <QAbstractButton>
#include <QtSvg/QSvgWidget>
#include <QtSvg/QSvgRenderer>

#include "geodata/data/GeoDataLatLonAltBox.h"
#include "AbstractFloatItem.h"
#include "DialogConfigurationInterface.h"

namespace Marble
{

/**
 * @short The class that creates an overview map.
 *
 */

class OverviewMap : public AbstractFloatItem
{
    Q_OBJECT
    Q_PLUGIN_METADATA( IID "org.kde.edu.marble.OverviewMap" )
    Q_INTERFACES( Marble::RenderPluginInterface )
    MARBLE_PLUGIN( OverviewMap )
    
 public:
    OverviewMap();
    explicit OverviewMap( const MarbleModel *marbleModel );
    ~OverviewMap() override;

    QStringList backendTypes() const override;

    QString name() const override;

    QString guiString() const override;

    QString nameId() const override;

    QString version() const override;

    QString description() const override;

    QString copyrightYears() const override;

    QList<PluginAuthor> pluginAuthors() const override;

    QIcon icon () const override;

    void initialize () override;

    bool isInitialized () const override;

    void setProjection( const ViewportParams *viewport ) override;

    void paintContent( QPainter *painter ) override;

    /**
     * @return: The settings of the item.
     */
    QHash<QString,QVariant> settings() const override;

    /**
     * Set the settings of the item.
     */
    void setSettings( const QHash<QString,QVariant> &settings ) override;

 protected:
    bool eventFilter( QObject *object, QEvent *e ) override;

 private:
    void changeBackground();

    QSvgWidget *
    currentWidget() const;

    void
    setCurrentWidget( QSvgWidget *widget );

    void
    loadPlanetMaps();

    void
    loadMapSuggestions();

    QString m_target;
    QPixmap        m_worldmap;
    QHash<QString,QVariant> m_settings;
    QColor m_posColor;
    QSizeF m_defaultSize;

    GeoDataLatLonAltBox m_latLonAltBox;
    qreal m_centerLat;
    qreal m_centerLon;
    bool m_mapChanged;
};

}

#endif
