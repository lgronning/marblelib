//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Patrick Spendrin <ps_ml@gmx.de>
//

#ifndef MARBLE_GEODATACONTAINERPRIVATE_H
#define MARBLE_GEODATACONTAINERPRIVATE_H

#include "geodata/data/GeoDataFeature_p.h"

#include "geodata/parser/GeoDataTypes.h"
#include <QtCore/QHash>

namespace Marble
{

class GeoDataContainerPrivate : public GeoDataFeaturePrivate
{
  public:
    GeoDataContainerPrivate()
    {
    }
    
    ~GeoDataContainerPrivate() override
    {
        qDeleteAll( m_vector );
    }

    GeoDataContainerPrivate& operator=( const GeoDataContainerPrivate &other)
    {
        GeoDataFeaturePrivate::operator=( other );
        qDeleteAll( m_vector );
        foreach( GeoDataFeature *feature, other.m_vector )
        {
            m_vector.append( new GeoDataFeature( *feature ) );
        }
        return *this;
    }

    GeoDataFeaturePrivate* copy() override
    { 
        GeoDataContainerPrivate* copy = new GeoDataContainerPrivate;
        *copy = *this;
        return copy;
    }

    const char* nodeType() const override
    {
        return GeoDataTypes::GeoDataContainerType;
    }

    EnumFeatureId featureId() const override
    {
        return GeoDataFolderId;
    }

    QVector<GeoDataFeature*> m_vector;
    QHash<const GeoDataFeature*, int> m_childPosition;
};

} // namespace Marble

#endif
