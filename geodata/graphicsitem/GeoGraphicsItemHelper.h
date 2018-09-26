#ifndef GEOGRAPHICSITEMHELPER_H
#define GEOGRAPHICSITEMHELPER_H


#include <QtCore/QSize>

namespace Marble
{

class GeoDataFeature;
class GeoDataGeometry;
class GeoSceneTextureTileDataset;
class TileId;

namespace GeoGraphicsItemHelper
{

QSizeF
getSize(const GeoDataFeature* feature, const GeoDataGeometry *geometry);


double
getTileIdRadius(GeoSceneTextureTileDataset *tileDataset, const TileId &tileId);


}

}

#endif // GEOGRAPHICSITEMHELPER_H
