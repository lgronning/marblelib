#include <QUrl>

#include "geodata/data/GeoDataSpectroStyle.h"
#include "MarbleDirs.h"
#include "MarbleDebug.h"
#include "RemoteIconLoader.h"

#include "geodata/parser/GeoDataTypes.h"

namespace Marble
{

class GeoDataSpectroStylePrivate
{
public:
    GeoDataSpectroStylePrivate()
    {
    }

    const char* nodeType() const
    {
        return GeoDataTypes::GeoDataSpectroStyleType;
    }

    ColorMapPtr colorMap;
    ColorMapInterval interval;

};

GeoDataSpectroStyle::GeoDataSpectroStyle() :
    d( new GeoDataSpectroStylePrivate() )
{
}

GeoDataSpectroStyle::GeoDataSpectroStyle( const GeoDataSpectroStyle& other )
    :   GeoDataObject( other ),
        d( new GeoDataSpectroStylePrivate( *other.d ) )
{
}

GeoDataSpectroStyle::GeoDataSpectroStyle(ColorMapPtr colorMap, ColorMapInterval interval) :
    d( new GeoDataSpectroStylePrivate( ) )
{
    d->colorMap = colorMap;
    d->interval = interval;

}

GeoDataSpectroStyle::~GeoDataSpectroStyle()
{
    delete d;
}

GeoDataSpectroStyle& GeoDataSpectroStyle::operator=( const GeoDataSpectroStyle& other )
{
    GeoDataObject::operator=( other );
    *d = *other.d;
    return *this;
}

bool GeoDataSpectroStyle::operator==( const GeoDataSpectroStyle &other ) const
{
    return d->colorMap->operator==(*other.d->colorMap.data()) &&
            qFuzzyCompare(d->interval.begin, other.d->interval.begin) &&
            qFuzzyCompare(d->interval.end, other.d->interval.end);
}

bool GeoDataSpectroStyle::operator!=( const GeoDataSpectroStyle &other ) const
{
    return !this->operator==( other );
}

void GeoDataSpectroStyle::setColorMap(ColorMapPtr colorMap)
{
    d->colorMap = colorMap;
}

void GeoDataSpectroStyle::setColorInterval(const ColorMapInterval &interval)
{
    d->interval = interval;
}

ColorMapPtr GeoDataSpectroStyle::colorMap() const
{
    return d->colorMap;
}

const ColorMapInterval &GeoDataSpectroStyle::colorInterval() const
{
    return d->interval;
}


const char* GeoDataSpectroStyle::nodeType() const
{
    return d->nodeType();
}

}
