#include "GeoDataMultiPolygon.h"

//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008      Torsten Rahn   <rahn@kde.org>
// Copyright 2009 Patrick Spendrin <ps_ml@gmx.de>
//


#include "geodata/data/GeoDataMultiPolygon.h"
#include "geodata/data/GeoDataGeometry_p.h"
#include "geodata/parser/GeoDataTypes.h"
#include "service/plot/GeometryHelper.h"

#include "geodata/data/GeoDataPolygon.h"
#include "MarbleMath.h"
#include "Quaternion.h"
#include "MarbleDebug.h"

#include <QDataStream>
#include <QtGui/QPolygonF>

namespace Marble
{

class GeoDataMultiPolygonPrivate : public GeoDataGeometryPrivate
{
  public:
    explicit GeoDataMultiPolygonPrivate( const QVector<Marble::GeoDataPolygon > &polygons )
        :  m_dirtyBox( true ),
          aPolygons(polygons)
    {
    }

    explicit GeoDataMultiPolygonPrivate()
        :  m_dirtyBox( true )
    {
    }

    
    ~GeoDataMultiPolygonPrivate() override
    {
    }

    GeoDataMultiPolygonPrivate& operator=( const GeoDataMultiPolygonPrivate &other)
    {
        GeoDataGeometryPrivate::operator=( other );
        m_dirtyBox = other.m_dirtyBox;
        aPolygons = other.aPolygons;
        return *this;
    }


    
    GeoDataMultiPolygonPrivate* copy() override;

    const char* nodeType() const override
    {
        return GeoDataTypes::GeoDataMultiPolygonType;
    }


    mutable bool m_dirtyBox; // tells whether there have been changes to the
                                            // GeoDataPoints since the LatLonAltBox has
                                            // been calculated. Saves performance.

    QVector<Marble::GeoDataPolygon> aPolygons;
};



GeoDataMultiPolygonPrivate *GeoDataMultiPolygonPrivate::copy()
{
    GeoDataMultiPolygonPrivate* copy = new GeoDataMultiPolygonPrivate;
    *copy = *this;
    return copy;
}


GeoDataMultiPolygon::GeoDataMultiPolygon(QVector<Marble::GeoDataPolygon> polygons)
    :   GeoDataGeometry(new GeoDataMultiPolygonPrivate(polygons))
{

}

GeoDataMultiPolygon::GeoDataMultiPolygon()
    : GeoDataGeometry( new GeoDataMultiPolygonPrivate() )
{

}

GeoDataMultiPolygon::GeoDataMultiPolygon( GeoDataMultiPolygonPrivate* priv )
  : GeoDataGeometry( priv )
{
//    mDebug() << "2) GeoDataMultiPolygon created:" << p();
}

GeoDataMultiPolygon::GeoDataMultiPolygon( const GeoDataGeometry & other )
  : GeoDataGeometry( other )
{
//    mDebug() << "3) GeoDataMultiPolygon created:" << p();
}

GeoDataMultiPolygon::~GeoDataMultiPolygon()
{
#ifdef DEBUG_GEODATA
    mDebug() << "delete Linestring";
#endif
}

GeoDataMultiPolygonPrivate*
GeoDataMultiPolygon::p()
{
    return static_cast<GeoDataMultiPolygonPrivate*>(d);
}

const GeoDataMultiPolygonPrivate*
GeoDataMultiPolygon::p() const
{
    return static_cast<GeoDataMultiPolygonPrivate*>(d);
}

bool
GeoDataMultiPolygon::operator==( const GeoDataMultiPolygon &other ) const
{
    if ( !GeoDataGeometry::equals(other) && p()->aPolygons.size() != other.p()->aPolygons.size() ) {
        return false;
    }

    for(int i = 0; i < p()->aPolygons.size(); ++i)
    {
        if(p()->aPolygons[i].operator!=(other.p()->aPolygons[i]))
        {
            return false;
        }
    }

    return true;
}

bool GeoDataMultiPolygon::operator!=( const GeoDataMultiPolygon &other ) const
{
    return !this->operator==(other);
}

const QVector<GeoDataPolygon> &
GeoDataMultiPolygon::polygons() const
{
    return p()->aPolygons;
}

const GeoDataLatLonAltBox& GeoDataMultiPolygon::latLonAltBox() const
{
    if(p()->m_dirtyBox)
    {
        auto it = p()->aPolygons.constBegin();
        auto end = p()->aPolygons.constEnd();

        p()->m_latLonAltBox.clear();
        for (; it != end; ++it) {
            if ( !(*it).latLonAltBox().isEmpty() ) {
                if ( p()->m_latLonAltBox.isEmpty() ) {
                    p()->m_latLonAltBox = (*it).latLonAltBox();
                }
                else {
                    p()->m_latLonAltBox |= (*it).latLonAltBox();
                }
            }
        }
        p()->m_dirtyBox = false;
    }
    return p()->m_latLonAltBox;
}

}
