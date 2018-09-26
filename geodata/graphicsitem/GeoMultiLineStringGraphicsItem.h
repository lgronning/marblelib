#ifndef GEOMULTILINESTRINGGRAPHICSITEM_H
#define GEOMULTILINESTRINGGRAPHICSITEM_H

#include "graphicsview/GeoGraphicsItem.h"
#include "MarbleGlobal.h"

namespace Marble
{

class GeoDataMultiLineString;
class GeoDataLineStyle;

class MARBLE_EXPORT GeoMultiLineStringGraphicsItem : public GeoGraphicsItem
{
public:
    explicit GeoMultiLineStringGraphicsItem( const GeoDataFeature *feature, const GeoDataMultiLineString *lineStrings );

    
    ~GeoMultiLineStringGraphicsItem() override;

    const GeoDataLatLonAltBox& latLonAltBox() const override;

    
    void
    renderGeometry( GeoPainter* painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style ) override;

    
    void
    renderLabels( GeoPainter* painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style, GeoLabelPlaceHandler &placeHandler  ) override;

    
    void
    renderIcons( GeoPainter* painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style ) override;

    
    void
    getTiles(GeoSceneTextureTileDataset *tileDataset, const TileId &tileId, int zoomLevel, TileMap &tiles, std::atomic<bool> &aCancel) override;

protected:

    const GeoDataMultiLineString *m_lineStrings;
};

}


#endif // GEOMULTILINESTRINGGRAPHICSITEM_H
