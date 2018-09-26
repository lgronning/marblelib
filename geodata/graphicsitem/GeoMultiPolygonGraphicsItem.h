#ifndef GEOMULTIPOLYGONGRAPHICSITEM_H
#define GEOMULTIPOLYGONGRAPHICSITEM_H


#include "graphicsview/GeoGraphicsItem.h"
#include "MarbleGlobal.h"

namespace Marble
{

class GeoDataMultiPolygon;

class MARBLE_EXPORT GeoMultiPolygonGraphicsItem : public GeoGraphicsItem
{
public:
    explicit GeoMultiPolygonGraphicsItem( const GeoDataFeature *feature, const GeoDataMultiPolygon *polygons );

    
    ~GeoMultiPolygonGraphicsItem() override;

    const GeoDataLatLonAltBox& latLonAltBox() const override;

    
    void
    renderGeometry( GeoPainter* painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style ) override;

    
    void
    renderLabels( GeoPainter* painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style, GeoLabelPlaceHandler &placeHandler  ) override;


    
    void
    renderIcons( GeoPainter* painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style ) override;

    
    void
    getTiles(GeoSceneTextureTileDataset *tileDataset, const TileId &tile, int zoomLevel, TileMap &tiles, std::atomic<bool> &aCancel) override;

protected:
    const GeoDataMultiPolygon *m_polygons;
    QString m_cachedTexturePath;
    QColor m_cachedTextureColor;
    QImage m_cachedTexture;
};

}

#endif // GEOMULTIPOLYGONGRAPHICSITEM_H
