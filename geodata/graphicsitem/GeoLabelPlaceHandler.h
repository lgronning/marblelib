#ifndef GEOLABELPLACEHANDLER_H
#define GEOLABELPLACEHANDLER_H

#include <QtCore/QVector>
#include <QtCore/QMap>
#include <QtCore/QRectF>
class QRectF;

namespace Marble
{

class GeoDataFeature;

struct FeatureRect
{
    FeatureRect()
        :   feature(nullptr)
    {}

    FeatureRect(const QRectF &_rect, const GeoDataFeature *_feature)
        :   rect(_rect),
            feature(_feature)
    {}

    QRectF rect;
    const GeoDataFeature *feature;
};

class GeoLabelPlaceHandler
{
public:
    GeoLabelPlaceHandler();

    void
    clear();

    bool
    addLabelRect(const QRectF &labelRect, const GeoDataFeature *feature);

    void
    removeItem(const GeoDataFeature *feature);

    const GeoDataFeature *
    getFeature(const QPointF &point);

private:
    QMap<int, QVector<FeatureRect> > labelsMap;
};

}

#endif // GEOLABELPLACEHANDLER_H
