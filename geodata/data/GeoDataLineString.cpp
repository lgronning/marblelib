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


#include "geodata/data/GeoDataLineString.h"
#include "geodata/data/GeoDataLineString_p.h"
#include "service/plot/GeometryHelper.h"

#include "geodata/data/GeoDataLinearRing.h"
#include "MarbleMath.h"
#include "Quaternion.h"
#include "MarbleDebug.h"

#include <QDataStream>
#include <QtGui/QPolygonF>

namespace Marble
{
GeoDataLineString::GeoDataLineString(const QVector<GeoDataCoordinates> &points, TessellationFlags f, const QVector<double> &messure, const QVector<double> &messureInfo)
    : GeoDataGeometry( new GeoDataLineStringCoordinatesPrivate( points,  f, messure, messureInfo) )
{

}

GeoDataLineString::GeoDataLineString(const QVector<QPointF> &points, TessellationFlags f , const QVector<double> &messure, const QVector<double> &messureInfo)
  : GeoDataGeometry( new GeoDataLineStringPointsPrivate( points, f, messure, messureInfo) )
{
    //    mDebug() << "1) GeoDataLineString created:" << p();
}

GeoDataLineString::GeoDataLineString(const QVector<QwtPoint3D> &points, TessellationFlags f, const QVector<double> &messure, const QVector<double> &messureInfo )
    : GeoDataGeometry( new GeoDataLineStringPoints3DPrivate( points, f, messure, messureInfo ) )
{


}

GeoDataLineString::GeoDataLineString()
    : GeoDataGeometry( new GeoDataLineStringCoordinatesPrivate() )
{

}

GeoDataLineString::GeoDataLineString( GeoDataLineStringPrivate* priv )
  : GeoDataGeometry( priv )
{
//    mDebug() << "2) GeoDataLineString created:" << p();
}

GeoDataLineString::GeoDataLineString( const GeoDataGeometry & other )
  : GeoDataGeometry( other )
{
//    mDebug() << "3) GeoDataLineString created:" << p();
}

GeoDataLineString::~GeoDataLineString()
{
#ifdef DEBUG_GEODATA
    mDebug() << "delete Linestring";
#endif
}

GeoDataLineStringPrivate* GeoDataLineString::p()
{
    return static_cast<GeoDataLineStringPrivate*>(d);
}

const GeoDataLineStringPrivate* GeoDataLineString::p() const
{
    return static_cast<GeoDataLineStringPrivate*>(d);
}

int GeoDataLineStringPrivate::levelForResolution(qreal resolution) const {
    if (m_previousResolution == resolution) return m_level;

    m_previousResolution = resolution;

    if (resolution < 0.0000005) m_level = 17;
    else if (resolution < 0.0000010) m_level = 16;
    else if (resolution < 0.0000020) m_level = 15;
    else if (resolution < 0.0000040) m_level = 14;
    else if (resolution < 0.0000080) m_level = 13;
    else if (resolution < 0.0000160) m_level = 12;
    else if (resolution < 0.0000320) m_level = 11;
    else if (resolution < 0.0000640) m_level = 10;
    else if (resolution < 0.0001280) m_level = 9;
    else if (resolution < 0.0002560) m_level = 8;
    else if (resolution < 0.0005120) m_level = 7;
    else if (resolution < 0.0010240) m_level = 6;
    else if (resolution < 0.0020480) m_level = 5;
    else if (resolution < 0.0040960) m_level = 4;
    else if (resolution < 0.0081920) m_level = 3;
    else if (resolution < 0.0163840) m_level = 2;
    else m_level =  1;

    return m_level;
}

qreal GeoDataLineStringPrivate::resolutionForLevel(int level) const {
    switch (level) {
        case 0:
            return 0.0655360;
            break;
        case 1:
            return 0.0327680;
            break;
        case 2:
            return 0.0163840;
            break;
        case 3:
            return 0.0081920;
            break;
        case 4:
            return 0.0040960;
            break;
        case 5:
            return 0.0020480;
            break;
        case 6:
            return 0.0010240;
            break;
        case 7:
            return 0.0005120;
            break;
        case 8:
            return 0.0002560;
            break;
        case 9:
            return 0.0001280;
            break;
        case 10:
            return 0.0000640;
            break;
        case 11:
            return 0.0000320;
            break;
        case 12:
            return 0.0000160;
            break;
        case 13:
            return 0.0000080;
            break;
        case 14:
            return 0.0000040;
            break;
        case 15:
            return 0.0000020;
            break;
        case 16:
            return 0.0000010;
            break;
        default:
        case 17:
            return 0.0000005;
            break;
    }
}

void GeoDataLineStringPrivate::optimize (GeoDataLineString& lineString) const
{
    if (lineString.size() < 2) return;

    // Calculate the least non-zero detail-level by checking the bounding box
    int startLevel = levelForResolution( ( lineString.latLonAltBox().width() + lineString.latLonAltBox().height() ) / 2 );

    int currentLevel = startLevel;
    int maxLevel = startLevel;
    lineString.setDetail(0, startLevel);

    // Iterate through the linestring to assign different detail levels to the nodes.
    // In general the first and last node should have the start level assigned as
    // a detail level.
    // Starting from the first node the algorithm picks those nodes which
    // have a distance from each other that is just above the resolution that is
    // associated with the start level (which we use as a "current level").
    // Each of those nodes get the current level assigned as the detail level.
    // After iterating through the linestring we increment the current level value
    // and starting again with the first node we assign detail values in a similar way
    // to the remaining nodes which have no final detail level assigned yet.
    // We do as many iterations through the lineString as needed and bump up the
    // current level until all nodes have a non-zero detail level assigned.

    int itCoords = 0;
    int itEnd = lineString.size() -1 ;

    while ( currentLevel  < 16 && currentLevel <= maxLevel + 1 ) {
        itCoords = 0;

        double currentLon;
        double currentLat;

        lineString.getLonLat(itCoords, currentLon, currentLat, GeoDataCoordinates::Radian);
        ++itCoords;

        for( ; itCoords <= itEnd; ++itCoords)
        {
            if (lineString.detail(itCoords) != 0 && lineString.detail(itCoords) < currentLevel)
            {
                continue;
            }

            double lon1;
            double lat1;

            getLonLat(itCoords, lon1, lat1, GeoDataCoordinates::Radian);

            if ( currentLevel == startLevel && (lon1 == -M_PI || lon1 == M_PI
                || lat1 < -89 * DEG2RAD || lat1 > 89 * DEG2RAD))
            {
                lineString.setDetail(itCoords, startLevel);
                lineString.getLonLat(itCoords, currentLon, currentLat, GeoDataCoordinates::Radian);

                maxLevel = currentLevel;
                continue;
            }

            if (distanceSphere(currentLon, currentLat, lon1, lat1 ) < resolutionForLevel(currentLevel + 1))
            {
                lineString.setDetail(itCoords, currentLevel + 1);
            }
            else
            {
                lineString.setDetail(itCoords, currentLevel);
                lineString.getLonLat(itCoords, currentLon, currentLat, GeoDataCoordinates::Radian);
                maxLevel = currentLevel;
            }
        }
        ++currentLevel;
    }
    lineString.setDetail(itEnd, startLevel);
}

bool GeoDataLineString::isEmpty() const
{
    return p()->isEmpty();
}

int GeoDataLineString::size() const
{
    return p()->size();
}

int
GeoDataLineString::detail(int i) const
{
    return p()->m_details.value(i, 0);
}

void GeoDataLineString::setDetail(int i, int value)
{
    p()->m_details[i] = value;
}

double GeoDataLineString::altitude(int pos) const
{
    const GeoDataLineStringPrivate* d = p();

    if(d->type() == GeoDataLineStringPrivate::LineStringCoordinates)
    {
        return static_cast<const GeoDataLineStringCoordinatesPrivate *>(d)->m_points.at(pos).altitude();
    }

    if(d->type() == GeoDataLineStringPrivate::LineStringPoints3d)
    {
        return static_cast<const GeoDataLineStringPoints3DPrivate*>(d)->m_points.at(pos).z();
    }

    return  0;
}

double GeoDataLineString::messure(int pos) const
{
    return p()->messure(pos);
}


double GeoDataLineString::messureInfo(int pos) const
{
    return p()->messureInfo(pos);
}


void
GeoDataLineString::getLonLat(int pos, double &lon, double &lat, GeoDataCoordinates::Unit unit) const
{
    p()->getLonLat(pos, lon, lat, unit);
}

GeoDataCoordinates 
GeoDataLineString::at( int pos ) const
{
    return p()->at( pos );
}


bool GeoDataLineString::operator==( const GeoDataLineString &other ) const
{
    if ( !GeoDataGeometry::equals(other) ||
          size() != other.size() ||
          tessellate() != other.tessellate() ) {
        return false;
    }

    const GeoDataLineStringPrivate* d = p();
    const GeoDataLineStringPrivate* other_d = other.p();

    if(d->type() != other_d->type())
    {
        return false;
    }


    if(d->type() == GeoDataLineStringPrivate::LineStringCoordinates)
    {
        QVector<GeoDataCoordinates>::const_iterator itCoords = static_cast<const GeoDataLineStringCoordinatesPrivate *>(d)->m_points.constBegin();
        QVector<GeoDataCoordinates>::const_iterator otherItCoords = static_cast<const GeoDataLineStringCoordinatesPrivate *>(other_d)->m_points.constBegin();
        QVector<GeoDataCoordinates>::const_iterator itEnd = static_cast<const GeoDataLineStringCoordinatesPrivate *>(d)->m_points.constEnd();
        QVector<GeoDataCoordinates>::const_iterator otherItEnd = static_cast<const GeoDataLineStringCoordinatesPrivate *>(other_d)->m_points.constEnd();

        for ( ; itCoords != itEnd && otherItCoords != otherItEnd; ++itCoords, ++otherItCoords ) {
            if ( *itCoords != *otherItCoords ) {
                return false;
            }
        }
        Q_ASSERT ( itCoords == itEnd && otherItCoords == otherItEnd );
    }
    else if(d->type() == GeoDataLineStringPrivate::LineStringPoints)
    {
        return static_cast<const GeoDataLineStringPointsPrivate*>(d)->m_points == static_cast<const GeoDataLineStringPointsPrivate*>(other_d)->m_points;
    }
    else if(d->type() == GeoDataLineStringPrivate::LineStringPoints3d)
    {
        return static_cast<const GeoDataLineStringPoints3DPrivate*>(d)->m_points == static_cast<const GeoDataLineStringPoints3DPrivate*>(other_d)->m_points;
    }

    return true;
}

bool GeoDataLineString::operator!=( const GeoDataLineString &other ) const
{
    return !this->operator==(other);
}

bool GeoDataLineString::isClosed() const
{
    return false;
}

bool GeoDataLineString::tessellate() const
{
    return p()->m_tessellationFlags.testFlag(Tessellate);
}

void GeoDataLineString::setTessellate( bool tessellate )
{
    GeoDataGeometry::detach();
    // According to the KML reference the tesselation of line strings in Google Earth
    // is generally done along great circles. However for subsequent points that share
    // the same latitude the latitude circles are followed. Our Tesselate and RespectLatitude
    // Flags provide this behaviour. For true polygons the latitude circles don't get considered.

    if ( tessellate ) {
        p()->m_tessellationFlags |= Tessellate;
        p()->m_tessellationFlags |= RespectLatitudeCircle;
    } else {
        p()->m_tessellationFlags ^= Tessellate;
        p()->m_tessellationFlags ^= RespectLatitudeCircle;
    }
}

TessellationFlags GeoDataLineString::tessellationFlags() const
{
    return p()->m_tessellationFlags;
}

void GeoDataLineString::setTessellationFlags( TessellationFlags f )
{
    p()->m_tessellationFlags = f;
}

GeoDataLineString GeoDataLineString::toNormalized() const
{
    qreal lon;
    qreal lat;

    const GeoDataLineStringPrivate* d = p();

    if(d->type() == GeoDataLineStringPrivate::LineStringCoordinates)
    {
        QVector<GeoDataCoordinates> coordinates;

        QVector<GeoDataCoordinates>::const_iterator end = static_cast<const GeoDataLineStringCoordinatesPrivate*>(d)->m_points.constEnd();
        for( QVector<GeoDataCoordinates>::const_iterator itCoords = static_cast<const GeoDataLineStringCoordinatesPrivate*>(d)->m_points.constBegin(); itCoords != end; ++itCoords )
        {
            itCoords->geoCoordinates( lon, lat );
            qreal alt = itCoords->altitude();
            GeoDataCoordinates::normalizeLonLat( lon, lat );

            GeoDataCoordinates normalizedCoords( *itCoords );
            normalizedCoords.set( lon, lat, alt );
            coordinates << normalizedCoords;
        }

        return GeoDataLineString(coordinates, tessellationFlags());
    }
    else if(d->type() == GeoDataLineStringPrivate::LineStringPoints)
    {
        QVector<QPointF> coordinates;

        QVector<QPointF>::const_iterator end = static_cast<const GeoDataLineStringPointsPrivate*>(d)->m_points.constEnd();
        for( QVector<QPointF>::const_iterator itCoords = static_cast<const GeoDataLineStringPointsPrivate*>(d)->m_points.constBegin(); itCoords != end; ++itCoords )
        {
            lon = itCoords->x();
            lat = itCoords->y();
            GeoDataCoordinates::normalizeLonLat(lon,lat, GeoDataCoordinates::Degree);

            QPointF normalizedCoords(lon, lat);
            coordinates << normalizedCoords;
        }

        return GeoDataLineString(coordinates, tessellationFlags());
    }
    else if(d->type() == GeoDataLineStringPrivate::LineStringPoints3d)
    {
        QVector<QwtPoint3D> coordinates;

        QVector<QwtPoint3D>::const_iterator end = static_cast<const GeoDataLineStringPoints3DPrivate*>(d)->m_points.constEnd();
        for( QVector<QwtPoint3D>::const_iterator itCoords = static_cast<const GeoDataLineStringPoints3DPrivate*>(d)->m_points.constBegin(); itCoords != end; ++itCoords )
        {
            lon = itCoords->x();
            lat = itCoords->y();
            GeoDataCoordinates::normalizeLonLat(lon,lat, GeoDataCoordinates::Degree);

            QwtPoint3D normalizedCoords(lon, lat, itCoords->z());
            coordinates << normalizedCoords;
        }

        return GeoDataLineString(coordinates, tessellationFlags());
    }

    // FIXME: Think about how we can avoid unnecessary copies
    //        if the linestring stays the same.


    return GeoDataLineString();
}

const GeoDataLatLonAltBox& GeoDataLineString::latLonAltBox() const
{
    // GeoDataLatLonAltBox::fromLineString is very expensive
    // that's why we recreate it only if the m_dirtyBox
    // is TRUE.
    // DO NOT REMOVE THIS CONSTRUCT OR MARBLE WILL BE SLOW.
    if ( p()->m_dirtyBox ) {
        p()->m_latLonAltBox = GeoDataLatLonAltBox::fromLineString( *this );
    }
    p()->m_dirtyBox = false;

    return p()->m_latLonAltBox;
}

qreal GeoDataLineString::length( qreal planetRadius, int offset ) const
{
    if( offset < 0 || offset >= size() ) {
        return 0;
    }

    qreal length = 0.0;
    int const start = qMax(offset+1, 1);
    int const end = size();
    for( int i=start; i<end; ++i )
    {
        double lon1;
        double lat1;

        getLonLat(i-1, lon1, lat1, GeoDataCoordinates::Radian);

        double lon2;
        double lat2;

        getLonLat(i, lon2, lat2, GeoDataCoordinates::Radian);

        length += distanceSphere(lon1, lat1, lon2, lat2);
    }

    return planetRadius * length;
}

GeoDataLineString GeoDataLineString::optimized () const
{
    if( isClosed() ) {
        GeoDataLinearRing linearRing(*this);
        p()->optimize(linearRing);
        return linearRing;
    } else {
        GeoDataLineString lineString(*this);
        p()->optimize(lineString);
        return lineString;
    }
}

QVector<QPointF> GeoDataLineString::rawData() const
{
    const GeoDataLineStringPrivate* d = p();

    QVector<QPointF> data;
    if(d->type() == GeoDataLineStringPrivate::LineStringCoordinates)
    {
        QVector<GeoDataCoordinates>::const_iterator end = static_cast<const GeoDataLineStringCoordinatesPrivate*>(d)->m_points.constEnd();
        for( QVector<GeoDataCoordinates>::const_iterator itCoords = static_cast<const GeoDataLineStringCoordinatesPrivate*>(d)->m_points.constBegin(); itCoords != end; ++itCoords )
        {
            double lon, lat;
            itCoords->geoCoordinates( lon, lat, Marble::GeoDataCoordinates::Degree );
            data << QPointF(lon, lat);
        }
    }
    else if(d->type() == GeoDataLineStringPrivate::LineStringPoints)
    {
        data = static_cast<const GeoDataLineStringPointsPrivate*>(d)->m_points;
    }
    else if(d->type() == GeoDataLineStringPrivate::LineStringPoints3d)
    {
        QVector<QwtPoint3D>::const_iterator end = static_cast<const GeoDataLineStringPoints3DPrivate*>(d)->m_points.constEnd();
        for( QVector<QwtPoint3D>::const_iterator itCoords = static_cast<const GeoDataLineStringPoints3DPrivate*>(d)->m_points.constBegin(); itCoords != end; ++itCoords )
        {
            data << QPointF(itCoords->x(), itCoords->y());
        }
    }

    return data;
}

bool GeoDataLineString::hasMessure() const
{
    return p()->hasMessure();
}

bool GeoDataLineString::hasMessureInfo() const
{
    return p()->hasMessureInfo();
}

void
GeoDataLineStringCoordinatesPrivate::getLonLat(int at, double &lon, double &lat, GeoDataCoordinates::Unit unit) const
{
    GeoDataCoordinates point = m_points.at(at);
    lon = point.longitude(unit);
    lat = point.latitude(unit);
}

void GeoDataLineStringPointsPrivate::getLonLat(int at, double &lon, double &lat, GeoDataCoordinates::Unit unit) const
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

void GeoDataLineStringPoints3DPrivate::getLonLat(int at, double &lon, double &lat, GeoDataCoordinates::Unit unit) const
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
