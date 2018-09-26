#include "ColorMap.h"
#include "ColorMapInterval.h"
#include <QtCore/QVector>

namespace Marble
{

class LinearColorMap::ColorStops
{
public:
    ColorStops():
        d_doAlpha( false )
    {
        d_stops.reserve( 256 );
    }

    void insert( double pos, const QColor &color );
    QRgb rgb( LinearColorMap::Mode, double pos ) const;

    QVector<double> stops() const;

    bool
    operator==( const LinearColorMap::ColorStops& other ) const;

    bool
    operator!=( const LinearColorMap::ColorStops& other ) const;

private:

    class ColorStop
    {
    public:
        ColorStop():
            pos( 0.0 ),
            rgb( 0 )
        {
        }

        ColorStop( double p, const QColor &c ):
            pos( p ),
            rgb( c.rgba() )
        {
            r = qRed( rgb );
            g = qGreen( rgb );
            b = qBlue( rgb );
            a = qAlpha( rgb );

            /*
                when mapping a value to rgb we will have to calcualate:
                   - const int v = int( ( s1.v0 + ratio * s1.vStep ) + 0.5 );

                Thus adding 0.5 ( for rounding ) can be done in advance
             */
            r0 = r + 0.5;
            g0 = g + 0.5;
            b0 = b + 0.5;
            a0 = a + 0.5;

            rStep = gStep = bStep = aStep = 0.0;
            posStep = 0.0;
        }

        bool
        operator==( const LinearColorMap::ColorStops::ColorStop& other ) const
        {
            return pos == other.pos &&
                   rgb == other.rgb;
        }

        bool
        operator!=( const LinearColorMap::ColorStops::ColorStop& other ) const
        {
            return !operator==(other);
        }


        void updateSteps( const ColorStop &nextStop )
        {
            rStep = nextStop.r - r;
            gStep = nextStop.g - g;
            bStep = nextStop.b - b;
            aStep = nextStop.a - a;
            posStep = nextStop.pos - pos;
        }

        double pos;
        QRgb rgb;
        int r, g, b, a;

        // precalculated values
        double rStep, gStep, bStep, aStep;
        double r0, g0, b0, a0;
        double posStep;
    };

    inline int findUpper( double pos ) const;
    QVector<ColorStop> d_stops;
    bool d_doAlpha;
};

void LinearColorMap::ColorStops::insert( double pos, const QColor &color )
{
    // Lookups need to be very fast, insertions are not so important.
    // Anyway, a balanced tree is what we need here. TODO ...

    if ( pos < 0.0 || pos > 1.0 )
        return;

    int index;
    if ( d_stops.size() == 0 )
    {
        index = 0;
        d_stops.resize( 1 );
    }
    else
    {
        index = findUpper( pos );
        if ( index == d_stops.size() ||
                qAbs( d_stops[index].pos - pos ) >= 0.001 )
        {
            d_stops.resize( d_stops.size() + 1 );
            for ( int i = d_stops.size() - 1; i > index; i-- )
                d_stops[i] = d_stops[i-1];
        }
    }

    d_stops[index] = ColorStop( pos, color );
    if ( color.alpha() != 255 )
        d_doAlpha = true;

    if ( index > 0 )
        d_stops[index-1].updateSteps( d_stops[index] );

    if ( index < d_stops.size() - 1 )
        d_stops[index].updateSteps( d_stops[index+1] );
}

inline QVector<double> LinearColorMap::ColorStops::stops() const
{
    QVector<double> positions( d_stops.size() );
    for ( int i = 0; i < d_stops.size(); i++ )
        positions[i] = d_stops[i].pos;
    return positions;
}

bool
LinearColorMap::ColorStops::operator==(const LinearColorMap::ColorStops &other) const
{
    return d_stops == other.d_stops &&
            d_doAlpha == other.d_doAlpha;
}

bool
LinearColorMap::ColorStops::operator!=(const LinearColorMap::ColorStops &other) const
{
    return !operator==(other);
}

inline int LinearColorMap::ColorStops::findUpper( double pos ) const
{
    int index = 0;
    int n = d_stops.size();

    const ColorStop *stops = d_stops.data();

    while ( n > 0 )
    {
        const int half = n >> 1;
        const int middle = index + half;

        if ( stops[middle].pos <= pos )
        {
            index = middle + 1;
            n -= half + 1;
        }
        else
            n = half;
    }

    return index;
}

inline QRgb LinearColorMap::ColorStops::rgb(
    LinearColorMap::Mode mode, double pos ) const
{
    if ( pos <= 0.0 )
        return d_stops[0].rgb;
    if ( pos >= 1.0 )
        return d_stops[ d_stops.size() - 1 ].rgb;

    const int index = findUpper( pos );
    if ( mode == FixedColors )
    {
        return d_stops[index-1].rgb;
    }
    else
    {
        const ColorStop &s1 = d_stops[index-1];

        const double ratio = ( pos - s1.pos ) / ( s1.posStep );

        const int r = int( s1.r0 + ratio * s1.rStep );
        const int g = int( s1.g0 + ratio * s1.gStep );
        const int b = int( s1.b0 + ratio * s1.bStep );

        if ( d_doAlpha )
        {
            if ( s1.aStep )
            {
                const int a = int( s1.a0 + ratio * s1.aStep );
                return qRgba( r, g, b, a );
            }
            else
            {
                return qRgba( r, g, b, s1.a );
            }
        }
        else
        {
            return qRgb( r, g, b );
        }
    }
}

//! Constructor
ColorMap::ColorMap( Format format ):
    d_format( format )
{
}

//! Destructor
ColorMap::~ColorMap()
{
}

ColorMap::Format ColorMap::format() const
{
    return d_format;
}

QVector<QRgb> ColorMap::colorTable(const ColorMapInterval &interval) const
{
    QVector<QRgb> table( 256 );

    if ( interval.isValid() )
    {
        const double step = interval.width() / ( table.size() - 1 );
        for ( int i = 0; i < table.size(); i++ )
            table[i] = rgb( interval, interval.begin + step * i );
    }

    return table;
}

bool ColorMap::operator==(const ColorMap &other) const
{
    return d_format == other.d_format;
}

bool ColorMap::operator!=(const ColorMap &other) const
{
    return !operator==(other);
}


class LinearColorMap::PrivateData
{
public:
    ColorStops colorStops;
    LinearColorMap::Mode mode;
};

/*!
   Build a color map with two stops at 0.0 and 1.0. The color
   at 0.0 is Qt::blue, at 1.0 it is Qt::yellow.

   \param format Preferred format of the color map
*/
LinearColorMap::LinearColorMap( ColorMap::Format format ):
    ColorMap( format )
{
    d_data = new PrivateData;
    d_data->mode = ScaledColors;

    setColorInterval( Qt::blue, Qt::yellow );
}

/*!
   Build a color map with two stops at 0.0 and 1.0.

   \param color1 Color used for the minimum value of the value interval
   \param color2 Color used for the maximum value of the value interval
   \param format Preferred format for the color map
*/
LinearColorMap::LinearColorMap( const QColor &color1,
        const QColor &color2, ColorMap::Format format ):
    ColorMap( format )
{
    d_data = new PrivateData;
    d_data->mode = ScaledColors;
    setColorInterval( color1, color2 );
}

//! Destructor
LinearColorMap::~LinearColorMap()
{
    delete d_data;
}

/*!
   \brief Set the mode of the color map

   FixedColors means the color is calculated from the next lower
   color stop. ScaledColors means the color is calculated
   by interpolating the colors of the adjacent stops.

   \sa mode()
*/
void LinearColorMap::setMode( Mode mode )
{
    d_data->mode = mode;
}

/*!
   \return Mode of the color map
   \sa setMode()
*/
LinearColorMap::Mode LinearColorMap::mode() const
{
    return d_data->mode;
}

/*!
   Set the color range

   Add stops at 0.0 and 1.0.

   \param color1 Color used for the minimum value of the value interval
   \param color2 Color used for the maximum value of the value interval

   \sa color1(), color2()
*/
void LinearColorMap::setColorInterval(
    const QColor &color1, const QColor &color2 )
{
    d_data->colorStops = ColorStops();
    d_data->colorStops.insert( 0.0, color1 );
    d_data->colorStops.insert( 1.0, color2 );
}

/*!
   Add a color stop

   The value has to be in the range [0.0, 1.0].
   F.e. a stop at position 17.0 for a range [10.0,20.0] must be
   passed as: (17.0 - 10.0) / (20.0 - 10.0)

   \param value Value between [0.0, 1.0]
   \param color Color stop
*/
void LinearColorMap::addColorStop( double value, const QColor& color )
{
    if ( value >= 0.0 && value <= 1.0 )
        d_data->colorStops.insert( value, color );
}

/*!
   \return Positions of color stops in increasing order
*/
QVector<double>
LinearColorMap::colorStops() const
{
    return d_data->colorStops.stops();
}

QVector<QColor>
LinearColorMap::colorStopColors() const
{
    QVector<QColor> colors;
    foreach(double stop, colorStops())
    {
        colors.append(d_data->colorStops.rgb(d_data->mode, stop));
    }

    return colors;
}

/*!
  \return the first color of the color range
  \sa setColorInterval()
*/
QColor LinearColorMap::color1() const
{
    return QColor( d_data->colorStops.rgb( d_data->mode, 0.0 ) );
}

/*!
  \return the second color of the color range
  \sa setColorInterval()
*/
QColor LinearColorMap::color2() const
{
    return QColor( d_data->colorStops.rgb( d_data->mode, 1.0 ) );
}

QRgb LinearColorMap::rgb(const ColorMapInterval &interval, double value) const
{
    if ( qIsNaN(value) )
        return 0u;

    const double width = interval.width();
    if ( width <= 0.0 )
        return 0u;

    const double ratio = ( value - interval.begin ) / width;
    return d_data->colorStops.rgb( d_data->mode, ratio );
}

unsigned char LinearColorMap::colorIndex(const ColorMapInterval &interval, double value) const
{
    const double width = interval.width();

    if ( qIsNaN(value) || width <= 0.0 || value <= interval.begin )
        return 0;

    if ( value >= interval.end )
        return 255;

    const double ratio = ( value - interval.begin) / width;

    unsigned char index;
    if ( d_data->mode == FixedColors )
        index = static_cast<unsigned char>( ratio * 255 ); // always floor
    else
        index = static_cast<unsigned char>( ratio * 255 + 0.5 );

    return index;
}

bool LinearColorMap::operator==(const LinearColorMap &other) const
{
    if(!ColorMap::operator==(other))
    {
        return false;
    }

    return d_data->colorStops == d_data->colorStops &&
            d_data->mode == d_data->mode;
}


bool LinearColorMap::operator!=(const LinearColorMap &other) const
{
    return !operator==(other);
}

}
