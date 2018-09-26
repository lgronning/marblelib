/*
    Copyright (C) 2007 Murad Tagirov <tmurad@gmail.com>
    Copyright (C) 2007 Nikolas Zimmermann <zimmermann@kde.org>

    This file is part of the KDE project

    This library is free software you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    aint with this library see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "geodata/data/GeoDataFolder.h"

#include "geodata/parser/GeoDataTypes.h"

#include "geodata/data/GeoDataContainer_p.h"

namespace Marble
{

class GeoDataFolderPrivate : public GeoDataContainerPrivate
{
  public:
    GeoDataFolderPrivate()
    {
    }

    GeoDataFeaturePrivate* copy() override
    {
        GeoDataFolderPrivate* copy = new GeoDataFolderPrivate;
        *copy = *this;
        return copy;
    }

    const char* nodeType() const override
    {
        return GeoDataTypes::GeoDataFolderType;
    }
};


GeoDataFolder::GeoDataFolder()
        : GeoDataContainer( new GeoDataFolderPrivate )
{
    setVisualCategory( GeoDataFeature::Folder );
}

GeoDataFolder::GeoDataFolder( const GeoDataFolder& other )
    = default;

GeoDataFolder::~GeoDataFolder()
= default;

GeoDataFolderPrivate* GeoDataFolder::p() const
{
    return static_cast<GeoDataFolderPrivate*>(d);
}

bool GeoDataFolder::operator==( const GeoDataFolder &other ) const
{
    return GeoDataContainer::equals( other );
}

bool GeoDataFolder::operator!=( const GeoDataFolder &other ) const
{
    return !this->operator==( other );
}

}
