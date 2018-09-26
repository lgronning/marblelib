//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Patrick Spendrin <ps_ml@gmx.de>
//

#ifndef MARBLE_GEODATAPOLYGONPRIVATE_H
#define MARBLE_GEODATAPOLYGONPRIVATE_H

#include "geodata/data/GeoDataGeometry_p.h"

#include "geodata/parser/GeoDataTypes.h"

namespace Marble
{

class GeoDataPolygonPrivate : public GeoDataGeometryPrivate
{
  public:
     explicit GeoDataPolygonPrivate( TessellationFlags f )
         : m_dirtyBox( true ),
           m_tessellationFlags( f )
    {
    }

     GeoDataPolygonPrivate()
         : m_dirtyBox( true )
    {
    }

    GeoDataGeometryPrivate* copy() override
    { 
         GeoDataPolygonPrivate* copy = new  GeoDataPolygonPrivate;
        *copy = *this;
        return copy;
    }

    const char* nodeType() const override
    {
        return GeoDataTypes::GeoDataPolygonType;
    }

    EnumGeometryId geometryId() const override
    {
        return GeoDataPolygonId;
    }

    GeoDataLinearRing           outer;
    QVector<GeoDataLinearRing>  inner;
    bool                        m_dirtyBox; // tells whether there have been changes to the
                                            // GeoDataPoints since the LatLonAltBox has 
                                            // been calculated. Saves performance. 
    TessellationFlags           m_tessellationFlags;
};

} // namespace Marble

#endif
