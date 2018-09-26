#ifndef COLORMAPINTERVAL_H
#define COLORMAPINTERVAL_H

namespace Marble
{

struct ColorMapInterval
{
    ColorMapInterval(double _begin, double _end);

    ColorMapInterval();

    bool isValid() const;

    inline
    double
    width() const
    {
        return isValid() ? ( end - begin) : 0.0;
    }

    double begin;
    double end;
};

}

#endif // COLORMAPINTERVAL_H
