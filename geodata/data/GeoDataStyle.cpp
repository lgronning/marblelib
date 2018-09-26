//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2007      Murad Tagirov <tmurad@gmail.com>
//


#include "geodata/data/GeoDataStyle.h"

#include "geodata/parser/GeoDataTypes.h"
#include "geodata/data/GeoDataStyleSelector.h"
#include "geodata/data/GeoDataIconStyle.h"
#include "geodata/data/GeoDataLabelStyle.h"
#include "geodata/data/GeoDataPolyStyle.h"
#include "geodata/data/GeoDataLineStyle.h"
#include "geodata/data/GeoDataBalloonStyle.h"
#include "geodata/data/GeoDataListStyle.h"
#include "geodata/data/GeoDataPointStyle.h"
#include "geodata/data/GeoDataSpectroStyle.h"

namespace Marble
{

class GeoDataStylePrivate
{
  public:
    GeoDataStylePrivate()
        :   useAntialising(true)
    {
    }

    GeoDataStylePrivate(const QString& iconPath,
                        const QFont &font, const QColor &color )
        : m_iconStyle( iconPath ),
          m_labelStyle( font, color ),
          m_lineStyle( color ),
          m_polyStyle( color ),
          m_balloonStyle(),
          m_pointStyle(),
          useAntialising(true)

    {
    }

    const char* nodeType() const
    {
        return GeoDataTypes::GeoDataStyleType;
    }

    GeoDataIconStyle   m_iconStyle;
    GeoDataLabelStyle  m_labelStyle;
    GeoDataLineStyle   m_lineStyle;
    GeoDataPolyStyle   m_polyStyle;
    GeoDataBalloonStyle m_balloonStyle;
    GeoDataListStyle m_listStyle;
    GeoDataPointStyle m_pointStyle;
    GeoDataSpectroStyle m_spectroStyle;
    bool useAntialising;
};

GeoDataStyle::GeoDataStyle()
    : d( new GeoDataStylePrivate )
{
}

GeoDataStyle::GeoDataStyle( const GeoDataStyle& other )
    : GeoDataStyleSelector( other ), d( new GeoDataStylePrivate( *other.d ) )
{
}

GeoDataStyle::GeoDataStyle( const QString& iconPath,
                            const QFont &font, const QColor &color  )
    : d( new GeoDataStylePrivate( iconPath, font, color ) )
{
}

GeoDataStyle::~GeoDataStyle()
{
    delete d;
}

GeoDataStyle& GeoDataStyle::operator=( const GeoDataStyle& other )
{
    GeoDataStyleSelector::operator=( other );
    *d = *other.d;
    return *this;
}

bool GeoDataStyle::operator==( const GeoDataStyle &other ) const
{
    if ( GeoDataStyleSelector::operator!=( other ) ) {
        return false;
    }

    return d->m_iconStyle == other.d->m_iconStyle &&
           d->m_labelStyle == other.d->m_labelStyle &&
           d->m_lineStyle == other.d->m_lineStyle &&
           d->m_polyStyle == other.d->m_polyStyle &&
           d->m_balloonStyle == other.d->m_balloonStyle &&
           d->m_listStyle == other.d->m_listStyle &&
           d->m_pointStyle == other.d->m_pointStyle
            ;
}

bool GeoDataStyle::operator!=( const GeoDataStyle &other ) const
{
    return !this->operator==( other );
}

const char* GeoDataStyle::nodeType() const
{
    return d->nodeType();
}

void GeoDataStyle::setIconStyle( const GeoDataIconStyle& style )
{
    d->m_iconStyle = style;
    d->m_iconStyle.setParent( this );
}

void GeoDataStyle::setLineStyle( const GeoDataLineStyle& style )
{
    d->m_lineStyle = style;
}

void GeoDataStyle::setLabelStyle( const GeoDataLabelStyle& style )
{
    d->m_labelStyle = style;
}

void GeoDataStyle::setPolyStyle( const GeoDataPolyStyle& style )
{
    d->m_polyStyle = style;
}

void GeoDataStyle::setBalloonStyle( const GeoDataBalloonStyle& style )
{
    d->m_balloonStyle = style;
}

void GeoDataStyle::setListStyle( const GeoDataListStyle& style )
{
    d->m_listStyle = style;
    d->m_listStyle.setParent( this );
}

GeoDataIconStyle& GeoDataStyle::iconStyle()
{
    return d->m_iconStyle;
}

const GeoDataIconStyle& GeoDataStyle::iconStyle() const
{
    return d->m_iconStyle;
}

GeoDataLineStyle& GeoDataStyle::lineStyle()
{
    return d->m_lineStyle;
}

const GeoDataLineStyle& GeoDataStyle::lineStyle() const
{
    return d->m_lineStyle;
}

GeoDataPolyStyle& GeoDataStyle::polyStyle()
{
    return d->m_polyStyle;
}

const GeoDataPolyStyle& GeoDataStyle::polyStyle() const
{
    return d->m_polyStyle;
}

GeoDataLabelStyle& GeoDataStyle::labelStyle()
{
    return d->m_labelStyle;
}

const GeoDataLabelStyle& GeoDataStyle::labelStyle() const
{
    return d->m_labelStyle;
}

GeoDataBalloonStyle& GeoDataStyle::balloonStyle()
{
    return d->m_balloonStyle;
}

const GeoDataBalloonStyle& GeoDataStyle::balloonStyle() const
{
    return d->m_balloonStyle;
}

GeoDataListStyle& GeoDataStyle::listStyle()
{
    return d->m_listStyle;
}

const GeoDataListStyle& GeoDataStyle::listStyle() const
{
    return d->m_listStyle;
}

void
GeoDataStyle::setPointStyle(const GeoDataPointStyle &style)
{
    d->m_pointStyle = style;
    d->m_pointStyle.setParent( this );
}

GeoDataPointStyle &GeoDataStyle::pointStyle()
{
    return d->m_pointStyle;
}

const GeoDataPointStyle &GeoDataStyle::pointStyle() const
{
    return d->m_pointStyle;
}


void
GeoDataStyle::setSpectroStyle(const GeoDataSpectroStyle &style)
{
    d->m_spectroStyle = style;
    d->m_spectroStyle.setParent( this );
}

GeoDataSpectroStyle &
GeoDataStyle::spectroStyle()
{
    return d->m_spectroStyle;
}

const GeoDataSpectroStyle &
GeoDataStyle::spectroStyle() const
{
    return d->m_spectroStyle;
}


bool
GeoDataStyle::useAntialising() const
{
    return d->useAntialising;
}

void
GeoDataStyle::setUseAntialising(bool value)
{
    d->useAntialising = value;
}


}
