//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008-2009  Torsten Rahn <tackat@kde.org>
// Copyright 2009       Patrick Spendrin <ps_ml@gmx.de>
//


#ifndef MARBLE_GEODATALINESTRING_H
#define MARBLE_GEODATALINESTRING_H

#include <QFlags>
#include <QVector>
#include <QMetaType>

#include "MarbleGlobal.h"

#include "geodata/geodata_export.h"
#include "geodata/data/GeoDataGeometry.h"
#include "geodata/data/GeoDataCoordinates.h"
#include "geodata/data/GeoDataLatLonAltBox.h"
#include <qwt_point_3d.h>


namespace Marble
{

class GeoDataLineStringPrivate;

/*!
    \class GeoDataLineString
    \brief A LineString that allows to store a contiguous set of line segments.

    GeoDataLineString is a tool class that implements the LineString tag/class
    of the Open Geospatial Consortium standard KML 2.2.

    GeoDataLineString extends GeoDataGeometry to store and edit
    LineStrings.

    In the QPainter API "pure" LineStrings are also referred to as "polylines".
    As such they are similar to the outline of a non-closed QPolygon.

    Whenever a LineString is painted GeoDataLineStyle should be used to assign a
    color and line width.

    A GeoDataLineString consists of several (geodetic) nodes which are each
    connected through line segments. The nodes are stored as GeoDataCoordinates
    objects.

    The API which provides access to the nodes is similar to the API of
    QVector.

    GeoDataLineString allows LineStrings to be tessellated in order to make them
    follow the terrain and the curvature of the earth. The tessellation options
    allow for different ways of visualization:

    \li Not tessellated: A LineString that connects each two nodes directly and
        straight in screen coordinate space.
    \li A tessellated line: Each line segment is bent so that the LineString
        follows the curvature of the earth and its terrain. A tessellated
        line segment connects two nodes at the shortest possible distance
        ("along great circles").
    \li A tessellated line that follows latitude circles whenever possible:
        In this case Latitude circles are followed as soon as two subsequent
        nodes have exactly the same amount of latitude. In all other places the
        line segments follow great circles.

    Some convenience methods have been added that allow to calculate the
    geodesic bounding box or the length of a LineString.
*/

class GEODATA_EXPORT GeoDataLineString : public GeoDataGeometry
{

 public:
/*!
    \brief Creates a new LineString.
*/
    explicit GeoDataLineString( const QVector<GeoDataCoordinates> &points, TessellationFlags f = NoTessellation, const QVector<double> &messure = QVector<double>(), const QVector<double> &messureInfo = QVector<double>() );
    explicit GeoDataLineString( const QVector<QPointF> &points, TessellationFlags f = NoTessellation, const QVector<double> &messure = QVector<double>(), const QVector<double> &messureInfo = QVector<double>() );
    explicit GeoDataLineString( const QVector<QwtPoint3D> &points, TessellationFlags f = NoTessellation, const QVector<double> &messure = QVector<double>(), const QVector<double> &messureInfo = QVector<double>() );
    explicit GeoDataLineString();

/*!
    \brief Creates a LineString from an existing geometry object.
*/
    explicit GeoDataLineString( const GeoDataGeometry &other );


/*!
    \brief Destroys a LineString.
*/
    ~GeoDataLineString() override;

/*!
    \brief Returns whether a LineString is a closed polygon.

    \return <code>false</code> if the LineString is not a LinearRing.
*/
    virtual bool isClosed() const;


/*!
    \brief Returns whether the LineString follows the earth's surface.

    \return <code>true</code> if the LineString's line segments follow the
    earth's surface and terrain along great circles.
*/
    bool tessellate() const;


/*!
    \brief Sets the tessellation property for the LineString.

    If \a tessellate is <code>true</code> then the LineString's line segments
    are bent and follow the earth's surface and terrain along great circles.
    If \a tessellate is <code>false</code> then the LineString's line segments
    are rendered as straight lines in screen coordinate space.
*/
    void setTessellate( bool tessellate );


/*!
    \brief Returns the tessellation flags for a LineString.
*/
    TessellationFlags tessellationFlags() const;


/*!
    \brief Sets the given tessellation flags for a LineString.
*/
    void setTessellationFlags( TessellationFlags f );


/*!
    \brief Returns the smallest latLonAltBox that contains the LineString.

    \see GeoDataLatLonAltBox
*/

   const GeoDataLatLonAltBox& latLonAltBox() const override;

/**
  * @brief Returns the length of LineString across a sphere starting from a coordinate in LineString
  * This method can be used as an approximation for distances along LineStrings.
  * The unit used for the resulting length matches the unit of the planet
  * radius.
  * @param planetRadius radius of the sphere
  * @param offset position of coordinate within LineString
  */
    virtual qreal length( qreal planetRadius, int offset = 0 ) const;

/*!
    \brief The line string with nodes that have proper longitude/latitude ranges.

    \return A LineString that resembles the original linestring with nodes that
            have longitude values between -180 and +180 deg and that
            feature latitude values between -90 and +90 deg.

    Deprecation Warning: This method will likely be removed from the public API.
*/
    virtual GeoDataLineString toNormalized() const;


    // "Reimplementation" of QVector API
/*!
    \brief Returns whether the LineString has no nodes at all.

    \return <code>true</code> if there are no nodes inside the line string.
*/
    bool
    isEmpty() const;


    int
    size() const;

    int
    detail(int i) const;

    void
    setDetail(int i, int value);

    double
    altitude(int pos) const;


    double
    messure(int pos) const;

    double
    messureInfo(int pos) const;

    void
    getLonLat(int pos, double &lon, double &lat, GeoDataCoordinates::Unit unit) const;

/*!
    \brief Returns a reference to the coordinates of a node at a given position.
    This method does not detach the returned coordinate object from the line string.
*/
    GeoDataCoordinates at( int pos ) const;

/*!
    \brief Returns true/false depending on whether this and other are/are not equal.
*/
    bool operator==( const GeoDataLineString &other ) const;
    bool operator!=( const GeoDataLineString &other ) const;

    /*!
        \brief Returns a linestring with detail values assigned to each node.
    */
    GeoDataLineString optimized() const;

    QVector<QPointF>
    rawData() const;

    bool
    hasMessure() const;

    bool
    hasMessureInfo() const;

 protected:
    GeoDataLineString(GeoDataLineStringPrivate* priv);

 private:
    GeoDataLineStringPrivate *p();
    const GeoDataLineStringPrivate *p() const;
};

}

Q_DECLARE_METATYPE( Marble::GeoDataLineString )

#endif
