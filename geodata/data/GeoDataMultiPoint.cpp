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


#include "geodata/data/GeoDataMultiPoint.h"
#include "geodata/data/GeoDataMultiPoint_p.h"
#include "service/plot/GeometryHelper.h"

#include "geodata/data/GeoDataLinearRing.h"
#include "MarbleMath.h"
#include "Quaternion.h"
#include "MarbleDebug.h"

#include <QDataStream>
#include <QtGui/QPolygonF>

namespace Marble
{
GeoDataMultiPoint::GeoDataMultiPoint(const QVector<GeoDataCoordinates> &points)
    : GeoDataGeometry( new GeoDataMultiPointCoordinatesPrivate( points) )
{

}

GeoDataMultiPoint::GeoDataMultiPoint(const QVector<QPointF> &points )
  : GeoDataGeometry( new GeoDataMultiPointPointsPrivate( points) )
{
    //    mDebug() << "1) GeoDataMultiPoint created:" << p();
}

GeoDataMultiPoint::GeoDataMultiPoint(const QVector<QwtPoint3D> &points)
    : GeoDataGeometry( new GeoDataMultiPointPoints3DPrivate( points ) )
{

}

GeoDataMultiPoint::GeoDataMultiPoint()
    : GeoDataGeometry( new GeoDataMultiPointCoordinatesPrivate() )
{

}

GeoDataMultiPoint::GeoDataMultiPoint( GeoDataMultiPointPrivate* priv )
  : GeoDataGeometry( priv )
{
//    mDebug() << "2) GeoDataMultiPoint created:" << p();
}

GeoDataMultiPoint::GeoDataMultiPoint( const GeoDataGeometry & other )
  : GeoDataGeometry( other )
{
//    mDebug() << "3) GeoDataMultiPoint created:" << p();
}

GeoDataMultiPoint::~GeoDataMultiPoint()
{
#ifdef DEBUG_GEODATA
    mDebug() << "delete Linestring";
#endif
}

GeoDataMultiPointPrivate* GeoDataMultiPoint::p()
{
    return static_cast<GeoDataMultiPointPrivate*>(d);
}

const GeoDataMultiPointPrivate* GeoDataMultiPoint::p() const
{
    return static_cast<GeoDataMultiPointPrivate*>(d);
}

bool GeoDataMultiPoint::isEmpty() const
{
    return p()->isEmpty();
}

int GeoDataMultiPoint::size() const
{
    return p()->size();
}


double GeoDataMultiPoint::altitude(int pos) const
{
    const GeoDataMultiPointPrivate* d = p();

    if(d->type() == GeoDataMultiPointPrivate::Coordinates)
    {
        return static_cast<const GeoDataMultiPointCoordinatesPrivate *>(d)->m_points.at(pos).altitude();
    }
    else if(d->type() == GeoDataMultiPointPrivate::Points3d)
    {
        return static_cast<const GeoDataMultiPointPoints3DPrivate*>(d)->m_points.at(pos).z();
    }

    return  0;
}

void
GeoDataMultiPoint::getLonLat(int pos, double &lon, double &lat, GeoDataCoordinates::Unit unit) const
{
    p()->getLonLat(pos, lon, lat, unit);
}

GeoDataCoordinates
GeoDataMultiPoint::at( int pos ) const
{
    return p()->at( pos );
}


bool GeoDataMultiPoint::operator==( const GeoDataMultiPoint &other ) const
{
    if ( !GeoDataGeometry::equals(other) ||
          size() != other.size() ) {
        return false;
    }

    const GeoDataMultiPointPrivate* d = p();
    const GeoDataMultiPointPrivate* other_d = other.p();

    if(d->type() != other_d->type())
    {
        return false;
    }


    if(d->type() == GeoDataMultiPointPrivate::Coordinates)
    {
        QVector<GeoDataCoordinates>::const_iterator itCoords = static_cast<const GeoDataMultiPointCoordinatesPrivate *>(d)->m_points.constBegin();
        QVector<GeoDataCoordinates>::const_iterator otherItCoords = static_cast<const GeoDataMultiPointCoordinatesPrivate *>(other_d)->m_points.constBegin();
        QVector<GeoDataCoordinates>::const_iterator itEnd = static_cast<const GeoDataMultiPointCoordinatesPrivate *>(d)->m_points.constEnd();
        QVector<GeoDataCoordinates>::const_iterator otherItEnd = static_cast<const GeoDataMultiPointCoordinatesPrivate *>(other_d)->m_points.constEnd();

        for ( ; itCoords != itEnd && otherItCoords != otherItEnd; ++itCoords, ++otherItCoords ) {
            if ( *itCoords != *otherItCoords ) {
                return false;
            }
        }
        Q_ASSERT ( itCoords == itEnd && otherItCoords == otherItEnd );
    }
    else if(d->type() == GeoDataMultiPointPrivate::Points)
    {
        return static_cast<const GeoDataMultiPointPointsPrivate*>(d)->m_points == static_cast<const GeoDataMultiPointPointsPrivate*>(other_d)->m_points;
    }
    else if(d->type() == GeoDataMultiPointPrivate::Points3d)
    {
        return static_cast<const GeoDataMultiPointPoints3DPrivate*>(d)->m_points == static_cast<const GeoDataMultiPointPoints3DPrivate*>(other_d)->m_points;
    }

    return true;
}

bool GeoDataMultiPoint::operator!=( const GeoDataMultiPoint &other ) const
{
    return !this->operator==(other);
}


const GeoDataLatLonAltBox& GeoDataMultiPoint::latLonAltBox() const
{
    // GeoDataLatLonAltBox::fromLineString is very expensive
    // that's why we recreate it only if the m_dirtyBox
    // is TRUE.
    // DO NOT REMOVE THIS CONSTRUCT OR MARBLE WILL BE SLOW.
    if ( p()->m_dirtyBox )
    {
        const GeoDataMultiPointPrivate* d = p();
        if(d->type() == GeoDataMultiPointPrivate::Coordinates)
        {
            GeoDataLineString lineString(static_cast<const GeoDataMultiPointCoordinatesPrivate *>(d)->m_points);
            p()->m_latLonAltBox = GeoDataLatLonAltBox::fromLineString(lineString);

        }
        else if(d->type() == GeoDataMultiPointPrivate::Points)
        {
            GeoDataLineString lineString(static_cast<const GeoDataMultiPointPointsPrivate *>(d)->m_points);
            p()->m_latLonAltBox = GeoDataLatLonAltBox::fromLineString(lineString);
        }
        else if(d->type() == GeoDataMultiPointPrivate::Points3d)
        {
            GeoDataLineString lineString(static_cast<const GeoDataMultiPointPointsPrivate *>(d)->m_points);
            p()->m_latLonAltBox = GeoDataLatLonAltBox::fromLineString(lineString);
        }
    }
    p()->m_dirtyBox = false;

    return p()->m_latLonAltBox;
}

void
GeoDataMultiPointCoordinatesPrivate::getLonLat(int at, double &lon, double &lat, GeoDataCoordinates::Unit unit) const
{
    GeoDataCoordinates point = m_points.at(at);
    lon = point.longitude(unit);
    lat = point.latitude(unit);
}

void GeoDataMultiPointPointsPrivate::getLonLat(int at, double &lon, double &lat, GeoDataCoordinates::Unit unit) const
{
    QPointF point = m_points.at(at);
    lon = point.x();
    lat = point.y();

    if(unit == GeoDataCoordinates::Radian)
    {
        lon = lon * DEG2RAD;
        lat = lat * DEG2RAD;
    }
}

void GeoDataMultiPointPoints3DPrivate::getLonLat(int at, double &lon, double &lat, GeoDataCoordinates::Unit unit) const
{
    QwtPoint3D point = m_points.at(at);
    lon = point.x();
    lat = point.y();

    if(unit == GeoDataCoordinates::Radian)
    {
        lon = lon * DEG2RAD;
        lat = lat * DEG2RAD;
    }
}


}
