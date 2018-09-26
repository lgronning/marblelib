#include "ColorMapInterval.h"

namespace Marble
{

ColorMapInterval::ColorMapInterval(double _begin, double _end)
    :   begin(_begin),
      end(_end)
{}

ColorMapInterval::ColorMapInterval()
    :   begin(-1),
      end(-1)
{}

bool ColorMapInterval::isValid() const
{
    return begin < end;
}

}
