//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2007      Murad Tagirov <tmurad@gmail.com>
// Copyright 2007      Inge Wallin   <ingwa@kde.org>
//


#ifndef MARBLE_GEODATAICONSTYLE_H
#define MARBLE_GEODATAICONSTYLE_H


#include <QPixmap>

#include "geodata/data/GeoDataColorStyle.h"

#include "geodata/geodata_export.h"

namespace Marble
{

class GeoDataIconStylePrivate;
class RemoteIconLoader;

// Limits for the user in case of scaling the icon too much
static const QSize s_maximumIconSize = QSize( 100, 100 );
static const QSize s_minimumIconSize = QSize( 15, 15 );

class GEODATA_EXPORT GeoDataIconStyle : public GeoDataColorStyle
{
  public:
    GeoDataIconStyle();
    GeoDataIconStyle( const GeoDataIconStyle& other );
    explicit GeoDataIconStyle( const QString& iconPath );
    ~GeoDataIconStyle() override;

    GeoDataIconStyle& operator=( const GeoDataIconStyle& other );

    bool operator==( const GeoDataIconStyle &other ) const;
    bool operator!=( const GeoDataIconStyle &other ) const;

    /// Provides type information for downcasting a GeoData
    const char* nodeType() const override;

    void setIcon(const QPixmap &icon );
    QPixmap icon() const;
    bool hasIcon() const;

    /**
     * @brief Returns a scaled version of label icon
     * @return  the scaled icon
     */
    QPixmap scaledIcon() const;

    /**
     * @brief setIconPath Set the path to load the icon from. Any existing icon is invalidated.
     * @param filename Path to the icon to load. Can also be a virtual file system like qrc:/
     */
    void setIconPath( const QString& filename );
    QString iconPath() const;

    void
    setHotSpot( const QPointF &position );

    const QPointF &
    hotSpot( ) const;

    void setScale( const double &scale );
    double scale() const;

    int heading() const;
    void setHeading( int heading );
    
    RemoteIconLoader *remoteIconLoader() const;
  private:
    GeoDataIconStylePrivate * const d;
};

}

#endif
