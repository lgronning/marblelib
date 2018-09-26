#define QT_NO_DEBUG_OUTPUT
//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Andrew Manson <g.real.ate@gmail.com>
//

#include "GeoMultiGraphicsItem.h"

#include "GeoLineStringGraphicsItem.h"
#include "GeoPointGraphicsItem.h"
#include "GeoPolygonGraphicsItem.h"
#include "GeoMultiLineStringGraphicsItem.h"
#include "GeoMultiPolygonGraphicsItem.h"
#include "GeoMultiPointGraphicsItem.h"


#include "geodata/data/GeoDataFeature.h"
#include "geodata/data/GeoDataMultiGeometry.h"
#include "geodata/data/GeoDataLineString.h"
#include "geodata/data/GeoDataMultiLineString.h"
#include "geodata/data/GeoDataPolygon.h"
#include "geodata/data/GeoDataMultiPolygon.h"
#include "geodata/data/GeoDataLineStyle.h"
#include "geodata/data/GeoDataPolyStyle.h"
#include "geodata/data/GeoDataLabelStyle.h"
#include "geodata/parser/GeoDataTypes.h"
#include "GeoPainter.h"
#include "ViewportParams.h"
#include "geodata/data/GeoDataStyle.h"
#include <QtCore/QDebug>

namespace Marble
{

GeoMultiGraphicsItem::GeoMultiGraphicsItem( const GeoDataFeature *feature, const GeoDataMultiGeometry* multiGeometry )
        : GeoGraphicsItem( feature ),
          m_multiGeometry( multiGeometry )
{
}



void GeoMultiGraphicsItem::setMultiGeometry( const GeoDataMultiGeometry* multiGeometry )
{
    m_multiGeometry = multiGeometry;
}

const GeoDataLatLonAltBox&
GeoMultiGraphicsItem::latLonAltBox() const
{
    return m_multiGeometry->latLonAltBox();
}

void GeoMultiGraphicsItem::renderGeometry(GeoPainter *painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style)
{
    QVector<GeoDataGeometry*>::const_iterator it = m_multiGeometry->constBegin();
    QVector<GeoDataGeometry*>::const_iterator end = m_multiGeometry->constEnd();

    for (; it != end; ++it)
    {
        if ( (*it)->latLonAltBox().isEmpty() || !viewport->resolves( (*it)->latLonAltBox(), 0.1) )
        {
            continue;
        }

        QSharedPointer<GeoGraphicsItem> item;
        if ( (*it)->nodeType() == GeoDataTypes::GeoDataLineStringType )
        {
            const GeoDataLineString* line = static_cast<const GeoDataLineString*>( (*it) );
            item = QSharedPointer<GeoGraphicsItem>(new GeoLineStringGraphicsItem( feature(), line ));
        }
        else if ( (*it)->nodeType() == GeoDataTypes::GeoDataMultiLineStringType )
        {
            const GeoDataMultiLineString* line = static_cast<const GeoDataMultiLineString*>( (*it) );
            item = QSharedPointer<GeoGraphicsItem>(new GeoMultiLineStringGraphicsItem( feature(), line ));
        }
        else if ( (*it)->nodeType() == GeoDataTypes::GeoDataPointType )
        {
            const GeoDataPoint * point = static_cast<const GeoDataPoint*>( (*it) );
            item = QSharedPointer<GeoGraphicsItem>(new GeoPointGraphicsItem( feature(), point ));
        }
        else if ( (*it)->nodeType() == GeoDataTypes::GeoDataLinearRingType )
        {
            const GeoDataLinearRing *ring = static_cast<const GeoDataLinearRing*>( (*it) );
            item = QSharedPointer<GeoGraphicsItem>(new GeoPolygonGraphicsItem( feature(), ring ));
        }
        else if ( (*it)->nodeType() == GeoDataTypes::GeoDataPolygonType )
        {
            const GeoDataPolygon *poly = static_cast<const GeoDataPolygon*>( (*it) );
            item = QSharedPointer<GeoGraphicsItem>(new GeoPolygonGraphicsItem( feature(), poly ));
        }
        else if ( (*it)->nodeType() == GeoDataTypes::GeoDataMultiPolygonType )
        {
            const GeoDataMultiPolygon *poly = static_cast<const GeoDataMultiPolygon*>( (*it) );
            item = QSharedPointer<GeoGraphicsItem>(new GeoMultiPolygonGraphicsItem( feature(), poly ));
        }
        else if ( (*it)->nodeType() == GeoDataTypes::GeoDataMultiPointType)
        {
            const GeoDataMultiPoint *poly = static_cast<const GeoDataMultiPoint*>( (*it) );
            item = QSharedPointer<GeoGraphicsItem>(new GeoMultiPointGraphicsItem( feature(), poly ));
        }

        if(item)
        {
            item->renderGeometry(painter, viewport, style);
        }
    }
}

void GeoMultiGraphicsItem::renderLabels(GeoPainter *painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style, GeoLabelPlaceHandler &placeHandler)
{
    QVector<GeoDataGeometry*>::const_iterator it = m_multiGeometry->constBegin();
    QVector<GeoDataGeometry*>::const_iterator end = m_multiGeometry->constEnd();

    for (; it != end; ++it)
    {
        if ( (*it)->latLonAltBox().isEmpty() || !viewport->resolves( (*it)->latLonAltBox(), 0.1) )
        {
            continue;
        }

        QSharedPointer<GeoGraphicsItem> item;
        if ( (*it)->nodeType() == GeoDataTypes::GeoDataLineStringType )
        {
            const GeoDataLineString* line = static_cast<const GeoDataLineString*>( (*it) );
            item = QSharedPointer<GeoGraphicsItem>(new GeoLineStringGraphicsItem( feature(), line ));
        }
        else if ( (*it)->nodeType() == GeoDataTypes::GeoDataMultiLineStringType )
        {
            const GeoDataMultiLineString* line = static_cast<const GeoDataMultiLineString*>( (*it) );
            item = QSharedPointer<GeoGraphicsItem>(new GeoMultiLineStringGraphicsItem( feature(), line ));
        }
        else if ( (*it)->nodeType() == GeoDataTypes::GeoDataPointType )
        {
            const GeoDataPoint * point = static_cast<const GeoDataPoint*>( (*it) );
            item = QSharedPointer<GeoGraphicsItem>(new GeoPointGraphicsItem( feature(), point ));
        }
        else if ( (*it)->nodeType() == GeoDataTypes::GeoDataLinearRingType )
        {
            const GeoDataLinearRing *ring = static_cast<const GeoDataLinearRing*>( (*it) );
            item = QSharedPointer<GeoGraphicsItem>(new GeoPolygonGraphicsItem( feature(), ring ));
        }
        else if ( (*it)->nodeType() == GeoDataTypes::GeoDataPolygonType )
        {
            const GeoDataPolygon *poly = static_cast<const GeoDataPolygon*>( (*it) );
            item = QSharedPointer<GeoGraphicsItem>(new GeoPolygonGraphicsItem( feature(), poly ));
        }
        else if ( (*it)->nodeType() == GeoDataTypes::GeoDataMultiPolygonType )
        {
            const GeoDataMultiPolygon *poly = static_cast<const GeoDataMultiPolygon*>( (*it) );
            item = QSharedPointer<GeoGraphicsItem>(new GeoMultiPolygonGraphicsItem( feature(), poly ));
        }

        if(item)
        {
            GeoDataStyle tempStyle(*(style.data()));
            item->renderLabels(painter, viewport, GeoDataStyle::ConstPtr(new GeoDataStyle(tempStyle)), placeHandler);
        }
    }
}

void GeoMultiGraphicsItem::renderIcons(GeoPainter *painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style)
{
    QVector<GeoDataGeometry*>::const_iterator it = m_multiGeometry->constBegin();
    QVector<GeoDataGeometry*>::const_iterator end = m_multiGeometry->constEnd();

    for (; it != end; ++it)
    {
        if ( (*it)->latLonAltBox().isEmpty() || !viewport->resolves( (*it)->latLonAltBox(), 0.1) )
        {
            continue;
        }

        QSharedPointer<GeoGraphicsItem> item;
        if ( (*it)->nodeType() == GeoDataTypes::GeoDataLineStringType )
        {
            const GeoDataLineString* line = static_cast<const GeoDataLineString*>( (*it) );
            item = QSharedPointer<GeoGraphicsItem>(new GeoLineStringGraphicsItem( feature(), line ));
        }
        else if ( (*it)->nodeType() == GeoDataTypes::GeoDataMultiLineStringType )
        {
            const GeoDataMultiLineString* line = static_cast<const GeoDataMultiLineString*>( (*it) );
            item = QSharedPointer<GeoGraphicsItem>(new GeoMultiLineStringGraphicsItem( feature(), line ));
        }
        else if ( (*it)->nodeType() == GeoDataTypes::GeoDataPointType )
        {
            const GeoDataPoint * point = static_cast<const GeoDataPoint*>( (*it) );
            item = QSharedPointer<GeoGraphicsItem>(new GeoPointGraphicsItem( feature(), point ));
        }
        else if ( (*it)->nodeType() == GeoDataTypes::GeoDataLinearRingType )
        {
            const GeoDataLinearRing *ring = static_cast<const GeoDataLinearRing*>( (*it) );
            item = QSharedPointer<GeoGraphicsItem>(new GeoPolygonGraphicsItem( feature(), ring ));
        }
        else if ( (*it)->nodeType() == GeoDataTypes::GeoDataPolygonType )
        {
            const GeoDataPolygon *poly = static_cast<const GeoDataPolygon*>( (*it) );
            item = QSharedPointer<GeoGraphicsItem>(new GeoPolygonGraphicsItem( feature(), poly ));
        }
        else if ( (*it)->nodeType() == GeoDataTypes::GeoDataMultiPolygonType )
        {
            const GeoDataMultiPolygon *poly = static_cast<const GeoDataMultiPolygon*>( (*it) );
            item = QSharedPointer<GeoGraphicsItem>(new GeoMultiPolygonGraphicsItem( feature(), poly ));
        }

        if(item)
        {
            GeoDataStyle tempStyle(*(style.data()));

            item->renderIcons(painter, viewport, GeoDataStyle::ConstPtr(new GeoDataStyle(tempStyle)));
        }
    }
}

}
