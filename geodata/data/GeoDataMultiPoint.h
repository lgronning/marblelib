#ifndef GEODATAMULTIPOINT_H
#define GEODATAMULTIPOINT_H


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

class GeoDataMultiPointPrivate;

/*!
    \class GeoDataMultiPoint
    \brief A LineString that allows to store a contiguous set of line segments.

    GeoDataMultiPoint is a tool class that implements the LineString tag/class
    of the Open Geospatial Consortium standard KML 2.2.

    GeoDataMultiPoint extends GeoDataGeometry to store and edit
    LineStrings.

    In the QPainter API "pure" LineStrings are also referred to as "polylines".
    As such they are similar to the outline of a non-closed QPolygon.

    Whenever a LineString is painted GeoDataLineStyle should be used to assign a
    color and line width.

    A GeoDataMultiPoint consists of several (geodetic) nodes which are each
    connected through line segments. The nodes are stored as GeoDataCoordinates
    objects.

    The API which provides access to the nodes is similar to the API of
    QVector.

    GeoDataMultiPoint allows LineStrings to be tessellated in order to make them
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

class GEODATA_EXPORT GeoDataMultiPoint : public GeoDataGeometry
{

 public:
/*!
    \brief Creates a new LineString.
*/
    explicit GeoDataMultiPoint( const QVector<GeoDataCoordinates> &points);
    explicit GeoDataMultiPoint( const QVector<QPointF> &points);
    explicit GeoDataMultiPoint( const QVector<QwtPoint3D> &points);
    explicit GeoDataMultiPoint();

/*!
    \brief Creates a LineString from an existing geometry object.
*/
    explicit GeoDataMultiPoint( const GeoDataGeometry &other );


/*!
    \brief Destroys a LineString.
*/
    
    ~GeoDataMultiPoint() override;


/*!
    \brief Returns the smallest latLonAltBox that contains the LineString.

    \see GeoDataLatLonAltBox
*/

   const GeoDataLatLonAltBox& latLonAltBox() const override;

    bool
    isEmpty() const;

    int
    size() const;

    double
    altitude(int pos) const;

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
    bool operator==( const GeoDataMultiPoint &other ) const;
    bool operator!=( const GeoDataMultiPoint &other ) const;

protected:
    GeoDataMultiPoint(GeoDataMultiPointPrivate* priv);

 private:
    GeoDataMultiPointPrivate *p();
    const GeoDataMultiPointPrivate *p() const;
};

}

Q_DECLARE_METATYPE( Marble::GeoDataMultiPoint )


#endif // GEODATAMULTIPOINT_H
