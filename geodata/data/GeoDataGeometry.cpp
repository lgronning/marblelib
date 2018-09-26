//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008      Torsten Rahn <rahn@kde.org>
// Copyright 2008-2009      Patrick Spendrin <ps_ml@gmx.de>
// Copyright 2008      Inge Wallin <inge@lysator.liu.se>
//


#include "geodata/data/GeoDataGeometry.h"
#include "geodata/data/GeoDataGeometry_p.h"

#include "geodata/data/GeoDataPoint.h"
#include "geodata/data/GeoDataPolygon.h"
#include "geodata/data/GeoDataLineString.h"
#include "geodata/data/GeoDataMultiGeometry.h"

#include "MarbleDebug.h"

#include <QDataStream>


namespace Marble
{

GeoDataGeometry::GeoDataGeometry()
    : d( new GeoDataGeometryPrivate() )
{
    d->ref.ref();
}

GeoDataGeometry::GeoDataGeometry( const GeoDataGeometry& other )
    : GeoDataObject(),
      d( other.d )
{
    d->ref.ref();
}

GeoDataGeometry::GeoDataGeometry( GeoDataGeometryPrivate* priv )
    : GeoDataObject(),
      d( priv )
{
    d->ref.ref();
}

GeoDataGeometry::~GeoDataGeometry()
{
    if (!d->ref.deref())
        delete d;
}

void GeoDataGeometry::detach()
{
    if(d->ref.load() == 1) {
        return;
    }

     GeoDataGeometryPrivate* new_d = d->copy();

    if (!d->ref.deref())
        delete d;

    d = new_d;
    d->ref.ref();
}

const char* GeoDataGeometry::nodeType() const
{
    return d->nodeType();
}

EnumGeometryId GeoDataGeometry::geometryId() const
{
    return d->geometryId();
}

GeoDataGeometry& GeoDataGeometry::operator=( const GeoDataGeometry& other )
{
    GeoDataObject::operator=( other );

    if (!d->ref.deref())
        delete d;

    d = other.d;
    d->ref.ref();
    
    return *this;
}

bool GeoDataGeometry::extrude() const
{
    return d->m_extrude;
}

void GeoDataGeometry::setExtrude( bool extrude )
{
    detach();
    d->m_extrude = extrude;
}

AltitudeMode GeoDataGeometry::altitudeMode() const
{
    return d->m_altitudeMode;
}

void GeoDataGeometry::setAltitudeMode( const AltitudeMode altitudeMode )
{
    detach();
    d->m_altitudeMode = altitudeMode;
}

const GeoDataLatLonAltBox& GeoDataGeometry::latLonAltBox() const
{
    return d->m_latLonAltBox;
}

bool GeoDataGeometry::equals(const GeoDataGeometry &other) const
{
    return GeoDataObject::equals(other) &&
           d->m_extrude == other.d->m_extrude &&
           d->m_altitudeMode == other.d->m_altitudeMode;
}

}
