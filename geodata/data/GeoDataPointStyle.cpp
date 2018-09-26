#include <QUrl>

#include "geodata/data/GeoDataPointStyle.h"
#include "geodata/data/GeoDataIconStyle.h"
#include "MarbleDirs.h"
#include "MarbleDebug.h"
#include "RemoteIconLoader.h"

#include "geodata/parser/GeoDataTypes.h"

namespace Marble
{

class GeoDataPointStylePrivate
{
  public:
    GeoDataPointStylePrivate()
        : m_scale( 1.0 ),
        m_iconPath()
    {
    }

    GeoDataPointStylePrivate( QString  iconPath)
        : m_scale( 1.0 ),
          m_iconPath(std::move( iconPath ))
    {
    }

    const char* nodeType() const
    {
        return GeoDataTypes::GeoDataPointStyleType;
    }

    RemoteIconLoader* remoteIconLoader() const
    {
        static RemoteIconLoader *remoteIconLoader = new RemoteIconLoader();
        return remoteIconLoader;
    }

    double            m_scale;

    QPixmap           m_icon;
    QPixmap           m_scaledIcon;
    QString           m_iconPath;
};

GeoDataPointStyle::GeoDataPointStyle() :
    d( new GeoDataPointStylePrivate() )
{
}

GeoDataPointStyle::GeoDataPointStyle( const GeoDataPointStyle& other ) :
    GeoDataColorStyle( other ), d( new GeoDataPointStylePrivate( *other.d ) )
{
}

GeoDataPointStyle::GeoDataPointStyle( const QString& iconPath) :
    d( new GeoDataPointStylePrivate( iconPath) )
{
}

GeoDataPointStyle::~GeoDataPointStyle()
{
    delete d;
}

GeoDataPointStyle& GeoDataPointStyle::operator=( const GeoDataPointStyle& other )
{
    GeoDataColorStyle::operator=( other );
    *d = *other.d;
    return *this;
}

bool GeoDataPointStyle::operator==( const GeoDataPointStyle &other ) const
{
    if ( GeoDataColorStyle::operator!=( other ) ) {
        return false;
    }

    return d->m_scale == other.d->m_scale &&
           d->m_icon.toImage() == other.d->m_icon.toImage() &&
           d->m_iconPath == other.d->m_iconPath;
}

bool GeoDataPointStyle::operator!=( const GeoDataPointStyle &other ) const
{
    return !this->operator==( other );
}

const char* GeoDataPointStyle::nodeType() const
{
    return d->nodeType();
}

void GeoDataPointStyle::setIcon(const QPixmap &icon)
{
    d->m_icon = icon;
    d->m_scaledIcon = QPixmap();
}

void GeoDataPointStyle::setIconPath( const QString& filename )
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

QString GeoDataPointStyle::iconPath() const
{
    return d->m_iconPath;
}

QPixmap GeoDataPointStyle::icon() const
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

void GeoDataPointStyle::setScale( const double &scale )
{
    d->m_scale = scale;
}

double
GeoDataPointStyle::scale() const
{
    return d->m_scale;
}

QPixmap
GeoDataPointStyle::scaledIcon() const
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

    if ( iconSize.width()*scale() > s_maximumPointSize.width()
         || iconSize.height()*scale() > s_maximumPointSize.height() ) {
       iconSize.scale( s_maximumPointSize, Qt::KeepAspectRatio );
    }
    else if ( iconSize.width()*scale() < s_minimumPointSize.width()
              || iconSize.height()*scale() < s_minimumPointSize.height() ) {
       iconSize.scale( s_minimumPointSize, Qt::KeepAspectRatio );
    }
    else {
       iconSize *= scale();
    }
    if (!icon().isNull()) {
        d->m_scaledIcon = icon().scaled( iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation ) ;
    }
    return d->m_scaledIcon;
}


RemoteIconLoader *GeoDataPointStyle::remoteIconLoader() const
{
    return d->remoteIconLoader();
}

}
