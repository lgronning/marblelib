//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008      Patrick Spendrin <ps_ml@gmx.de>
//


#include "geodata/data/GeoDataStyleMap.h"
#include <QDataStream>

#include "geodata/parser/GeoDataTypes.h"

namespace Marble
{

class GeoDataStyleMapPrivate
{
  public:
    const char* nodeType() const
    {
        return GeoDataTypes::GeoDataStyleMapType;
    }

    QString lastKey;
};


GeoDataStyleMap::GeoDataStyleMap()
    : d( new GeoDataStyleMapPrivate )
{
}

GeoDataStyleMap::GeoDataStyleMap( const GeoDataStyleMap& other )
    : GeoDataStyleSelector( other ) , QMap<QString,QString>(other), d( new GeoDataStyleMapPrivate( *other.d ) )

{
}

GeoDataStyleMap::~GeoDataStyleMap()
{
    delete d;
}

const char* GeoDataStyleMap::nodeType() const
{
    return d->nodeType();
}

QString GeoDataStyleMap::lastKey() const
{
    return d->lastKey;
}

void GeoDataStyleMap::setLastKey( QString key )
{
    d->lastKey = key;
}

GeoDataStyleMap& GeoDataStyleMap::operator=( const GeoDataStyleMap& other )
{
    QMap<QString, QString>::operator=( other );
    GeoDataStyleSelector::operator=( other );
    *d = *other.d;
    return *this;
}

bool GeoDataStyleMap::operator==( const GeoDataStyleMap &other ) const
{
    if ( GeoDataStyleSelector::operator!=( other ) ||
         QMap<QString, QString>::operator!=( other ) )
    {
        return false;
    }

    return d->lastKey == other.d->lastKey;
}

bool GeoDataStyleMap::operator!=( const GeoDataStyleMap &other ) const
{
    return !this->operator==( other );
}

}
