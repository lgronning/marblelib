//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010      Harshit Jain <hjain.itbhu@gmail.com>
//

#ifndef MARBLE_GEODATATIMESPAN_H
#define MARBLE_GEODATATIMESPAN_H

#include <QString>
#include <QDateTime>

#include "geodata/data/GeoDataObject.h"
#include "geodata/data/GeoDataTimePrimitive.h"
#include "geodata/data/GeoDataTimeStamp.h"

#include "geodata/geodata_export.h"

namespace Marble
{

class GeoDataTimeSpanPrivate;

class GEODATA_EXPORT GeoDataTimeSpan : public GeoDataTimePrimitive
{
  public:

    GeoDataTimeSpan();
    GeoDataTimeSpan( const GeoDataTimeSpan& other );
    ~GeoDataTimeSpan() override;

    /**
    * @brief assignment operator
    */
    GeoDataTimeSpan& operator=( const GeoDataTimeSpan& other );

    /**
     * @brief equality operators
     */
    bool operator==( const GeoDataTimeSpan& other ) const;
    bool operator!=( const GeoDataTimeSpan& other ) const;

    /// Provides type information for downcasting a GeoNode
    const char* nodeType() const override;

    /**
    * @brief return the beginning instant of a timespan
    */
    const GeoDataTimeStamp & begin() const;
    GeoDataTimeStamp & begin();

    /**
    * @brief Set the beginning instant of a timespan
    * @param begin the beginning instant of a timespan
    */
    void setBegin( const GeoDataTimeStamp& begin );
    
    /**
    * @brief return the ending instant of a timespan
    */
    const GeoDataTimeStamp & end() const;
    GeoDataTimeStamp & end();

    /**
    * @brief Set the ending instant of a timespan
    * @param begin the ending instant of a timespan
    */
    void setEnd( const GeoDataTimeStamp& end );

    /**
     * @return True iff either of begin or end is valid, or if begin and end are both valid and begin is <= end
     */
    bool isValid() const;

  private:
    GeoDataTimeSpanPrivate * const d;
};

}

#endif //MARBLE_GEODATATIMESPAN_H
