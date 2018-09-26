#ifndef GEODATASPECTROSTYLE_H
#define GEODATASPECTROSTYLE_H

#include "geodata/data/GeoDataObject.h"
#include "ColorMap.h"
#include "ColorMapInterval.h"

#include "geodata/geodata_export.h"
#include <QtCore/QSharedPointer>
#include <QtCore/QVector>
#include <QtGui/QColor>

namespace Marble
{

class GeoDataSpectroStylePrivate;

class GEODATA_EXPORT GeoDataSpectroStyle : public GeoDataObject
{
public:
    GeoDataSpectroStyle();

    GeoDataSpectroStyle( const GeoDataSpectroStyle& other );

    explicit GeoDataSpectroStyle(ColorMapPtr colorMap, ColorMapInterval interval);
    ~GeoDataSpectroStyle() override;

    GeoDataSpectroStyle& operator=( const GeoDataSpectroStyle& other );

    bool
    operator==( const GeoDataSpectroStyle &other ) const;

    bool
    operator!=( const GeoDataSpectroStyle &other ) const;

    void
    setColorMap(ColorMapPtr colorMap);

    void
    setColorInterval(const ColorMapInterval &interval);


    ColorMapPtr
    colorMap() const;

    const ColorMapInterval &
    colorInterval() const;

    /// Provides type information for downcasting a GeoData
    const char*
    nodeType() const override;

private:
    GeoDataSpectroStylePrivate * const d;
};

}

#endif // GEODATASPECTROSTYLE_H
