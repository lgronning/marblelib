//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2014      Sanjiban Bairagya <sanjiban22393@gmail.com>
//

#ifndef GEODATACREATE_H
#define GEODATACREATE_H

#include <QString>
#include <QDateTime>

#include "geodata/data/GeoDataContainer.h"
#include "marble_export.h"

namespace Marble
{

class GeoDataCreatePrivate;

class MARBLE_EXPORT GeoDataCreate : public GeoDataContainer
{
public:
    GeoDataCreate();

    GeoDataCreate( const GeoDataCreate &other );

    GeoDataCreate& operator=( const GeoDataCreate &other );

    bool operator==( const GeoDataCreate &other ) const;
    bool operator!=( const GeoDataCreate &other ) const;

    ~GeoDataCreate() override;

    /** Provides type information for downcasting a GeoNode */
    const char* nodeType() const override;

private:
    GeoDataCreatePrivate* const d;
};

}

#endif
