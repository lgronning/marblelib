//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008      Torsten Rahn   <rahn@kde.org>
//


#include "geodata/data/GeoDataLinearRing.h"
#include "geodata/data/GeoDataLinearRing_p.h"

#include "MarbleMath.h"
#include "MarbleDebug.h"

namespace Marble
{



GeoDataLinearRing::GeoDataLinearRing(const QVector<GeoDataCoordinates> &points, TessellationFlags f, const QVector<double> &messure, const QVector<double> &messureInfo)
    : GeoDataLineString( new GeoDataLinearRingCoordinatesPrivate( points, f, messure, messureInfo ) )
{

}

GeoDataLinearRing::GeoDataLinearRing(const QVector<QPointF> &points, TessellationFlags f, const QVector<double> &messure, const QVector<double> &messureInfo)
    : GeoDataLineString( new GeoDataLinearRingPointsPrivate( points, f, messure, messureInfo ) )
{

}

GeoDataLinearRing::GeoDataLinearRing(const QVector<QwtPoint3D> &points, TessellationFlags f, const QVector<double> &messure, const QVector<double> &messureInfo)
    : GeoDataLineString( new GeoDataLinearRingPoints3DPrivate( points, f, messure, messureInfo ) )
{

}

GeoDataLinearRing::GeoDataLinearRing()
    : GeoDataLineString( )
{

}

GeoDataLinearRing::GeoDataLinearRing( const GeoDataGeometry & other )
  : GeoDataLineString( other )
{
}

GeoDataLinearRing::~GeoDataLinearRing()
= default;

bool GeoDataLinearRing::operator==( const GeoDataLinearRing &other ) const
{
    return isClosed() == other.isClosed() &&
           GeoDataLineString::operator==( other );
}

bool GeoDataLinearRing::operator!=( const GeoDataLinearRing &other ) const
{
    return !this->operator==(other);
}

bool GeoDataLinearRing::isClosed() const
{
    return true;
}

qreal GeoDataLinearRing::length( qreal planetRadius, int offset ) const
{
    qreal  length = GeoDataLineString::length( planetRadius, offset );

    if(size() == 0)
    {
        return length;
    }

    double lon1;
    double lat1;

    getLonLat(0, lon1, lat1, GeoDataCoordinates::Radian);

    double lon2;
    double lat2;

    getLonLat(size()-1, lon2, lat2, GeoDataCoordinates::Radian);


    return length + planetRadius * distanceSphere( lon2, lat2, lon1, lat1 );
}

bool GeoDataLinearRing::contains( const GeoDataCoordinates &coordinates ) const
{
    // Quick bounding box check
    if ( !latLonAltBox().contains( coordinates ) ) {
        return false;
    }

    int const points = size();
    bool inside = false; // also true for points = 0
    int j = points - 1;

    for ( int i=0; i<points; ++i )
    {
        double lon1;
        double lat1;

        getLonLat(i, lon1, lat1, GeoDataCoordinates::Radian);

        double lon2;
        double lat2;

        getLonLat(j, lon2, lat2, GeoDataCoordinates::Radian);

        if ( ( lon1 < coordinates.longitude() && lon2 >= coordinates.longitude() ) ||
             ( lon2 < coordinates.longitude() && lon1 >= coordinates.longitude() ) )
        {
            if ( lat1 + ( coordinates.longitude() - lon1) / ( lon2 - lon1) * ( lat2-lat1 ) < coordinates.latitude() )
            {
                inside = !inside;
            }
        }

        j = i;
    }

    return inside;
}

bool GeoDataLinearRing::isClockwise() const
{
    int n = size();
    qreal area = 0;
    for ( int i = 1; i < n - 1; ++i )
    {
        double lon1;
        double lat1;

        getLonLat(i-1, lon1, lat1, GeoDataCoordinates::Radian);

        double lon2;
        double lat2;

        getLonLat(i, lon2, lat2, GeoDataCoordinates::Radian);


        area += ( lon2 - lon1 ) * ( lat2 + lat1 );
    }

    double lon1;
    double lat1;

    getLonLat(0, lon1, lat1, GeoDataCoordinates::Radian);

    double lon2;
    double lat2;

    getLonLat(n-2, lon2, lat2, GeoDataCoordinates::Radian);

    area += ( lon1 - lon2 ) * ( lat1 + lat2 );

    return area > 0;
}

}
