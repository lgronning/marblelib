//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Patrick Spendrin <ps_ml@gmx.de>
//

#ifndef MARBLE_GEODATAMULTIGEOMETRYPRIVATE_H
#define MARBLE_GEODATAMULTIGEOMETRYPRIVATE_H

#include "geodata/data/GeoDataGeometry_p.h"

#include "geodata/data/GeoDataPoint.h"
#include "geodata/data/GeoDataPolygon.h"
#include "geodata/data/GeoDataLineString.h"
#include "geodata/data/GeoDataLinearRing.h"
#include "geodata/data/GeoDataModel.h"
#include "geodata/parser/GeoDataTypes.h"
#include <QtCore/QHash>

namespace Marble
{

class GeoDataMultiGeometryPrivate : public GeoDataGeometryPrivate
{
  public:
    GeoDataMultiGeometryPrivate()
        :   m_dirtyBox(true)
    {
    }

    ~GeoDataMultiGeometryPrivate() override
    {
        qDeleteAll(m_vector);
    }

    GeoDataMultiGeometryPrivate& operator=( const GeoDataMultiGeometryPrivate &other)
    {
        GeoDataGeometryPrivate::operator=( other );

        qDeleteAll( m_vector );
        foreach( GeoDataGeometry *geometry, other.m_vector ) {
            GeoDataGeometry *newGeometry;

            // This piece of code has been used for a couple of times. Isn't it possible
            // to add a virtual method copy() similar to how abstract view does this?
            if ( geometry->nodeType() == GeoDataTypes::GeoDataLineStringType ) {
                newGeometry = new GeoDataLineString( *geometry );
            } else if ( geometry->nodeType() == GeoDataTypes::GeoDataPointType ) {
                // FIXME: Doesn't have a constructor which creates the object from a
                // GeoDataGeometry so cast is needed.
                newGeometry = new GeoDataPoint( *static_cast<GeoDataPoint*>( geometry ) );
            } else if ( geometry->nodeType() == GeoDataTypes::GeoDataModelType ) {
                // FIXME: Doesn't have a constructor which creates the object from a
                // GeoDataGeometry so cast is needed.
                newGeometry = new GeoDataModel( *static_cast<GeoDataModel*>( geometry ) );
            } else if ( geometry->nodeType() == GeoDataTypes::GeoDataPolygonType ) {
                newGeometry = new GeoDataPolygon( *geometry );
            } else if ( geometry->nodeType() == GeoDataTypes::GeoDataLinearRingType ) {
                newGeometry = new GeoDataLinearRing( *geometry );
            }

            m_vector.append( newGeometry );
        }

        m_dirtyBox = true;
        return *this;
    }

    GeoDataGeometryPrivate* copy() override
    { 
        GeoDataMultiGeometryPrivate* copy = new GeoDataMultiGeometryPrivate;
        *copy = *this;
        return copy;
    }

    const char* nodeType() const override
    {
        return GeoDataTypes::GeoDataMultiGeometryType;
    }

    EnumGeometryId geometryId() const override
    {
        return GeoDataMultiGeometryId;
    }
    QVector<GeoDataGeometry*>  m_vector;
    QHash<const GeoDataGeometry*, int> m_childPosition;
    mutable bool m_dirtyBox;
};

} // namespace Marble

#endif
