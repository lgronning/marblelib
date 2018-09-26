#ifndef GEODATAMULTIPOINT_P_H
#define GEODATAMULTIPOINT_P_H

#include "geodata/data/GeoDataGeometry_p.h"

#include "geodata/parser/GeoDataTypes.h"
#include <QtCore/QHash>
#include <QtCore/QPointF>
#include <qwt_point_3d.h>

namespace Marble
{

class GeoDataMultiPointPrivate : public GeoDataGeometryPrivate
{
  public:
    enum Type
    {
        Coordinates,
        Points,
        Points3d,
    };

    explicit GeoDataMultiPointPrivate()
        :  m_dirtyBox( true )
    {
    }

    ~GeoDataMultiPointPrivate() override
    {
    }

    GeoDataMultiPointPrivate& operator=( const GeoDataMultiPointPrivate &other)
    {
        GeoDataGeometryPrivate::operator=( other );
        m_dirtyBox = other.m_dirtyBox;
        return *this;
    }


    GeoDataGeometryPrivate* copy() override = 0;

    const char* nodeType() const override
    {
        return GeoDataTypes::GeoDataMultiPointType;
    }

    EnumGeometryId geometryId() const override
    {
        return GeoDataPointId;
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

    mutable bool m_dirtyBox; // tells whether there have been changes to the
                                            // GeoDataPoints since the LatLonAltBox has
                                            // been calculated. Saves performance.
};

class GeoDataMultiPointCoordinatesPrivate : public GeoDataMultiPointPrivate
{
  public:
    explicit GeoDataMultiPointCoordinatesPrivate(const QVector<GeoDataCoordinates> &points)
        :   GeoDataMultiPointPrivate(),
            m_points(points)
    {
    }

    explicit GeoDataMultiPointCoordinatesPrivate()
        :  GeoDataMultiPointPrivate()
    {
    }

    GeoDataGeometryPrivate* copy() override
    {
        GeoDataMultiPointCoordinatesPrivate* copy = new GeoDataMultiPointCoordinatesPrivate;
        *copy = *this;
        return copy;
    }

    GeoDataMultiPointCoordinatesPrivate& operator=( const GeoDataMultiPointCoordinatesPrivate &other)
    {
        GeoDataMultiPointPrivate::operator=( other );
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
        return GeoDataMultiPointPrivate::Coordinates;
    }


    QVector<GeoDataCoordinates> m_points;
};


class GeoDataMultiPointPointsPrivate : public GeoDataMultiPointPrivate
{
  public:
    explicit GeoDataMultiPointPointsPrivate( const QVector<QPointF> &points)
        :   GeoDataMultiPointPrivate(),
            m_points(points)
    {
    }

    explicit GeoDataMultiPointPointsPrivate()
        :  GeoDataMultiPointPrivate()
    {
    }

    GeoDataGeometryPrivate* copy() override
    {
        GeoDataMultiPointPointsPrivate* copy = new GeoDataMultiPointPointsPrivate;
        *copy = *this;
        return copy;
    }

    GeoDataMultiPointPointsPrivate& operator=( const GeoDataMultiPointPointsPrivate &other)
    {
        GeoDataMultiPointPrivate::operator=( other );
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
        QPointF point = m_points.at(pos);
        return GeoDataCoordinates(point.x(), point.y(), 0, GeoDataCoordinates::Degree);
    }


    
    Type
    type() const override
    {
        return GeoDataMultiPointPrivate::Points;
    }



    QVector<QPointF> m_points;
};


class GeoDataMultiPointPoints3DPrivate : public GeoDataMultiPointPrivate
{
  public:
    explicit GeoDataMultiPointPoints3DPrivate( const QVector<QwtPoint3D> &points)
        :   GeoDataMultiPointPrivate(),
            m_points(points)
    {
    }

    explicit GeoDataMultiPointPoints3DPrivate()
        :  GeoDataMultiPointPrivate()
    {
    }

    GeoDataGeometryPrivate* copy() override
    {
        GeoDataMultiPointPoints3DPrivate* copy = new GeoDataMultiPointPoints3DPrivate;
        *copy = *this;
        return copy;
    }

    GeoDataMultiPointPoints3DPrivate& operator=( const GeoDataMultiPointPoints3DPrivate &other)
    {
        GeoDataMultiPointPrivate::operator=( other );
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
        return GeoDataMultiPointPrivate::Points3d;
    }


    QVector<QwtPoint3D> m_points;
private:
};


} // namespace Marble

#endif // GEODATAMULTIPOINT_P_H
