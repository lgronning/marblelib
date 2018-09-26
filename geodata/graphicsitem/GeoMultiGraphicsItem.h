#ifndef GEOMULTIGRAPHICSITEM_H
#define GEOMULTIGRAPHICSITEM_H


#include "graphicsview/GeoGraphicsItem.h"
#include "marble_export.h"

namespace Marble
{

class GeoDataMultiGeometry;
class GeoDataLineStyle;

class MARBLE_EXPORT GeoMultiGraphicsItem : public GeoGraphicsItem
{
public:
    explicit GeoMultiGraphicsItem( const GeoDataFeature *feature, const GeoDataMultiGeometry *multiGeometry );

    void setMultiGeometry( const GeoDataMultiGeometry* multiGeometry );

    
    const
    GeoDataLatLonAltBox& latLonAltBox() const override;

    
    void
    renderGeometry( GeoPainter* painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style ) override;

    
    void
    renderLabels( GeoPainter* painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style, GeoLabelPlaceHandler &placeHandler  ) override;


    
    void
    renderIcons( GeoPainter* painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style ) override;

protected:
    const GeoDataMultiGeometry *m_multiGeometry;
};

}

#endif // GEOMULTIGRAPHICSITEM_H
