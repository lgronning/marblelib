#ifndef COLORMAP_H
#define COLORMAP_H

#include <QtCore/QSharedPointer>
#include <QtGui/QColor>

namespace Marble
{

struct ColorMapInterval;

class ColorMap
{
public:
    /*!
        Format for color mapping
        \sa rgb(), colorIndex(), colorTable()
    */

    enum Format
    {
        //! The map is intended to map into RGB values.
        RGB,

        /*!
          The map is intended to map into 8 bit values, that
          are indices into the color table.
         */
        Indexed
    };

    ColorMap( Format = ColorMap::RGB );
    virtual ~ColorMap();

    Format
    format() const;


    /*!
       Map a value of a given interval into a RGB value.

       \param interval Range for the values
       \param value Value
       \return RGB value, corresponding to value
    */
    virtual
    QRgb
    rgb( const ColorMapInterval &interval, double value ) const = 0;

    /*!
       Map a value of a given interval into a color index

       \param interval Range for the values
       \param value Value
       \return color index, corresponding to value
     */
    virtual unsigned char colorIndex(const ColorMapInterval &interval, double value ) const = 0;

    QColor
    color( const ColorMapInterval &, double value ) const;

    virtual
    QVector<QRgb>
    colorTable( const ColorMapInterval &interval ) const;

    virtual
    bool
    operator==( const ColorMap& other ) const;

    virtual
    bool
    operator!=( const ColorMap& other ) const;

private:
    Format d_format;
};

class LinearColorMap: public ColorMap
{
public:
    /*!
       Mode of color map
       \sa setMode(), mode()
    */
    enum Mode
    {
        //! Return the color from the next lower color stop
        FixedColors,

        //! Interpolating the colors of the adjacent stops.
        ScaledColors
    };

    LinearColorMap( ColorMap::Format = ColorMap::RGB );
    LinearColorMap( const QColor &from, const QColor &to, ColorMap::Format = ColorMap::RGB );

    ~LinearColorMap() override;

    void
    setMode( Mode );

    Mode
    mode() const;

    void
    setColorInterval( const QColor &color1, const QColor &color2 );

    void
    addColorStop( double value, const QColor& );

    QVector<double>
    colorStops() const;

    QVector<QColor>
    colorStopColors() const;

    QColor color1() const;
    QColor color2() const;

    QRgb rgb(const ColorMapInterval &interval, double value ) const override;

    /*!
      \brief Map a value of a given interval into a color index

      \param interval Range for all values
      \param value Value to map into a color index

      \return Index, between 0 and 255
    */
    unsigned char colorIndex(const ColorMapInterval &interval, double value ) const override;

    bool
    operator==( const LinearColorMap& other ) const;

    bool
    operator!=( const LinearColorMap& other ) const;

    class ColorStops;

private:
    // Disabled copy constructor and operator=
    LinearColorMap( const LinearColorMap & );
    LinearColorMap &operator=( const LinearColorMap & );

    class PrivateData;
    PrivateData *d_data;
};

typedef QSharedPointer<ColorMap> ColorMapPtr;

}

#endif // COLORMAP_H
