//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2007      Murad Tagirov <tmurad@gmail.com>
//


#include "geodata/data/GeoDataLabelStyle.h"

#include <QFont>
#include <QDataStream>

#include "geodata/parser/GeoDataTypes.h"

namespace Marble
{
#ifdef Q_OS_DARWIN
static const int defaultSize = 10;
#else
static const int defaultSize = 8;
#endif


class GeoDataLabelStylePrivate
{
  public:
    GeoDataLabelStylePrivate() 
        : m_scale( 1.0 ),
          m_alignment( GeoDataLabelStyle::Corner ),
          m_font( QFont(QStringLiteral("Sans Serif")).family(), defaultSize, 50, false ),
          m_glow( true ),
          m_highlightText(false),
          m_showLabel( false ),
          m_dirtyScaledFont(true)
    {
    }

    explicit GeoDataLabelStylePrivate( const QFont &font )
        : m_scale( 1.0 ),
          m_alignment( GeoDataLabelStyle::Corner ),
          m_font( font ),
          m_glow( true ),
          m_highlightText(false),
          m_showLabel( false ),
          m_dirtyScaledFont(true)
    {
    }

    const char* nodeType() const
    {
        return GeoDataTypes::GeoDataLabelStyleType;
    }

    /// The current scale of the label
    double  m_scale;
    /// The current alignment of the label
    GeoDataLabelStyle::Alignment m_alignment;
    /// The current font of the label
    QFont  m_font;   // Not a KML property
    /// Whether or not the text should glow
    bool m_glow; // Not a KML property
    bool m_highlightText;
    bool m_showLabel;
    mutable QFont m_scaledFont;
    mutable bool m_dirtyScaledFont;
};

GeoDataLabelStyle::GeoDataLabelStyle()
    : d (new GeoDataLabelStylePrivate )
{
    setColor( QColor( Qt::black ) );
}

GeoDataLabelStyle::GeoDataLabelStyle( const GeoDataLabelStyle& other )
    : GeoDataColorStyle( other ), d (new GeoDataLabelStylePrivate( *other.d ) )
{
}

GeoDataLabelStyle::GeoDataLabelStyle( const QFont &font, const QColor &color )
    : d (new GeoDataLabelStylePrivate( font ) )
{
    setColor( color );
}

GeoDataLabelStyle::~GeoDataLabelStyle()
{
    delete d;
}

GeoDataLabelStyle& GeoDataLabelStyle::operator=( const GeoDataLabelStyle& other )
{
    GeoDataColorStyle::operator=( other );
    *d = *other.d;
    return *this;
}

bool GeoDataLabelStyle::operator==( const GeoDataLabelStyle &other ) const
{
    if ( GeoDataColorStyle::operator!=( other ) ) {
        return false;
    }

    return qFuzzyCompare(d->m_scale, other.d->m_scale) &&
           d->m_alignment == other.d->m_alignment &&
           d->m_font == other.d->m_font &&
           d->m_glow == other.d->m_glow &&
           d->m_highlightText == other.d->m_highlightText;
}

bool GeoDataLabelStyle::operator!=( const GeoDataLabelStyle &other ) const
{
    return !this->operator==( other );
}

const char* GeoDataLabelStyle::nodeType() const
{
    return d->nodeType();
}

void GeoDataLabelStyle::setAlignment( GeoDataLabelStyle::Alignment alignment )
{
    d->m_alignment = alignment;
}

GeoDataLabelStyle::Alignment GeoDataLabelStyle::alignment() const
{
    return d->m_alignment;
}

void GeoDataLabelStyle::setScale( const double &scale )
{
    d->m_dirtyScaledFont = true;
    d->m_scale = scale;
}

double GeoDataLabelStyle::scale() const
{
    return d->m_scale;
}

void GeoDataLabelStyle::setFont( const QFont &font )
{
    d->m_dirtyScaledFont = true;
    d->m_font = font;
}

QFont GeoDataLabelStyle::font() const
{
    return d->m_font;
}

QFont GeoDataLabelStyle::scaledFont() const
{
   // Font shouldn't be smaller (or equal to) than 0, but if it is, regular font is returned
   // setPointSize() takes an integer as parameter, so rounded value should be checked
    if(d->m_dirtyScaledFont)
    {
       if( qRound( font().pointSize() * scale() ) <= 0 )
       {
           d->m_scaledFont = font();
       }
       else
       {
           d->m_scaledFont = font();
           d->m_scaledFont.setPointSize( qRound( d->m_scaledFont.pointSize() * scale() ));
       }

       d->m_dirtyScaledFont = false;
    }

    return d->m_scaledFont;
}

bool GeoDataLabelStyle::glow() const
{
    return d->m_glow;
}

void GeoDataLabelStyle::setGlow(bool on)
{
    d->m_glow = on;
}

bool GeoDataLabelStyle::highlightText() const
{
    return d->m_highlightText;
}

void GeoDataLabelStyle::setHighlightText(bool on)
{
    d->m_highlightText = on;
}

bool GeoDataLabelStyle::showLabel() const
{
    return d->m_showLabel;
}

void GeoDataLabelStyle::setShowLabel(bool on)
{
    d->m_showLabel = on;
}

}
