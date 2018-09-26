//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008 Torsten Rahn <tackat@kde.org>
//

#ifndef COMPASS_FLOAT_ITEM_H
#define COMPASS_FLOAT_ITEM_H

#include <QObject>

#include "AbstractFloatItem.h"
#include "DialogConfigurationInterface.h"

class QSvgRenderer;

namespace Ui {
    class CompassConfigWidget;
}

namespace Marble
{

/**
 * @short The class that creates a compass
 *
 */

class CompassFloatItem  : public AbstractFloatItem
{
    Q_OBJECT
    Q_PLUGIN_METADATA( IID "org.kde.edu.marble.CompassFloatItem" )
    Q_INTERFACES( Marble::RenderPluginInterface )
    MARBLE_PLUGIN( CompassFloatItem )
 public:
    CompassFloatItem();
    explicit CompassFloatItem( const MarbleModel *marbleModel );
    ~CompassFloatItem () override;

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

    QPainterPath backgroundShape() const override;

    void setProjection( const ViewportParams *viewport ) override;

    void paintContent( QPainter *painter ) override;

    QHash<QString,QVariant> settings() const override;

    void setSettings( const QHash<QString,QVariant> &settings ) override;

private:
    Q_DISABLE_COPY( CompassFloatItem )

    bool           m_isInitialized;

    QSvgRenderer  *m_svgobj;
    QPixmap        m_compass;

    /// allowed values: -1, 0, 1; default here: 0. FIXME: Declare enum
    int            m_polarity;
};
}

#endif
