#ifndef GEODATAMULTILINESTRING_H
#define GEODATAMULTILINESTRING_H

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

class GeoDataMultiLineStringPrivate;

class GeoDataMultiLineString : public GeoDataGeometry
{

 public:
/*!
    \brief Creates a new LineString.
*/
    explicit GeoDataMultiLineString( const QVector<QVector<GeoDataCoordinates> > &points, TessellationFlags f = NoTessellation, const QVector<QVector<double> > &messures = QVector<QVector<double> >(), const QVector<QVector<double> > &messureInfos = QVector<QVector<double> >() );
    explicit GeoDataMultiLineString( const QVector<QVector<QPointF> > &points, TessellationFlags f = NoTessellation, const QVector<QVector<double> > &messures = QVector<QVector<double> >(), const QVector<QVector<double> > &messureInfos = QVector<QVector<double> >() );
    explicit GeoDataMultiLineString( const QVector<QVector<QwtPoint3D> > &points, TessellationFlags f = NoTessellation, const QVector<QVector<double> > &messures = QVector<QVector<double> >(), const QVector<QVector<double> > &messureInfos = QVector<QVector<double> >() );
    explicit GeoDataMultiLineString();

/*!
    \brief Creates a LineString from an existing geometry object.
*/
    explicit GeoDataMultiLineString( const GeoDataGeometry &other );


/*!
    \brief Destroys a LineString.
*/
    ~GeoDataMultiLineString() override;

/*!
    \brief Returns the smallest latLonAltBox that contains the LineString.

    \see GeoDataLatLonAltBox
*/

   const GeoDataLatLonAltBox& latLonAltBox() const override;

/*!
    \brief Returns true/false depending on whether this and other are/are not equal.
*/
    bool operator==( const GeoDataMultiLineString &other ) const;
    bool operator!=( const GeoDataMultiLineString &other ) const;

    const QVector<GeoDataLineString> &lineStrings() const;

 protected:
    GeoDataMultiLineString(GeoDataMultiLineStringPrivate* priv);

 private:
    GeoDataMultiLineStringPrivate *p();
    const GeoDataMultiLineStringPrivate *p() const;
};

}

Q_DECLARE_METATYPE( Marble::GeoDataMultiLineString )

#endif // GEODATAMULTILINESTRING_H
