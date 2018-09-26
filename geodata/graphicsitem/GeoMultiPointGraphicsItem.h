#ifndef GEOMULTIPOINTGRAPHICSITEM_H
#define GEOMULTIPOINTGRAPHICSITEM_H


#include "geodata/data/GeoDataMultiPoint.h"
#include "graphicsview/GeoGraphicsItem.h"
#include "geodata/data/GeoDataLatLonAltBox.h"
#include "marble_export.h"

namespace Marble
{

class MARBLE_EXPORT GeoMultiPointGraphicsItem : public GeoGraphicsItem
{
public:
    explicit GeoMultiPointGraphicsItem(const GeoDataFeature *feature , const GeoDataMultiPoint *points);

    void
    setPoints( const GeoDataMultiPoint* point );

    const GeoDataMultiPoint *
    points() const;

    
    void
    renderGeometry( GeoPainter* painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style) override;


    
    void
    renderIcons( GeoPainter* painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style ) override;

    
    void
    renderLabels( GeoPainter* painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style, GeoLabelPlaceHandler &placeHandler  ) override;

    
    const
    GeoDataLatLonAltBox& latLonAltBox() const override;

    
    void
    getTiles(GeoSceneTextureTileDataset *tileDataset, const TileId &tile, int zoomLevel, TileMap &tiles, std::atomic<bool> &aCancel) override;

protected:   
    const GeoDataMultiPoint *m_points;
};

}

#endif // GEOMULTIPOINTGRAPHICSITEM_H
