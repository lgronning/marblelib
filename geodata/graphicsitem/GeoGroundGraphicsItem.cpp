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

#include "GeoGroundGraphicsItem.h"
#include "GeoPolygonGraphicsItem.h"
#include "GeoGraphicsItemHelper.h"
#include "GeoPainter.h"
#include "ViewportParams.h"
#include "geodata/data/GeoDataFeature.h"
#include "geodata/data/GeoDataStyle.h"
#include "geodata/data/GeoDataGroundOverlay.h"

#include "imagerenderers/ImageMapperContext.h"
#include "imagerenderers/SphericalImageRenderer.h"
#include "imagerenderers/MercatorImageRenderer.h"
#include "imagerenderers/GenericImageRenderer.h"
#include "imagerenderers/EquirectImageRenderer.h"

#include <QtCore/QDebug>
#include <QtCore/QTime>

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/adapted/c_array.hpp>

using namespace Marble;

BOOST_GEOMETRY_REGISTER_C_ARRAY_CS(cs::cartesian)

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/register/point.hpp>

BOOST_GEOMETRY_REGISTER_POINT_2D_GET_SET(QPointF, double, boost::geometry::cs::cartesian, x, y, setX, setY)


namespace Marble
{

GeoGroundGraphicsItem::GeoGroundGraphicsItem(const GeoDataGroundOverlay *feature)
    :   GeoGraphicsItem(feature)
{
    const GeoDataGroundOverlay* overlay = dynamic_cast<const GeoDataGroundOverlay*>(feature);

    m_latLonAltBox = GeoDataLatLonAltBox(overlay->latLonBox(), overlay->altitude(), overlay->altitude());
}

GeoGroundGraphicsItem::~GeoGroundGraphicsItem()
{
}

void GeoGroundGraphicsItem::renderGeometry(GeoPainter *painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style)
{
    const GeoDataGroundOverlay* overlay = dynamic_cast<const GeoDataGroundOverlay*>(feature());
    QImage image = overlay->icon();
    Marble::GeoDataLatLonBox overlayLatLonBox = overlay->latLonBox();

    QTime timer;
    timer.start();

    QSharedPointer<ImageRenderInterface> imageRenderInterface;
    QRect canvasRect(QPoint(0,0), viewport->size());
    switch( viewport->projection() )
    {
        case Spherical:
            imageRenderInterface = QSharedPointer<ImageRenderInterface>(new SphericalImageRenderer(viewport, canvasRect, image, overlayLatLonBox));
            break;
        case Equirectangular:
            imageRenderInterface = QSharedPointer<ImageRenderInterface>(new EquirectImageRenderer(viewport, canvasRect, image, overlayLatLonBox));
            break;
        case Mercator:
            imageRenderInterface = QSharedPointer<ImageRenderInterface>(new MercatorImageRenderer(viewport, canvasRect, image, overlayLatLonBox));
            break;
        case Gnomonic:
        case Stereographic:
        case LambertAzimuthal:
        case AzimuthalEquidistant:
        case VerticalPerspective:
            imageRenderInterface = QSharedPointer<ImageRenderInterface>(new GenericImageRenderer(viewport, canvasRect, image, overlayLatLonBox));
            break;
    }

    if(imageRenderInterface)
    {
        imageRenderInterface->renderImage(painter);
    }

    qWarning() << "elapsed drawImage" << timer.elapsed() << painter->mapQuality();
}

void
GeoGroundGraphicsItem::renderLabels(GeoPainter *painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style, GeoLabelPlaceHandler &placeHandler)
{

}

void
GeoGroundGraphicsItem::renderIcons(GeoPainter *painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style)
{

}

const GeoDataLatLonAltBox&
GeoGroundGraphicsItem::latLonAltBox() const
{
    return m_latLonAltBox;
}

namespace
{

void getTilesImpl(GeoSceneTextureTileDataset *tileDataset, const TileId &tileId, int zoomLevel, const GeoDataLatLonAltBox &m_latLonAltBox, TileMap &tiles, std::atomic<bool> &cancel)
{
    double north;
    double south;
    double east;
    double west;

    m_latLonAltBox.boundaries(north, south, east, west, GeoDataCoordinates::Radian);

    double radius = ( 1 << tileId.tileLevel() ) *tileDataset->levelZeroColumns()  / 2.0;

    double lonLeft   = ( tileId.x() - radius) / radius * M_PI;
    double lonRight  = ( tileId.x() - radius + 1 ) / radius * M_PI;

    radius = ( 1 << tileId.tileLevel() ) * tileDataset->levelZeroRows() / 2.0;

    double latBottom = 0;
    double latTop = 0;

    switch ( tileDataset->projection() ) {
    case GeoSceneTileDataset::Equirectangular:
        latTop = (radius - tileId.y() - 1) / radius * M_PI / 2.0;
        latBottom = (radius - tileId.y()) / radius * M_PI / 2.0;
        break;
    case GeoSceneTileDataset::Mercator:
        latTop = atan( sinh( ( radius - tileId.y() - 1) / radius * M_PI ) );
        latBottom = atan( sinh( ( radius - tileId.y()) / radius * M_PI ) );
        break;
    }

    typedef boost::geometry::model::d2::point_xy<double> point_type;
    typedef boost::geometry::model::box<point_type> box_type;

    box_type box = boost::geometry::make<box_type>(lonLeft,
                                                   latTop,
                                                   lonRight,
                                                   latBottom);

    box_type box2 = boost::geometry::make<box_type>(west,
                                                    south,
                                                    east,
                                                    north);


    if(boost::geometry::within(box, box2))
    {
        tiles[tileId.tileLevel()][tileId.x()][tileId.y()] = Marble::TileStatus::Full;
    }
    else if(boost::geometry::within(box2, box) || boost::geometry::intersects(box2, box))
    {
        if (tileId.tileLevel()< zoomLevel)
        {
            TileId tile1(tileId.mapThemeIdHash(), tileId.tileLevel()+1, tileId.x()*2, tileId.y()*2);
            TileId tile2(tileId.mapThemeIdHash(), tileId.tileLevel()+1, (tileId.x()*2) + 1, tileId.y()*2);
            TileId tile3(tileId.mapThemeIdHash(), tileId.tileLevel()+1, tileId.x()*2, (tileId.y()*2)+1);
            TileId tile4(tileId.mapThemeIdHash(), tileId.tileLevel()+1, (tileId.x()*2) + 1, (tileId.y()*2)+1);

            getTilesImpl(tileDataset, tile1, zoomLevel, m_latLonAltBox, tiles, cancel);
            getTilesImpl(tileDataset, tile2, zoomLevel, m_latLonAltBox, tiles, cancel);
            getTilesImpl(tileDataset, tile3, zoomLevel, m_latLonAltBox, tiles, cancel);
            getTilesImpl(tileDataset, tile4, zoomLevel, m_latLonAltBox, tiles, cancel);
        }
        else
        {
            tiles[tileId.tileLevel()][tileId.x()][tileId.y()] = Marble::TileStatus::Partially;
        }
    }
}

}

void GeoGroundGraphicsItem::getTiles(GeoSceneTextureTileDataset *tileDataset, const TileId &tile, int zoomLevel, TileMap &tiles, std::atomic<bool> &cancel)
{
    getTilesImpl(tileDataset, tile, zoomLevel, m_latLonAltBox, tiles, cancel);
}

}
