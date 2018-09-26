//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Patrick Spendrin <ps_ml@gmx.de>
//

#ifndef MARBLE_GEODATALINESTRINGPRIVATE_H
#define MARBLE_GEODATALINESTRINGPRIVATE_H

#include "geodata/data/GeoDataGeometry_p.h"

#include "geodata/parser/GeoDataTypes.h"
#include <qwt_point_3d.h>
#include <QtCore/QHash>
#include <QtCore/QMutex>
#include <QtCore/QPointF>
namespace Marble
{

class GeoDataLineStringPrivate : public GeoDataGeometryPrivate
{
  public:
    enum Type
    {
        LineStringCoordinates,
        LineStringPoints,
        LineStringPoints3d,
    };

    explicit GeoDataLineStringPrivate(TessellationFlags f, const QVector<double> &messure, const QVector<double> &messureInfo)
        :  m_dirtyBox( true ),
           m_tessellationFlags( f ),
           m_messure(messure),
           m_messureInfo(messureInfo),
           m_previousResolution( -1 ),
           m_level( -1 )
    {
    }

    GeoDataLineStringPrivate()
         : m_dirtyBox( true ),
           m_previousResolution( -1 ),
           m_level( -1 )
    {
    }

    ~GeoDataLineStringPrivate() override
    {
    }

    GeoDataLineStringPrivate& operator=( const GeoDataLineStringPrivate &other)
    {
        GeoDataGeometryPrivate::operator=( other );
        m_dirtyBox = other.m_dirtyBox;
        m_tessellationFlags = other.m_tessellationFlags;
        m_details = other.m_details;
        m_previousResolution = other.m_previousResolution;
        m_level = other.m_level;
        m_messure = other.m_messure;
        m_messureInfo = other.m_messureInfo;
        return *this;
    }


    GeoDataGeometryPrivate* copy() override = 0;

    const char* nodeType() const override
    {
        return GeoDataTypes::GeoDataLineStringType;
    }

    EnumGeometryId geometryId() const override 
    {
        return GeoDataLineStringId;
    }

    virtual
    void
    getLonLat(int at, double &lon, double &lat, GeoDataCoordinates::Unit unit) const = 0;

    virtual
    bool
    isEmpty() const = 0;

    virtual
    int
    size() const = 0;

    virtual
    GeoDataCoordinates
    at( int pos ) const = 0;

    virtual
    Type
    type() const = 0;

    virtual
    bool
    hasMessure() const
    {
        return !m_messure.isEmpty();
    }

    double
    messure(int pos) const
    {
        return m_messure.value(pos, 0);
    }

    virtual
    bool
    hasMessureInfo() const
    {
        return !m_messureInfo.isEmpty();
    }

    double
    messureInfo(int pos) const
    {
        return m_messureInfo.value(pos, 0);
    }

    int levelForResolution(qreal resolution) const;
    qreal resolutionForLevel(int level) const;
    void optimize(GeoDataLineString& lineString) const;

    QHash<int, int> m_details;

    mutable bool m_dirtyBox; // tells whether there have been changes to the
                                            // GeoDataPoints since the LatLonAltBox has 
                                            // been calculated. Saves performance. 
    TessellationFlags m_tessellationFlags;
    QVector<double> m_messure;
    QVector<double> m_messureInfo;

    mutable qreal  m_previousResolution;
    mutable qreal  m_level;
};

class GeoDataLineStringCoordinatesPrivate : public GeoDataLineStringPrivate
{
  public:
    explicit GeoDataLineStringCoordinatesPrivate(const QVector<GeoDataCoordinates> &points, TessellationFlags f, const QVector<double> &messure, const QVector<double> &messureInfo  )
        :   GeoDataLineStringPrivate(f, messure, messureInfo),
            m_points(points)
    {
    }

    explicit GeoDataLineStringCoordinatesPrivate()
        :  GeoDataLineStringPrivate()
    {
    }

    GeoDataGeometryPrivate* copy() override
    {
        GeoDataLineStringCoordinatesPrivate* copy = new GeoDataLineStringCoordinatesPrivate;
        *copy = *this;
        return copy;
    }

    GeoDataLineStringCoordinatesPrivate& operator=( const GeoDataLineStringCoordinatesPrivate &other)
    {
        GeoDataLineStringPrivate::operator=( other );
        m_points = other.m_points;
        return *this;
    }


    
    void
    getLonLat(int at, double &lon, double &lat, GeoDataCoordinates::Unit unit) const override;

    
    bool
    isEmpty() const override
    {
        return m_points.isEmpty();
    }

    
    int
    size() const override
    {
        return m_points.size();
    }


    
    GeoDataCoordinates
    at( int pos ) const override
    {
        return m_points.at(pos);
    }

    
    Type
    type() const override
    {
        return GeoDataLineStringPrivate::LineStringCoordinates;
    }


    QVector<GeoDataCoordinates> m_points;
};


class GeoDataLineStringPointsPrivate : public GeoDataLineStringPrivate
{
  public:
    explicit GeoDataLineStringPointsPrivate( const QVector<QPointF> &points, TessellationFlags f, const QVector<double> &messure, const QVector<double> &messureInfo )
        :   GeoDataLineStringPrivate(f, messure, messureInfo),
            m_points(points)
    {
    }

    explicit GeoDataLineStringPointsPrivate()
        :  GeoDataLineStringPrivate()
    {
    }

    GeoDataGeometryPrivate* copy() override
    {
        GeoDataLineStringPointsPrivate* copy = new GeoDataLineStringPointsPrivate;
        *copy = *this;
        return copy;
    }

    GeoDataLineStringPointsPrivate& operator=( const GeoDataLineStringPointsPrivate &other)
    {
        GeoDataLineStringPrivate::operator=( other );
        m_points = other.m_points;
        return *this;
    }


    
    void
    getLonLat(int at, double &lon, double &lat, GeoDataCoordinates::Unit unit) const override;

    
    bool
    isEmpty() const override
    {
        return m_points.isEmpty();
    }

    
    int
    size() const override
    {
        return m_points.size();
    }

    
    GeoDataCoordinates
    at( int pos ) const override
    {
        QPointF point = m_points.constData()[pos];
        return GeoDataCoordinates(point.x(), point.y(), 0, GeoDataCoordinates::Degree);
    }


    
    Type
    type() const override
    {
        return GeoDataLineStringPrivate::LineStringPoints;
    }



    QVector<QPointF> m_points;
};


class GeoDataLineStringPoints3DPrivate : public GeoDataLineStringPrivate
{
  public:
    explicit GeoDataLineStringPoints3DPrivate( const QVector<QwtPoint3D> &points, TessellationFlags f, const QVector<double> &messure, const QVector<double> &messureInfo )
        :   GeoDataLineStringPrivate(f, messure, messureInfo),
            m_points(points)
    {
    }

    explicit GeoDataLineStringPoints3DPrivate()
        :  GeoDataLineStringPrivate()
    {
    }

    GeoDataGeometryPrivate* copy() override
    {
        GeoDataLineStringPoints3DPrivate* copy = new GeoDataLineStringPoints3DPrivate;
        *copy = *this;
        return copy;
    }

    GeoDataLineStringPoints3DPrivate& operator=( const GeoDataLineStringPoints3DPrivate &other)
    {
        GeoDataLineStringPrivate::operator=( other );
        m_points = other.m_points;
        return *this;
    }


    
    void
    getLonLat(int at, double &lon, double &lat, GeoDataCoordinates::Unit unit) const override;

    
    bool
    isEmpty() const override
    {
        return m_points.isEmpty();
    }

    
    int
    size() const override
    {
        return m_points.size();
    }

    
    GeoDataCoordinates
    at( int pos ) const override
    {
        QwtPoint3D point= m_points.at(pos);
        return GeoDataCoordinates(point.x(), point.y(), point.z(), GeoDataCoordinates::Degree);
    }

    
    Type
    type() const override
    {
        return GeoDataLineStringPrivate::LineStringPoints3d;
    }


    QVector<QwtPoint3D> m_points;
};


} // namespace Marble

#endif
