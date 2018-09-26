#include "GeoGraphicsItemHelper.h"
#include "geodata/data/GeoDataFeature.h"
#include "geodata/data/GeoDataLineString.h"
#include "geodata/data/GeoDataLineStyle.h"
#include "geodata/data/GeoDataPolyStyle.h"
#include "geodata/data/GeoDataLabelStyle.h"
#include "geodata/data/GeoDataIconStyle.h"
#include "geodata/data/GeoDataPointStyle.h"
#include "geodata/data/GeoDataStyle.h"
#include "geodata/scene/GeoSceneTextureTileDataset.h"
#include "TileId.h"
#include "MarbleGlobal.h"


namespace
{

QSizeF
getStyleSize(QSharedPointer<const Marble::GeoDataStyle> style, const Marble::GeoDataGeometry *geometry)
{
    QSizeF size(0,0);

    double penWidth = style->lineStyle().width();
    if( style->lineStyle().cosmeticOutline() &&
        style->lineStyle().penStyle() == Qt::SolidLine )
    {
        if ( penWidth > 2.5 )
        {
            penWidth -= 2.0;
        }
    }

    size = size.expandedTo(QSizeF(penWidth, penWidth));

    QPixmap linePointPixmap = style->lineStyle().scaledPointPixmap();
    if(!linePointPixmap.isNull())
    {
        const Marble::GeoDataLineString *lineString = dynamic_cast<const Marble::GeoDataLineString *>(geometry);

        if(style->lineStyle().alwaysUsePointPixmap() || (!style->lineStyle().alwaysUsePointPixmap() && (!lineString || (lineString && lineString->size() <= 2))))
        {
            double width = static_cast<double>(linePointPixmap.width());
            double height = static_cast<double>(linePointPixmap.height());
            size = size.expandedTo(QSizeF(width, height));
        }
    }

    QPixmap iconsPixmap = style->iconStyle().scaledIcon();
    if(!iconsPixmap.isNull())
    {
        double width = static_cast<double>(linePointPixmap.width());
        double height = static_cast<double>(linePointPixmap.height());
        size = size.expandedTo(QSizeF(width, height));
    }

    QPixmap pointPixmap = style->pointStyle().scaledIcon();
    if(!pointPixmap.isNull())
    {
        double width = static_cast<double>(pointPixmap.width());
        double height = static_cast<double>(pointPixmap.height());
        size = size.expandedTo(QSizeF(width, height));
    }

    return size;

}

}

QSizeF
Marble::GeoGraphicsItemHelper::getSize(const Marble::GeoDataFeature *feature, const Marble::GeoDataGeometry *geometry)
{
    QSizeF size(0,0);
    if(feature->style())
    {
        size = size.expandedTo(getStyleSize(feature->style(), geometry));
    }

    if(feature->hightlighStyle())
    {
        size = size.expandedTo(getStyleSize(feature->hightlighStyle(), geometry));
    }

    return size;
}


double
Marble::GeoGraphicsItemHelper::getTileIdRadius(GeoSceneTextureTileDataset *tileDataset, const TileId &tileId)
{
    // choose the smaller dimension for selecting the tile level, leading to higher-resolution results
    const int levelZeroWidth = tileDataset->tileSize().width() * tileDataset->levelZeroColumns();
    const int levelZeroHight = tileDataset->tileSize().height() * tileDataset->levelZeroRows();
    const int levelZeroMinDimension = qMin( levelZeroWidth, levelZeroHight );

    double tileLevelF = tileId.tileLevel();
    double linearLevel = std::exp(tileLevelF * std::log( 2.0 ) );

    if ( linearLevel < 1.0 )
        linearLevel = 1.0; // Dirty fix for invalid entry linearLevel

    return  (static_cast<double>( levelZeroMinDimension ) * linearLevel) /4.0 ;
}
