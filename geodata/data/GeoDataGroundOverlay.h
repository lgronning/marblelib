//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2012      Dennis Nienhüser <nienhueser@kde.org>
//


#ifndef MARBLE_GEODATAGROUNDOVERLAY_H
#define MARBLE_GEODATAGROUNDOVERLAY_H

#include "geodata/data/GeoDataOverlay.h"
#include "MarbleGlobal.h"
#include "geodata/data/GeoDataLatLonBox.h"
#include "geodata/data/GeoDataLatLonQuad.h"

namespace Marble {

class GeoDataGroundOverlayPrivate;

class MARBLE_EXPORT GeoDataGroundOverlay: public GeoDataOverlay
{
public:
    GeoDataGroundOverlay();

    GeoDataGroundOverlay( const GeoDataGroundOverlay &other );

    GeoDataGroundOverlay& operator=( const GeoDataGroundOverlay &other );
    bool operator==( const GeoDataGroundOverlay &other ) const;
    bool operator!=( const GeoDataGroundOverlay &other ) const;
    ~GeoDataGroundOverlay() override;

    /** Provides type information for downcasting a GeoNode */
    const char* nodeType() const override;

    double altitude() const;

    void setAltitude( double altitude );

    AltitudeMode altitudeMode() const;

    void setAltitudeMode( const AltitudeMode altitudeMode );

    GeoDataLatLonBox& latLonBox() const;

    void setLatLonBox( const GeoDataLatLonBox &box );

private:
    GeoDataGroundOverlayPrivate* const d;
};

}

#endif
