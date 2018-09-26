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

// Own
#include "TileId.h"

#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include <QtCore/QtMath>

namespace Marble
{

TileId::TileId( QString const & mapThemeId, int zoomLevel, int tileX, int tileY )
    : m_mapThemeIdHash( qHash( mapThemeId )), m_tileLevel( zoomLevel ), m_tileX( tileX ), m_tileY( tileY )
{
}

TileId::TileId( uint mapThemeIdHash, int zoomLevel, int tileX, int tileY )
    : m_mapThemeIdHash( mapThemeIdHash ), m_tileLevel( zoomLevel ), m_tileX( tileX ), m_tileY( tileY )
{
}

TileId::TileId()
    : m_mapThemeIdHash( 0 ), m_tileLevel( 0 ), m_tileX( 0 ), m_tileY( 0 )
{
}

GeoDataLatLonBox TileId::toLatLonBox( const GeoSceneTileDataset *textureLayer ) const
{

    qreal radius = ( 1 << tileLevel() ) * textureLayer->levelZeroColumns() / 2.0;

    qreal lonLeft   = ( x() - radius ) / radius * M_PI;
    qreal lonRight  = ( x() - radius + 1 ) / radius * M_PI;

    radius = ( 1 << tileLevel() ) * textureLayer->levelZeroRows() / 2.0;
    qreal latTop = 0;
    qreal latBottom = 0;

    switch ( textureLayer->projection() ) {
    case GeoSceneTileDataset::Equirectangular:
        latTop = ( radius - y() ) / radius *  M_PI / 2.0;
        latBottom = ( radius - y() - 1 ) / radius *  M_PI / 2.0;
        break;
    case GeoSceneTileDataset::Mercator:
        latTop = atan( sinh( ( radius - y() ) / radius * M_PI ) );
        latBottom = atan( sinh( ( radius - y() - 1 ) / radius * M_PI ) );
        break;
    }

    return GeoDataLatLonBox( latTop, latBottom, lonRight, lonLeft );
}

namespace
{
int lon2tileX( qreal lon, int maxTileX )
{
    return qFloor((lon + M_PI) / (2*M_PI) * maxTileX);
}

int lat2tileY( qreal lat, int maxTileY )
{
    return qFloor((1.0 - log( tan(lat) + 1.0 / cos(lat)) / M_PI) / 2.0 * maxTileY);
}

}

TileId TileId::fromCoordinates(const GeoSceneTileDataset *textureLayer, const GeoDataCoordinates &coords, int zoomLevel)
{
    const int maxTileX = ( 1 << zoomLevel ) * textureLayer->levelZeroColumns();
    const int maxTileY = ( 1 << zoomLevel ) * textureLayer->levelZeroRows();

    // New tiles X and Y for moved screen coordinates
    // More info: http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames#Subtiles
    // More info: http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames#C.2FC.2B.2B
    // Sometimes the formula returns wrong huge values, x and y have to be between 0 and 2^ZoomLevel

    int x = 0;
    if(qFuzzyCompare(coords.longitude(), M_PI) || coords.longitude() > M_PI)
    {
        x = maxTileX;
    }
    else if(qFuzzyCompare(coords.longitude(), -M_PI) || coords.longitude() < -M_PI)
    {
        x = 0;
    }
    else
    {
        x = qMin<int>( maxTileX, qMax<int>( lon2tileX( coords.longitude(GeoDataCoordinates::Radian), maxTileX ), 0 ) );
    }

    int y = 0;
    if(qFuzzyCompare(coords.latitude(), M_PI/2.0) || coords.latitude() > M_PI/2.0)
    {
        y = 0;
    }
    else if(qFuzzyCompare(coords.latitude(), -M_PI/2.0) || coords.latitude() < -M_PI/2.0)
    {
        y = maxTileY;
    }
    else
    {
        y = qMin<int>( maxTileY, qMax< int>( lat2tileY( coords.latitude(GeoDataCoordinates::Radian), maxTileY ), 0 ) );
    }

    return TileId(0, zoomLevel, x, y);
}

}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<( QDebug dbg, const Marble::TileId &id )
{
    return dbg << QStringLiteral( "Marble::TileId(%1, %2, %3, %4)" ).arg( id.mapThemeIdHash() )
                                                             .arg( id.tileLevel() )
                                                             .arg( id.x() )
                                                             .arg( id.y() );
}
#endif
