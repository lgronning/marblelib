//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008      Patrick Spendrin <ps_ml@gmx.de>
//


#include "geodata/data/GeoDataLineStyle.h"

#include "geodata/parser/GeoDataTypes.h"

#include <QDataStream>
#include <QtGui/QPixmap>

namespace Marble
{

static const QSize s_maximumIconSize = QSize( 100, 100 );
static const QSize s_minimumIconSize = QSize( 5, 5 );


class GeoDataLineStylePrivate
{

  public:
    GeoDataLineStylePrivate() 
        : m_width( 0.0 ),
          m_physicalWidth( 0.0 ),
          m_cosmeticOutline( false ),
          m_capStyle( Qt::FlatCap ),
          m_penStyle( Qt::SolidLine ),
          m_background( false ),
          m_alwaysUsePointPixmap(false)
    {
    }

    const char* nodeType() const
    {
        return GeoDataTypes::GeoDataLineStyleType;
    }

    /// The current width of the line
    double  m_width;
    /// The current real width of the line
    double  m_physicalWidth;
    bool  m_cosmeticOutline;
    Qt::PenCapStyle m_capStyle;
    Qt::PenStyle m_penStyle;
    bool m_background;
    QVector< qreal > m_pattern;
    QPixmap m_pointPixmap;
    QPixmap m_scaledPointPixmap;
    bool m_alwaysUsePointPixmap;
};

GeoDataLineStyle::GeoDataLineStyle()
    : d (new GeoDataLineStylePrivate )
{
}

GeoDataLineStyle::GeoDataLineStyle( const GeoDataLineStyle& other )
    : GeoDataColorStyle( other ), d (new GeoDataLineStylePrivate( *other.d ) )
{
}

GeoDataLineStyle::GeoDataLineStyle( const QColor &color )
    : d ( new GeoDataLineStylePrivate )
{
    setColor( color );
}

GeoDataLineStyle::~GeoDataLineStyle()
{
    delete d;
}

GeoDataLineStyle& GeoDataLineStyle::operator=( const GeoDataLineStyle& other )
{
    GeoDataColorStyle::operator=( other );
    *d = *other.d;
    return *this;
}

bool GeoDataLineStyle::operator==( const GeoDataLineStyle &other ) const
{
    if ( GeoDataColorStyle::operator!=( other ) ) {
        return false;
    }

    return d->m_width == other.d->m_width &&
           d->m_physicalWidth == other.d->m_physicalWidth &&
           d->m_capStyle == other.d->m_capStyle &&
           d->m_penStyle == other.d->m_penStyle &&
           d->m_background == other.d->m_background &&
           d->m_pattern == other.d->m_pattern;
}

bool GeoDataLineStyle::operator!=( const GeoDataLineStyle &other ) const
{
    return !this->operator==( other );
}

const char* GeoDataLineStyle::nodeType() const
{
    return d->nodeType();
}

void GeoDataLineStyle::setWidth( const double &width )
{
    d->m_width = width;
}

double GeoDataLineStyle::width() const
{
    return d->m_width;
}

double GeoDataLineStyle::physicalWidth() const
{
    return d->m_physicalWidth;
}

void GeoDataLineStyle::setPhysicalWidth( const double& realWidth )
{
    d->m_physicalWidth = realWidth;
}

bool GeoDataLineStyle::cosmeticOutline() const
{
    return d->m_cosmeticOutline;
}

void GeoDataLineStyle::setCosmeticOutline(bool enabled)
{
    d->m_cosmeticOutline = enabled;
}

Qt::PenCapStyle GeoDataLineStyle::capStyle() const
{
    return d->m_capStyle;
}

void GeoDataLineStyle::setCapStyle( Qt::PenCapStyle style )
{
    d->m_capStyle = style;
}

Qt::PenStyle GeoDataLineStyle::penStyle() const
{
    return d->m_penStyle;
}

void GeoDataLineStyle::setPenStyle( Qt::PenStyle style )
{
   d->m_penStyle = style;
}

bool GeoDataLineStyle::background() const
{
    return d->m_background;
}

void GeoDataLineStyle::setBackground( bool background )
{
    d->m_background = background;
}

QVector< qreal > GeoDataLineStyle::dashPattern() const
{
    return d->m_pattern;
}

void GeoDataLineStyle::setPointPixmap(const QPixmap &pixmap)
{
    d->m_pointPixmap = pixmap;
}

const QPixmap &
GeoDataLineStyle::pointPixmap() const
{
    return d->m_pointPixmap;
}

const QPixmap &GeoDataLineStyle::scaledPointPixmap() const
{
    if (!d->m_scaledPointPixmap.isNull()) {
        return d->m_scaledPointPixmap;
    }

    // Scale shouldn't be 0, but if it is, returning regular icon.
    if( pointPixmap().isNull() ) {
        return pointPixmap();
    }

    QSize pixmapSize = pointPixmap().size();

    // Scaling the placemark's icon based on its size, scale, and maximum icon size.

    if ( pixmapSize.width()> s_maximumIconSize.width()
         || pixmapSize.height() > s_maximumIconSize.height() ) {
       pixmapSize.scale( s_maximumIconSize, Qt::KeepAspectRatio );
    }
    else if ( pixmapSize.width() < s_minimumIconSize.width()
              || pixmapSize.height() < s_minimumIconSize.height() )
    {
       pixmapSize.scale( s_minimumIconSize, Qt::KeepAspectRatio );
    }


    d->m_scaledPointPixmap = pointPixmap().scaled( pixmapSize, Qt::KeepAspectRatio, Qt::SmoothTransformation ) ;

    return d->m_scaledPointPixmap;
}

bool
GeoDataLineStyle::alwaysUsePointPixmap() const
{
    return d->m_alwaysUsePointPixmap;
}

void
GeoDataLineStyle::setAlwaysUsePointPixmap(bool value) const
{
    d->m_alwaysUsePointPixmap = value;
}

void GeoDataLineStyle::setDashPattern( const QVector< qreal >& pattern )
{
    d->m_pattern = pattern;
}

}
