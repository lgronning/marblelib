#define QT_NO_DEBUG_OUTPUT
//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008      Patrick Spendrin <ps_ml@gmx.de>
//


#include "geodata/data/GeoDataMultiGeometry.h"
#include "geodata/data/GeoDataMultiGeometry_p.h"

#include "geodata/data/GeoDataLineString.h"
#include "geodata/data/GeoDataLinearRing.h"
#include "geodata/data/GeoDataPoint.h"
#include "geodata/data/GeoDataPolygon.h"

#include "MarbleDebug.h"

#include <QDataStream>


namespace Marble
{

GeoDataMultiGeometry::GeoDataMultiGeometry()
    : GeoDataGeometry( new GeoDataMultiGeometryPrivate )
{
}

GeoDataMultiGeometry::GeoDataMultiGeometry( const GeoDataGeometry& other )
    : GeoDataGeometry( other )
{
}

GeoDataMultiGeometry::~GeoDataMultiGeometry()
= default;

GeoDataMultiGeometryPrivate* GeoDataMultiGeometry::p()
{
    return static_cast<GeoDataMultiGeometryPrivate*>(d);
}

const GeoDataMultiGeometryPrivate* GeoDataMultiGeometry::p() const
{
    return static_cast<GeoDataMultiGeometryPrivate*>(d);
}

const GeoDataLatLonAltBox& GeoDataMultiGeometry::latLonAltBox() const
{
    if(p()->m_dirtyBox)
    {
        QVector<GeoDataGeometry*>::const_iterator it = p()->m_vector.constBegin();
        QVector<GeoDataGeometry*>::const_iterator end = p()->m_vector.constEnd();

        p()->m_latLonAltBox.clear();
        for (; it != end; ++it) {
            if ( !(*it)->latLonAltBox().isEmpty() ) {
                if ( p()->m_latLonAltBox.isEmpty() ) {
                    p()->m_latLonAltBox = (*it)->latLonAltBox();
                }
                else {
                    p()->m_latLonAltBox |= (*it)->latLonAltBox();
                }
            }
        }
        p()->m_dirtyBox = false;
    }
    return p()->m_latLonAltBox;
}

int GeoDataMultiGeometry::size() const
{
    return p()->m_vector.size();
}

QVector<GeoDataGeometry> GeoDataMultiGeometry::vector() const
{
    QVector<GeoDataGeometry> results;

    QVector<GeoDataGeometry*>::const_iterator it = p()->m_vector.constBegin();
    QVector<GeoDataGeometry*>::const_iterator end = p()->m_vector.constEnd();

    for (; it != end; ++it) {
            GeoDataGeometry f = **it;
            results.append( f );
    }

    return results;
}

GeoDataGeometry& GeoDataMultiGeometry::at( int pos )
{
    mDebug() << "detaching!";
    detach();
    return *(p()->m_vector[ pos ]);
}

const GeoDataGeometry& GeoDataMultiGeometry::at( int pos ) const
{
    return *(p()->m_vector.at( pos ));
}

GeoDataGeometry& GeoDataMultiGeometry::operator[]( int pos )
{
    detach();
    return *(p()->m_vector[ pos ]);
}

const GeoDataGeometry& GeoDataMultiGeometry::operator[]( int pos ) const
{
    return *(p()->m_vector[ pos ]);
}

GeoDataGeometry& GeoDataMultiGeometry::last()
{
    detach();
    return *(p()->m_vector.last());
}

GeoDataGeometry& GeoDataMultiGeometry::first()
{
    detach();
    return *(p()->m_vector.first());
}

const GeoDataGeometry& GeoDataMultiGeometry::last() const
{
    return *(p()->m_vector.last());
}

const GeoDataGeometry& GeoDataMultiGeometry::first() const
{
    return *(p()->m_vector.first());
}

QVector<GeoDataGeometry*>::Iterator GeoDataMultiGeometry::begin()
{
    detach();
    return p()->m_vector.begin();
}

QVector<GeoDataGeometry*>::Iterator GeoDataMultiGeometry::end()
{
    detach();
    return p()->m_vector.end();
}

QVector<GeoDataGeometry*>::ConstIterator GeoDataMultiGeometry::constBegin() const
{
    return p()->m_vector.constBegin();
}

QVector<GeoDataGeometry*>::ConstIterator GeoDataMultiGeometry::constEnd() const
{
    return p()->m_vector.constEnd();
}

/**
 * @brief  returns the requested child item
 */
GeoDataGeometry* GeoDataMultiGeometry::child( int i )
{
    detach();
    return p()->m_vector.at( i );
}

const GeoDataGeometry* GeoDataMultiGeometry::child( int i ) const
{
    return p()->m_vector.at( i );
}

/**
 * @brief returns the position of an item in the list
 */
int GeoDataMultiGeometry::childPosition( const GeoDataGeometry *object ) const
{
    return p()->m_childPosition.value(object, -1);
}

/**
* @brief add an element
*/
void GeoDataMultiGeometry::append( GeoDataGeometry *other )
{
    detach();
    other->setParent( this );
    p()->m_childPosition.insert(other, p()->m_vector.size());
    p()->m_vector.append( other );
    p()->m_dirtyBox = true;
}


GeoDataMultiGeometry& GeoDataMultiGeometry::operator << ( const GeoDataGeometry& value )
{
    detach();
    GeoDataGeometry *g = new GeoDataGeometry( value );
    g->setParent( this );
    p()->m_vector.append( g );
    p()->m_dirtyBox = true;

    return *this;
}

void GeoDataMultiGeometry::clear()
{
    detach();
    qDeleteAll(p()->m_vector);
    p()->m_childPosition.clear();
    p()->m_vector.clear();
    p()->m_dirtyBox = true;
}

}
