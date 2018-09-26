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


#include "geodata/data/GeoDataMultiLineString.h"
#include "geodata/data/GeoDataGeometry_p.h"
#include "geodata/parser/GeoDataTypes.h"
#include "service/plot/GeometryHelper.h"

#include "geodata/data/GeoDataLinearRing.h"
#include "MarbleMath.h"
#include "Quaternion.h"
#include "MarbleDebug.h"

#include <QDataStream>
#include <QtGui/QPolygonF>

namespace Marble
{

class GeoDataMultiLineStringPrivate : public GeoDataGeometryPrivate
{
  public:
    explicit GeoDataMultiLineStringPrivate( const QVector<QVector<GeoDataCoordinates> > &points, TessellationFlags f, const QVector<QVector<double> > &messures, const QVector<QVector<double> > &messureInfos )
        :  m_dirtyBox( true )
    {
        for(int i = 0; i < points.size(); ++i)
        {
            aLineStrings.append(GeoDataLineString(points[i], f, messures.value(i), messureInfos.value(i)));
        }
    }

    explicit GeoDataMultiLineStringPrivate( const QVector<QVector<QPointF> > &points, TessellationFlags f, const QVector<QVector<double> > &messures, const QVector<QVector<double> > &messureInfos)
        :  m_dirtyBox( true )
    {
        for(int i = 0; i < points.size(); ++i)
        {
            aLineStrings.append(GeoDataLineString(points[i], f, messures.value(i), messureInfos.value(i)));
        }
    }

    explicit GeoDataMultiLineStringPrivate( const QVector<QVector<QwtPoint3D> > &points, TessellationFlags f, const QVector<QVector<double> > &messures, const QVector<QVector<double> > &messureInfos)
        :  m_dirtyBox( true )
    {
        for(int i = 0; i < points.size(); ++i)
        {
            aLineStrings.append(GeoDataLineString(points[i], f, messures.value(i), messureInfos.value(i)));
        }
    }


    explicit GeoDataMultiLineStringPrivate()
        :  m_dirtyBox( true )
    {
    }

    
    ~GeoDataMultiLineStringPrivate() override
    {
    }

    GeoDataMultiLineStringPrivate& operator=( const GeoDataMultiLineStringPrivate &other)
    {
        GeoDataGeometryPrivate::operator=( other );
        m_dirtyBox = other.m_dirtyBox;
        aLineStrings = other.aLineStrings;
        return *this;
    }


    
    GeoDataMultiLineStringPrivate* copy() override;

    const char* nodeType() const override
    {
        return GeoDataTypes::GeoDataMultiLineStringType;
    }


    mutable bool m_dirtyBox; // tells whether there have been changes to the
                                            // GeoDataPoints since the LatLonAltBox has
                                            // been calculated. Saves performance.

    QVector<GeoDataLineString> aLineStrings;
};



GeoDataMultiLineStringPrivate *GeoDataMultiLineStringPrivate::copy()
{
    GeoDataMultiLineStringPrivate* copy = new GeoDataMultiLineStringPrivate;
    *copy = *this;
    return copy;
}


GeoDataMultiLineString::GeoDataMultiLineString(const QVector<QVector<GeoDataCoordinates> >  &points, TessellationFlags f, const QVector<QVector<double> > &messures, const QVector<QVector<double> > &messureInfos)
    : GeoDataGeometry( new GeoDataMultiLineStringPrivate( points,  f, messures, messureInfos) )
{

}

GeoDataMultiLineString::GeoDataMultiLineString(const QVector<QVector<QPointF> > &points, TessellationFlags f , const QVector<QVector<double> > &messures, const QVector<QVector<double> > &messureInfos)
  : GeoDataGeometry( new GeoDataMultiLineStringPrivate( points, f, messures, messureInfos ) )
{
}

GeoDataMultiLineString::GeoDataMultiLineString(const QVector<QVector<QwtPoint3D> > &points, TessellationFlags f, const QVector<QVector<double> > &messures, const QVector<QVector<double> > &messureInfos)
    : GeoDataGeometry( new GeoDataMultiLineStringPrivate( points, f, messures, messureInfos ) )
{

}

GeoDataMultiLineString::GeoDataMultiLineString()
    : GeoDataGeometry( new GeoDataMultiLineStringPrivate() )
{

}

GeoDataMultiLineString::GeoDataMultiLineString( GeoDataMultiLineStringPrivate* priv )
  : GeoDataGeometry( priv )
{
//    mDebug() << "2) GeoDataMultiLineString created:" << p();
}

GeoDataMultiLineString::GeoDataMultiLineString( const GeoDataGeometry & other )
  : GeoDataGeometry( other )
{
//    mDebug() << "3) GeoDataMultiLineString created:" << p();
}

GeoDataMultiLineString::~GeoDataMultiLineString()
{
#ifdef DEBUG_GEODATA
    mDebug() << "delete Linestring";
#endif
}

GeoDataMultiLineStringPrivate* GeoDataMultiLineString::p()
{
    return static_cast<GeoDataMultiLineStringPrivate*>(d);
}

const GeoDataMultiLineStringPrivate* GeoDataMultiLineString::p() const
{
    return static_cast<GeoDataMultiLineStringPrivate*>(d);
}

bool GeoDataMultiLineString::operator==( const GeoDataMultiLineString &other ) const
{
    if ( !GeoDataGeometry::equals(other) && p()->aLineStrings.size() != other.p()->aLineStrings.size() ) {
        return false;
    }

    for(int i = 0; i < p()->aLineStrings.size(); ++i)
    {
        if(p()->aLineStrings[i].operator!=(other.p()->aLineStrings[i]))
        {
            return false;
        }
    }

    return true;
}

bool GeoDataMultiLineString::operator!=( const GeoDataMultiLineString &other ) const
{
    return !this->operator==(other);
}

const QVector<GeoDataLineString> &
GeoDataMultiLineString::lineStrings() const
{
    return p()->aLineStrings;
}

const GeoDataLatLonAltBox& GeoDataMultiLineString::latLonAltBox() const
{
    if(p()->m_dirtyBox)
    {
        auto it = p()->aLineStrings.constBegin();
        auto end = p()->aLineStrings.constEnd();

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
