//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Patrick Spendrin <ps_ml@gmx.de>
//

#ifndef MARBLE_GEODATALINEARRINGPRIVATE_H
#define MARBLE_GEODATALINEARRINGPRIVATE_H

#include "geodata/data/GeoDataLineString_p.h"

#include "geodata/parser/GeoDataTypes.h"

namespace Marble
{

class GeoDataLinearRingCoordinatesPrivate : public GeoDataLineStringCoordinatesPrivate
{
  public:
    explicit GeoDataLinearRingCoordinatesPrivate(const QVector<GeoDataCoordinates> &points, TessellationFlags f, const QVector<double> &messure, const QVector<double> &messureInfo )
        :   GeoDataLineStringCoordinatesPrivate(points, f, messure, messureInfo)
    {
    }

    explicit GeoDataLinearRingCoordinatesPrivate()
        :  GeoDataLineStringCoordinatesPrivate()
    {
    }

    GeoDataGeometryPrivate* copy() override
    {
        GeoDataLinearRingCoordinatesPrivate* copy = new GeoDataLinearRingCoordinatesPrivate;
        *copy = *this;
        return copy;
    }

    const char* nodeType() const override
    {
        return GeoDataTypes::GeoDataLinearRingType;
    }

    EnumGeometryId geometryId() const override
    {
        return GeoDataLinearRingId;
    }
};


class GeoDataLinearRingPointsPrivate : public GeoDataLineStringPointsPrivate
{
  public:
    explicit GeoDataLinearRingPointsPrivate( const QVector<QPointF> &points, TessellationFlags f, const QVector<double> &messure, const QVector<double> &messureInfo )
        :   GeoDataLineStringPointsPrivate(points, f, messure, messureInfo)
    {
    }

    explicit GeoDataLinearRingPointsPrivate()
        :  GeoDataLineStringPointsPrivate()
    {
    }

    GeoDataGeometryPrivate* copy() override
    {
        GeoDataLinearRingPointsPrivate* copy = new GeoDataLinearRingPointsPrivate;
        *copy = *this;
        return copy;
    }

    const char* nodeType() const override
    {
        return GeoDataTypes::GeoDataLinearRingType;
    }

    EnumGeometryId geometryId() const override
    {
        return GeoDataLinearRingId;
    }

    QVector<QPointF> m_points;
};


class GeoDataLinearRingPoints3DPrivate : public GeoDataLineStringPoints3DPrivate
{
  public:
    explicit GeoDataLinearRingPoints3DPrivate( const QVector<QwtPoint3D> &points, TessellationFlags f, const QVector<double> &messure, const QVector<double> &messureInfo )
        :   GeoDataLineStringPoints3DPrivate(points, f, messure, messureInfo)
    {
    }

    explicit GeoDataLinearRingPoints3DPrivate()
        :  GeoDataLineStringPoints3DPrivate()
    {
    }

    GeoDataGeometryPrivate* copy() override
    {
        GeoDataLinearRingPoints3DPrivate* copy = new GeoDataLinearRingPoints3DPrivate;
        *copy = *this;
        return copy;
    }

    const char* nodeType() const override
    {
        return GeoDataTypes::GeoDataLinearRingType;
    }

    EnumGeometryId geometryId() const override
    {
        return GeoDataLinearRingId;
    }
};


} // namespace Marble

#endif
