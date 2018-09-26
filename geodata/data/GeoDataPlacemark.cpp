//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2004-2007 Torsten Rahn <tackat@kde.org>
// Copyright 2007      Inge Wallin  <ingwa@kde.org>
// Copyright 2008-2009      Patrick Spendrin <ps_ml@gmx.de>
//


// Own
#include "geodata/data/GeoDataPlacemark.h"

// Private
#include "geodata/data/GeoDataPlacemark_p.h"

#include "geodata/data/GeoDataMultiGeometry.h"
#include "geodata/data/GeoDataCoordinates.h"

// Qt
#include <QDataStream>
#include "MarbleDebug.h"
#include "geodata/data/GeoDataModel.h"
#include <QString>

namespace Marble
{
GeoDataPlacemark::GeoDataPlacemark()
    : GeoDataFeature( new GeoDataPlacemarkPrivate )
{
    p()->m_geometry->setParent( this );
}

GeoDataPlacemark::GeoDataPlacemark( const GeoDataPlacemark& other )
    : GeoDataFeature( other )
{
    // FIXME: temporary (until detach() is called) violates following invariant
    // which could lead to crashes
//    Q_ASSERT( this == p()->m_geometry->parent() );

    // FIXME: fails as well when "other" is a copy where detach wasn't called
//    Q_ASSERT( other.p()->m_geometry == 0 || &other == other.p()->m_geometry->parent() );
}

GeoDataPlacemark::GeoDataPlacemark( const QString& name )
    : GeoDataFeature( new GeoDataPlacemarkPrivate )
{
    d->m_name = name;
    p()->m_geometry->setParent( this );
}

GeoDataPlacemark::~GeoDataPlacemark()
{
    // nothing to do
}

GeoDataPlacemark &GeoDataPlacemark::operator=( const GeoDataPlacemark &other )
= default;

bool GeoDataPlacemark::operator==( const GeoDataPlacemark& other ) const
{ 
    if ( !equals(other) ||
         p()->m_countrycode != other.p()->m_countrycode ||
         p()->m_area != other.p()->m_area ||
         p()->m_population != other.p()->m_population ||
         p()->m_state != other.p()->m_state ) {
        return false;
    }

    if ( !p()->m_geometry && !other.p()->m_geometry ) {
        return true;
    } else if ( (!p()->m_geometry && other.p()->m_geometry) ||
                (p()->m_geometry && !other.p()->m_geometry) ) {
        return false;
    }

    if ( p()->m_geometry->nodeType() != other.p()->m_geometry->nodeType() ) {
        return false;
    }

    if ( p()->m_geometry->nodeType() == GeoDataTypes::GeoDataPolygonType ) {
        GeoDataPolygon *thisPoly = dynamic_cast<GeoDataPolygon*>( p()->m_geometry );
        GeoDataPolygon *otherPoly = dynamic_cast<GeoDataPolygon*>( other.p()->m_geometry );
        Q_ASSERT( thisPoly && otherPoly );

        if ( *thisPoly != *otherPoly ) {
            return false;
        }
    } else if ( p()->m_geometry->nodeType() == GeoDataTypes::GeoDataLineStringType ) {
        GeoDataLineString *thisLine = dynamic_cast<GeoDataLineString*>( p()->m_geometry );
        GeoDataLineString *otherLine = dynamic_cast<GeoDataLineString*>( other.p()->m_geometry );
        Q_ASSERT( thisLine && otherLine );

        if ( *thisLine != *otherLine ) {
            return false;
        }
    } else if ( p()->m_geometry->nodeType() == GeoDataTypes::GeoDataModelType ) {
        GeoDataModel *thisModel = dynamic_cast<GeoDataModel*>( p()->m_geometry );
        GeoDataModel *otherModel = dynamic_cast<GeoDataModel*>( other.p()->m_geometry );
        Q_ASSERT( thisModel && otherModel );

        if ( *thisModel != *otherModel ) {
            return false;
        }
    /*} else if ( p()->m_geometry->nodeType() == GeoDataTypes::GeoDataMultiGeometryType ) {
        GeoDataMultiGeometry *thisMG = dynamic_cast<GeoDataMultiGeometry*>( p()->m_geometry );
        GeoDataMultiGeometry *otherMG = dynamic_cast<GeoDataMultiGeometry*>( other.p()->m_geometry );
        Q_ASSERT( thisMG && otherMG );

        if ( *thisMG != *otherMG ) {
            return false;
        } */ // Does not have equality operators. I guess they need to be implemented soon.
    } else if ( p()->m_geometry->nodeType() == GeoDataTypes::GeoDataPointType ) {
        GeoDataPoint *thisPoint = dynamic_cast<GeoDataPoint*>( p()->m_geometry );
        GeoDataPoint *otherPoint = dynamic_cast<GeoDataPoint*>( other.p()->m_geometry );
        Q_ASSERT( thisPoint && otherPoint );

        if ( *thisPoint != *otherPoint ) {
            return false;
        }
    }

    return true;
}

bool GeoDataPlacemark::operator!=( const GeoDataPlacemark& other ) const
{
    return !this->operator==( other );
}

GeoDataPlacemarkPrivate* GeoDataPlacemark::p()
{
    return static_cast<GeoDataPlacemarkPrivate*>(d);
}

const GeoDataPlacemarkPrivate* GeoDataPlacemark::p() const
{
    return static_cast<GeoDataPlacemarkPrivate*>(d);
}

GeoDataGeometry*
GeoDataPlacemark::geometry()
{
    detach();
    p()->m_geometry->setParent( this );
    return p()->m_geometry;
}

const GeoDataGeometry*
GeoDataPlacemark::geometry() const
{
    return p()->m_geometry;
}

bool
GeoDataPlacemark::placemarkLayoutOrderCompare(const GeoDataPlacemark *left, const GeoDataPlacemark *right)
{
    if (left->d->m_zoomLevel != right->d->m_zoomLevel) {
        return (left->d->m_zoomLevel < right->d->m_zoomLevel); // lower zoom level comes first
    }

    if (left->d->m_popularity != right->d->m_popularity) {
        return left->d->m_popularity > right->d->m_popularity; // higher popularity comes first
    }

    return left < right; // lower pointer value comes first
}

GeoDataCoordinates GeoDataPlacemark::coordinate(const QDateTime &dateTime) const
{
    GeoDataCoordinates coord;
 
    if( p()->m_geometry ) {
        // Beware: comparison between pointers, not strings.
        if ( p()->m_geometry->nodeType() == GeoDataTypes::GeoDataPointType )
        {
            coord = static_cast<const GeoDataPoint *>( p()->m_geometry )->coordinates();
        }
        else
        {
            coord = p()->m_geometry->latLonAltBox().center();
        }
    }
    return coord;
}

void GeoDataPlacemark::coordinate( qreal& lon, qreal& lat, qreal& alt ) const
{
    coordinate().geoCoordinates( lon, lat, alt );
}

void GeoDataPlacemark::setCoordinate( qreal lon, qreal lat, qreal alt, GeoDataCoordinates::Unit _unit)
{
    setGeometry( new GeoDataPoint(lon, lat, alt, _unit ) );
}

void GeoDataPlacemark::setCoordinate( const GeoDataCoordinates &point )
{
    setGeometry ( new GeoDataPoint( point ) );
}

void GeoDataPlacemark::setCoordinate( const GeoDataPoint &point )
{
    setGeometry ( new GeoDataPoint( point ) );
}

void GeoDataPlacemark::setGeometry( GeoDataGeometry *entry )
{
    detach();
    delete p()->m_geometry;
    p()->m_geometry = entry;
    p()->m_geometry->setParent( this );
}


QString GeoDataPlacemark::displayName() const
{
    return name();
}



qreal GeoDataPlacemark::area() const
{
    return p()->m_area;
}

void GeoDataPlacemark::setArea( qreal area )
{
    detach();
    p()->m_geometry->setParent( this );
    p()->m_area = area;
}

qint64 GeoDataPlacemark::population() const
{
    return p()->m_population;
}

void GeoDataPlacemark::setPopulation( qint64 population )
{
    detach();
    p()->m_geometry->setParent( this );
    p()->m_population = population;
}

const QString GeoDataPlacemark::state() const
{
    return p()->m_state;
}

void GeoDataPlacemark::setState( const QString &state )
{
    detach();
    p()->m_geometry->setParent( this );
    p()->m_state = state;
}

const QString GeoDataPlacemark::countryCode() const
{
    return p()->m_countrycode;
}

void GeoDataPlacemark::setCountryCode( const QString &countrycode )
{
    detach();
    p()->m_geometry->setParent( this );
    p()->m_countrycode = countrycode;
}

bool GeoDataPlacemark::isBalloonVisible() const
{
    return p()->m_isBalloonVisible;
}

void GeoDataPlacemark::setBalloonVisible( bool visible )
{
    detach();
    p()->m_geometry->setParent( this );
    p()->m_isBalloonVisible = visible;
}

}
