//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Bastian Holst <bastianholst@gmx.de>
//

// Self
#include "GeoGraphicsItem.h"
#include "GeoGraphicsItem_p.h"

#include "geodata/data/GeoDataPlacemark.h"

// Qt
#include "MarbleDebug.h"

#include <QtGui/QColor>

using namespace Marble;

GeoGraphicsItem::GeoGraphicsItem( const GeoDataFeature *feature )
    : d( new GeoGraphicsItemPrivate( feature ) )
{
}

GeoGraphicsItem::~GeoGraphicsItem()
{
    delete d;
}

bool GeoGraphicsItem::visible() const
{
    return feature()->isGloballyVisible();
}

GeoGraphicsItem::GeoGraphicsItemFlags GeoGraphicsItem::flags() const
{
    return d->m_flags;
}

void GeoGraphicsItem::setFlag( GeoGraphicsItemFlag flag, bool enabled )
{
    if( enabled ) {
        d->m_flags = d->m_flags | flag;
    } else {
        d->m_flags = d->m_flags & ~flag;
    }
}

void GeoGraphicsItem::setFlags( GeoGraphicsItemFlags flags )
{
    d->m_flags = flags;
}

const GeoDataFeature* GeoGraphicsItem::feature() const
{
    return d->m_feature;
}

const GeoDataLatLonAltBox& GeoGraphicsItem::latLonAltBox() const
{
    return d->m_latLonAltBox;
}

void GeoGraphicsItem::setLatLonAltBox( const GeoDataLatLonAltBox& latLonAltBox )
{
    d->m_latLonAltBox = latLonAltBox;
}

GeoDataStyle::ConstPtr
GeoGraphicsItem::style() const
{
    return feature()->style();
}

qreal GeoGraphicsItem::zLevel() const
{
    return d->m_zValue;
}

void GeoGraphicsItem::setZLevel( qreal z )
{
    d->m_zValue = z;
}

int GeoGraphicsItem::minZoomLevel() const
{
    return d->m_minZoomLevel;
}

void GeoGraphicsItem::setMinZoomLevel(int zoomLevel)
{
    d->m_minZoomLevel = zoomLevel;
}


