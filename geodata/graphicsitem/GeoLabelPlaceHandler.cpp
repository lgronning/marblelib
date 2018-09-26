#include "GeoLabelPlaceHandler.h"
#include "geodata/data/GeoDataFeature.h"
#include <QtCore/QRectF>

namespace Marble
{

GeoLabelPlaceHandler::GeoLabelPlaceHandler()
{

}

void GeoLabelPlaceHandler::clear()
{
    labelsMap.clear();
}

bool GeoLabelPlaceHandler::addLabelRect(const QRectF &labelRect, const GeoDataFeature *feature)
{
    if(feature == nullptr)
    {
        return true;
    }

    // Check if there is another label or symbol that overlaps.
    auto zIt = labelsMap.constBegin();
    auto zItEnd = labelsMap.constEnd();

    for(; zIt != zItEnd; ++zIt)
    {
        foreach(const FeatureRect &featureRect, zIt.value())
        {
            if ( labelRect.intersects(featureRect.rect) )
            {
                return false;
            }
        }
    }

    labelsMap[feature->zLevel()].append(FeatureRect(labelRect, feature));
    return true;
}

void
GeoLabelPlaceHandler::removeItem(const GeoDataFeature *feature)
{

    if(!labelsMap.contains(feature->zLevel()))
    {
        return;
    }

    auto &featureList = labelsMap[feature->zLevel()];
    for(int i = 0; i < featureList.size(); ++i)
    {
        if ( featureList[i].feature == feature)
        {
            featureList.removeAt(i);
            return;
        }
    }
}

const GeoDataFeature *GeoLabelPlaceHandler::getFeature(const QPointF &point)
{
    // Check if there is another label or symbol that overlaps.
    auto zIt = labelsMap.constBegin();
    auto zItEnd = labelsMap.constEnd();

    for(; zIt != zItEnd; ++zIt)
    {
        foreach(const FeatureRect &featureRect, zIt.value())
        {
            if ( featureRect.rect.contains(point))
            {
                return featureRect.feature;
            }
        }
    }

    return nullptr;

}

} // namespace
