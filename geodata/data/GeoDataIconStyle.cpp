//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2007      Murad Tagirov <tmurad@gmail.com>
//


#include <QUrl>

#include "geodata/data/GeoDataIconStyle.h"
#include "MarbleDirs.h"
#include "MarbleDebug.h"
#include "RemoteIconLoader.h"

#include "geodata/parser/GeoDataTypes.h"

namespace Marble
{

class GeoDataIconStylePrivate
{
  public:
    GeoDataIconStylePrivate()
        : m_scale( 1.0 ),
        m_iconPath(),
        m_heading( 0 )
    {
    }

    GeoDataIconStylePrivate( QString  iconPath )
        : m_scale( 1.0 ),
          m_iconPath(std::move( iconPath )),
          m_heading( 0 )
    {
    }

    const char* nodeType() const
    {
        return GeoDataTypes::GeoDataIconStyleType;
    }

    RemoteIconLoader* remoteIconLoader() const
    {
        static RemoteIconLoader *remoteIconLoader = new RemoteIconLoader();
        return remoteIconLoader;
    }

    double           m_scale;

    QPixmap          m_icon;
    QPixmap          m_scaledIcon;
    QString          m_iconPath;
    QPointF          m_hotSpot;
    int              m_heading;
};

GeoDataIconStyle::GeoDataIconStyle() :
    d( new GeoDataIconStylePrivate() )
{
}

GeoDataIconStyle::GeoDataIconStyle( const GeoDataIconStyle& other ) :
    GeoDataColorStyle( other ), d( new GeoDataIconStylePrivate( *other.d ) )
{
}

GeoDataIconStyle::GeoDataIconStyle(const QString& iconPath) :
    d( new GeoDataIconStylePrivate( iconPath ) )
{
}

GeoDataIconStyle::~GeoDataIconStyle()
{
    delete d;
}

GeoDataIconStyle& GeoDataIconStyle::operator=( const GeoDataIconStyle& other )
{
    GeoDataColorStyle::operator=( other );
    *d = *other.d;
    return *this;
}

bool GeoDataIconStyle::operator==( const GeoDataIconStyle &other ) const
{
    if ( GeoDataColorStyle::operator!=( other ) ) {
        return false;
    }

    return qFuzzyCompare(d->m_scale, other.d->m_scale) &&
           d->m_icon.cacheKey() == other.d->m_icon.cacheKey() &&
           d->m_iconPath == other.d->m_iconPath &&
           d->m_hotSpot == other.d->m_hotSpot &&
           d->m_heading == other.d->m_heading;
}

bool GeoDataIconStyle::operator!=( const GeoDataIconStyle &other ) const
{
    return !this->operator==( other );
}

const char* GeoDataIconStyle::nodeType() const
{
    return d->nodeType();
}

void GeoDataIconStyle::setIcon(const QPixmap &icon)
{
    d->m_icon = icon;
    d->m_scaledIcon = QPixmap();
}

void GeoDataIconStyle::setIconPath( const QString& filename )
{
    d->m_iconPath = filename;

    /**
     * Set the m_icon to be a default-constructed icon
     * so that m_icon is null and icon() doesn't return
     * prevously loaded icon.
     */
    d->m_icon = QPixmap();
    d->m_scaledIcon = QPixmap();
}

QString GeoDataIconStyle::iconPath() const
{
    return d->m_iconPath;
}

QPixmap GeoDataIconStyle::icon() const
{
    if ( !d->m_icon.isNull() ) {
        return d->m_icon;
    }
    else if ( !d->m_iconPath.isEmpty() ) {
        d->m_icon = QPixmap( resolvePath( d->m_iconPath ) );
        if( d->m_icon.isNull() ) {
            // if image is not found on disk, check whether the icon is
            // at remote location. If yes then go for remote icon loading
            QUrl remoteLocation = QUrl( d->m_iconPath );
            if( remoteLocation.isValid() ) {
                d->m_icon = QPixmap::fromImage(d->remoteIconLoader()->load( d->m_iconPath ));
            }
            else {
                mDebug() << "Unable to open style icon at: " << d->m_iconPath;
            }
        }

        return d->m_icon;
    }
    else
        return QPixmap();
}

bool GeoDataIconStyle::hasIcon() const
{
    return (!d->m_icon.isNull() || !d->m_iconPath.isEmpty());
}

void GeoDataIconStyle::setHotSpot( const QPointF &position )
{
    d->m_hotSpot = position;
}

const QPointF &
GeoDataIconStyle::hotSpot() const
{
    return d->m_hotSpot;
}

void GeoDataIconStyle::setScale( const double &scale )
{
    d->m_scale = scale;
}

double GeoDataIconStyle::scale() const
{
    return d->m_scale;
}

QPixmap GeoDataIconStyle::scaledIcon() const
{
    if (!d->m_scaledIcon.isNull()) {
        return d->m_scaledIcon;
    }

    // Scale shouldn't be 0, but if it is, returning regular icon.
    if( scale() <= 0 || icon().isNull() ) {
        return icon();
    }

    QSize iconSize = icon().size();

    // Scaling the placemark's icon based on its size, scale, and maximum icon size.

    if ( iconSize.width()*scale() > s_maximumIconSize.width()
         || iconSize.height()*scale() > s_maximumIconSize.height() ) {
       iconSize.scale( s_maximumIconSize, Qt::KeepAspectRatio );
    }
    else if ( iconSize.width()*scale() < s_minimumIconSize.width()
              || iconSize.height()*scale() < s_minimumIconSize.height() ) {
       iconSize.scale( s_minimumIconSize, Qt::KeepAspectRatio );
    }
    else {
       iconSize *= scale();
    }
    if (!icon().isNull()) {
        d->m_scaledIcon = icon().scaled( iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation ) ;
    }
    return d->m_scaledIcon;
}

int GeoDataIconStyle::heading() const
{
    return d->m_heading;
}

void GeoDataIconStyle::setHeading( int heading )
{
    d->m_heading = heading;
}

RemoteIconLoader *GeoDataIconStyle::remoteIconLoader() const
{
    return d->remoteIconLoader();
}

}
