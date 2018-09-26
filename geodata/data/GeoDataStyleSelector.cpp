//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2007      Murad Tagirov <tmurad@gmail.com>
//


#include "geodata/data/GeoDataStyleSelector.h"
#include <QDataStream>

#include "geodata/parser/GeoDataTypes.h"

namespace Marble
{

class GeoDataStyleSelectorPrivate
{
};

GeoDataStyleSelector::GeoDataStyleSelector() :
    GeoDataObject(),
    d( nullptr )
{
}

GeoDataStyleSelector::GeoDataStyleSelector( const GeoDataStyleSelector& other ) :
    GeoDataObject( other ),
    d( nullptr )
{
}

GeoDataStyleSelector::~GeoDataStyleSelector()
{
    delete d;
}

GeoDataStyleSelector& GeoDataStyleSelector::operator=( const GeoDataStyleSelector& other )
{
    GeoDataObject::operator=( other );
    return *this;
}

bool GeoDataStyleSelector::operator==( const GeoDataStyleSelector &other ) const
{
    return GeoDataObject::equals( other );
}

bool GeoDataStyleSelector::operator!=( const GeoDataStyleSelector &other ) const
{
    return !this->operator==( other );
}

}
