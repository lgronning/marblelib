//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010      Harshit Jain <hjain.itbhu@gmail.com>
//

#ifndef MARBLE_GEODATATIMESTAMP_H
#define MARBLE_GEODATATIMESTAMP_H

#include <QString>
#include <QDateTime>

#include "geodata/data/GeoDataObject.h"
#include "geodata/data/GeoDataTimePrimitive.h"

#include "geodata/geodata_export.h"

namespace Marble
{

class GeoDataTimeStampPrivate;

class GEODATA_EXPORT GeoDataTimeStamp : public GeoDataTimePrimitive
{
  public:
    enum TimeResolution {
        SecondResolution,
        DayResolution,
        MonthResolution,
        YearResolution
    };

    GeoDataTimeStamp();
    GeoDataTimeStamp( const GeoDataTimeStamp& other );
    ~GeoDataTimeStamp() override;

    /**
    * @brief assignment operator
    */
    GeoDataTimeStamp& operator=( const GeoDataTimeStamp& other );

    /**
     * @brief equality operators
     */
    bool operator==( const GeoDataTimeStamp& other ) const;
    bool operator!=( const GeoDataTimeStamp& other ) const;


    /// Provides type information for downcasting a GeoNode
    const char* nodeType() const override;

    /**
    * @brief return the when time of timestamp
    */
    QDateTime when() const;
 
    /**
    * @brief Set the when time of timestamp
    * @param when the when time of timestamp
    */
    void setWhen( const QDateTime& when );
    
    void setResolution( TimeResolution resolution );

    TimeResolution resolution() const;

  private:
    GeoDataTimeStampPrivate * const d;
};

}

#endif
