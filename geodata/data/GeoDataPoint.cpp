//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2004-2007 Torsten Rahn <tackat@kde.org>
// Copyright 2007      Inge Wallin  <ingwa@kde.org>
// Copyright 2008      Patrick Spendrin <ps_ml@gmx.de>
//


#include "geodata/data/GeoDataPoint.h"
#include "geodata/data/GeoDataPoint_p.h"
#include "geodata/data/GeoDataCoordinates.h"

#include <cmath>

#include <QCoreApplication>

#include "MarbleDebug.h"
#include "MarbleGlobal.h"

#include "geodata/parser/GeoDataTypes.h"
#include "geodata/data/GeoDataLatLonAltBox.h"


namespace Marble
{

GeoDataPoint::GeoDataPoint( qreal lon, qreal lat, qreal alt,
                            GeoDataCoordinates::Unit unit )
    : GeoDataGeometry( new GeoDataPointPrivate )
{
    p()->m_coordinates = GeoDataCoordinates( lon, lat, alt, unit );
    p()->m_latLonAltBox = GeoDataLatLonAltBox( p()->m_coordinates );
}

GeoDataPoint::GeoDataPoint( const GeoDataPoint& other )
    : GeoDataGeometry( other )
    
{
    p()->m_coordinates = other.p()->m_coordinates;
    p()->m_latLonAltBox = other.p()->m_latLonAltBox;
}

GeoDataPoint::GeoDataPoint( const GeoDataCoordinates& other )
    : GeoDataGeometry ( new GeoDataPointPrivate )
{
    p()->m_coordinates = other;
    p()->m_latLonAltBox = GeoDataLatLonAltBox( p()->m_coordinates );
}

GeoDataPoint::GeoDataPoint()
    : GeoDataGeometry( new GeoDataPointPrivate )
{
    // nothing to do
}

GeoDataPoint::~GeoDataPoint()
{
    // nothing to do
}

bool GeoDataPoint::operator==( const GeoDataPoint &other ) const
{
    return equals(other) &&
           coordinates() == other.coordinates();
}

bool GeoDataPoint::operator!=( const GeoDataPoint &other ) const
{
    return !this->operator==(other);
}

void GeoDataPoint::setCoordinates( const GeoDataCoordinates &coordinates )
{
    detach();
    p()->m_coordinates = coordinates;
    p()->m_latLonAltBox = GeoDataLatLonAltBox( p()->m_coordinates );
}

const GeoDataCoordinates &GeoDataPoint::coordinates() const
{
    return p()->m_coordinates;
}

GeoDataPointPrivate* GeoDataPoint::p()
{
    return static_cast<GeoDataPointPrivate*>(GeoDataGeometry::d);
}

const GeoDataPointPrivate* GeoDataPoint::p() const
{
    return static_cast<GeoDataPointPrivate*>(GeoDataGeometry::d);
}

const char* GeoDataPoint::nodeType() const
{
    return GeoDataTypes::GeoDataPointType;
}

void GeoDataPoint::detach()
{
    GeoDataGeometry::detach();
}

}
