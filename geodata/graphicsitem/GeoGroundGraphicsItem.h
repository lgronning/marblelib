#ifndef GEOGROUNDGRAPHICSITEM_H
#define GEOGROUNDGRAPHICSITEM_H

#include "geodata/data/GeoDataPoint.h"
#include "geodata/data/GeoDataLatLonAltBox.h"
#include "graphicsview/GeoGraphicsItem.h"
#include "marble_export.h"

#include <QImage>

namespace Marble
{

class GeoDataGroundOverlay;

class MARBLE_EXPORT GeoGroundGraphicsItem : public GeoGraphicsItem
{
public:
    explicit
    GeoGroundGraphicsItem(const GeoDataGroundOverlay *feature );

    
    ~GeoGroundGraphicsItem() override;

    
    void
    renderGeometry( GeoPainter* painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style  ) override;

    
    void
    renderLabels( GeoPainter* painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style, GeoLabelPlaceHandler &placeHandler ) override;

    
    void
    renderIcons( GeoPainter* painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style ) override;

    
    const
    GeoDataLatLonAltBox&
    latLonAltBox() const override;

    
    void
    getTiles(GeoSceneTextureTileDataset *tileDataset, const TileId &tile, int zoomLevel, TileMap &tiles, std::atomic<bool> &aCancel) override;

protected:
    GeoDataLatLonAltBox m_latLonAltBox;
};

}

#endif // GEOGROUNDGRAPHICSITEM_H
