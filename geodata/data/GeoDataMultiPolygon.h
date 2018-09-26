#ifndef GEODATAMULTIPOLYGON_H
#define GEODATAMULTIPOLYGON_H

#include <QVector>

#include "MarbleGlobal.h"

#include "geodata/geodata_export.h"
#include "geodata/data/GeoDataGeometry.h"
#include "geodata/data/GeoDataLinearRing.h"
#include "geodata/data/GeoDataLatLonAltBox.h"

namespace Marble
{

class GeoDataMultiPolygonPrivate;


class GEODATA_EXPORT GeoDataMultiPolygon : public GeoDataGeometry
{
 public:
/*!
    \brief Creates a new Polygon.
*/
    explicit GeoDataMultiPolygon(QVector<Marble::GeoDataPolygon> polygons );
    explicit GeoDataMultiPolygon(GeoDataMultiPolygonPrivate *priv);

    GeoDataMultiPolygon();

/*!
    \brief Creates a Polygon from an existing geometry object.
*/
    explicit GeoDataMultiPolygon( const GeoDataGeometry &other );


/*!
    \brief Destroys a Polygon.
*/
    ~GeoDataMultiPolygon() override;


/*!
    \brief Returns true/false depending on whether this and other are/are not equal.
*/
    bool operator==( const GeoDataMultiPolygon &other ) const;
    bool operator!=( const GeoDataMultiPolygon &other ) const;


/*!
    \brief Returns the smallest latLonAltBox that contains the Polygon.

    \see GeoDataLatLonAltBox
*/
    const GeoDataLatLonAltBox& latLonAltBox() const override;

    const QVector<GeoDataPolygon> &
    polygons() const;

 private:
    GeoDataMultiPolygonPrivate *p();
    const GeoDataMultiPolygonPrivate *p() const;
};

}

#endif // GEODATAMULTIPOLYGON_H
