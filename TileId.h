//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008, 2010 Jens-Michael Hoffmann <jensmh@gmx.de>
// Copyright 2012       Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#ifndef MARBLE_TILEID_H
#define MARBLE_TILEID_H

#include <QHash>
#include <QString>

#include "marble_export.h"
#include "geodata/data/GeoDataCoordinates.h"
#include "geodata/data/GeoDataLatLonBox.h"
#include "geodata/scene/GeoSceneTextureTileDataset.h"

namespace Marble
{
class GeoDataCoordinates;

class MARBLE_EXPORT TileId
{
 public:
    TileId( QString const & mapThemeId, int tileLevel, int tileX, int tileY );
    TileId( uint mapThemeIdHash, int tileLevel, int tileX, int tileY );
    TileId();

    int tileLevel() const;
    int x() const;
    int y() const;
    uint mapThemeIdHash() const;

    bool operator==( TileId const& rhs ) const;
    bool operator<( TileId const& rhs ) const;

    GeoDataLatLonBox toLatLonBox( const GeoSceneTileDataset *textureLayer ) const;
    static TileId fromCoordinates( const GeoSceneTileDataset *textureLayer, const GeoDataCoordinates& coords, int popularity );

 private:
    uint m_mapThemeIdHash;
    int m_tileLevel;
    int m_tileX;
    int m_tileY;
};

uint qHash( TileId const& );


// inline definitions

inline int TileId::tileLevel() const
{
    return m_tileLevel;
}

inline int TileId::x() const
{
    return m_tileX;
}

inline int TileId::y() const
{
    return m_tileY;
}

inline uint TileId::mapThemeIdHash() const
{
    return m_mapThemeIdHash;
}

inline bool TileId::operator==( TileId const& rhs ) const
{
    return m_tileLevel == rhs.m_tileLevel
        && m_tileX == rhs.m_tileX
        && m_tileY == rhs.m_tileY
        && m_mapThemeIdHash == rhs.m_mapThemeIdHash;
}

inline bool TileId::operator<( TileId const& rhs ) const
{
    if (m_tileLevel < rhs.m_tileLevel)
        return true;
    else if (m_tileLevel == rhs.m_tileLevel
             && m_tileX < rhs.m_tileX)
        return true;
    else if (m_tileLevel == rhs.m_tileLevel
             && m_tileX == rhs.m_tileX
             && m_tileY < rhs.m_tileY)
        return true;
    else if (m_tileLevel == rhs.m_tileLevel
             && m_tileX == rhs.m_tileX
             && m_tileY == rhs.m_tileY
             && m_mapThemeIdHash < rhs.m_mapThemeIdHash)
        return true;
    return false;
}

inline uint qHash( TileId const& tid )
{
    const quint64 tmp = (( quint64 )( tid.tileLevel() ) << 36 )
        + (( quint64 )( tid.x() ) << 18 )
        + ( quint64 )( tid.y() );
    return ::qHash( tmp ) ^ tid.mapThemeIdHash();
}

}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<( QDebug, const Marble::TileId & );
#endif

#endif
