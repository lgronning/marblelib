//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013      Mayank Madan <maddiemadan@gmail.com>
// Copyright 2013      Sanjiban Bairagya <sanjiban22393@gmail.com>
//

#ifndef GEODATAMODEL_H
#define GEODATAMODEL_H

#include "geodata/data/GeoDataGeometry.h"
#include "MarbleGlobal.h"
#include "geodata/data/GeoDataLink.h"
#include "geodata/data/GeoDataScale.h"
#include "geodata/data/GeoDataOrientation.h"
#include "geodata/data/GeoDataCoordinates.h"
#include "geodata/data/GeoDataLocation.h"
#include "geodata/data/GeoDataResourceMap.h"
#include "geodata/data/GeoDataAlias.h"

namespace Marble {

class GeoDataModelPrivate;

class MARBLE_EXPORT GeoDataModel: public GeoDataGeometry
{
public:
    GeoDataModel();

    GeoDataModel( const GeoDataModel &other );

    GeoDataModel& operator=( const GeoDataModel &other );

    bool operator==( const GeoDataModel &other ) const;
    bool operator!=( const GeoDataModel &other ) const;

    ~GeoDataModel() override;

    const GeoDataLink& link() const;
    GeoDataLink& link();
    void setLink( const GeoDataLink &link );

    const GeoDataCoordinates& coordinates() const;
    GeoDataCoordinates& coordinates();
    void setCoordinates(const GeoDataCoordinates &coordinates);

    const GeoDataScale& scale() const;
    GeoDataScale& scale();
    void setScale(const GeoDataScale &scale);

    const GeoDataOrientation& orientation() const;
    GeoDataOrientation& orientation();
    void setOrientation( const GeoDataOrientation &orientation);

    const GeoDataLocation& location() const;
    GeoDataLocation& location();
    void setLocation( const GeoDataLocation &location);

    const GeoDataResourceMap& resourceMap() const;
    GeoDataResourceMap& resourceMap();
    void setResourceMap( const GeoDataResourceMap &map);

    QString sourceHref() const;
    QString targetHref() const;
    void setSourceHref(const QString &sourceHref);
    void setTargetHref(const QString &targetHref);

private:
    GeoDataModelPrivate *p() const;
};

}


#endif // GEODATAMODEL_H