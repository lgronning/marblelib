//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010,2011 Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

// Own
#include "ServerLayout.h"

#include "geodata/scene/GeoSceneTileDataset.h"
#include "MarbleGlobal.h"
#include "TileId.h"

#include <QUrlQuery>
#include <QtCore/QDebug>

#include <math.h>

namespace Marble
{

ServerLayout::ServerLayout( GeoSceneTileDataset *textureLayer )
    : m_textureLayer( textureLayer )
{
}

ServerLayout::~ServerLayout()
= default;

MarbleServerLayout::MarbleServerLayout( GeoSceneTileDataset *textureLayer )
    : ServerLayout( textureLayer )
{
}

QUrl MarbleServerLayout::downloadUrl( const QUrl &prototypeUrl, const TileId &id ) const
{
    const QString path = QStringLiteral( "%1/%2/%3/%3_%4.%5" )
        .arg( prototypeUrl.path() )
        .arg( id.tileLevel() )
        .arg( id.y(), tileDigits, 10, QChar('0') )
        .arg( id.x(), tileDigits, 10, QChar('0') )
        .arg( m_textureLayer->fileFormat().toLower() );

    QUrl url = prototypeUrl;
    url.setPath( path );

    return url;
}

QString MarbleServerLayout::name() const
{
    return QStringLiteral("Marble");
}

QString ServerLayout::sourceDir() const
{
    return m_textureLayer ? m_textureLayer->sourceDir() : QString();
}


OsmServerLayout::OsmServerLayout( GeoSceneTileDataset *textureLayer )
    : ServerLayout( textureLayer )
{
}

QUrl OsmServerLayout::downloadUrl( const QUrl &prototypeUrl, const TileId &id ) const
{
    const QString suffix = m_textureLayer->fileFormat().toLower();
    const QString path = QStringLiteral( "%1/%2/%3.%4" ).arg( id.tileLevel() )
                                                 .arg( id.x() )
                                                 .arg( id.y() )
                                                 .arg( suffix );

    QUrl url = prototypeUrl;
    url.setPath( url.path() + path );

    return url;
}

QString OsmServerLayout::name() const
{
    return QStringLiteral("OpenStreetMap");
}


CustomServerLayout::CustomServerLayout( GeoSceneTileDataset *texture )
    : ServerLayout( texture )
{
}

QUrl CustomServerLayout::downloadUrl( const QUrl &prototypeUrl, const TileId &id ) const
{
    const GeoDataLatLonBox bbox = id.toLatLonBox( m_textureLayer );

    qWarning() << "prototypeUrl" << prototypeUrl;
    QString urlStr = prototypeUrl.toString( QUrl::DecodeReserved );
    qWarning() << "urlStr" << urlStr;

    urlStr.replace( QLatin1String("{zoomLevel}"), QString::number( id.tileLevel() ) );
    urlStr.replace( QLatin1String("{x}"), QString::number( id.x() ) );
    urlStr.replace( QLatin1String("{y}"), QString::number( id.y() ) );
    urlStr.replace( QLatin1String("{west}"), QString::number( bbox.west( GeoDataCoordinates::Degree ), 'f', 12 ) );
    urlStr.replace( QLatin1String("{south}"), QString::number( bbox.south( GeoDataCoordinates::Degree ), 'f', 12 ) );
    urlStr.replace( QLatin1String("{east}"), QString::number( bbox.east( GeoDataCoordinates::Degree ), 'f', 12 ) );
    urlStr.replace( QLatin1String("{north}"), QString::number( bbox.north( GeoDataCoordinates::Degree ), 'f', 12 ) );
    urlStr.replace( QLatin1String("%3F"), QLatin1String("?") );

    return QUrl( urlStr );
}

QString CustomServerLayout::name() const
{
    return QStringLiteral("Custom");
}


WmsServerLayout::WmsServerLayout( GeoSceneTileDataset *texture )
    : ServerLayout( texture )
{
    qWarning() << "WmsServerLayout::WmsServerLayout";
}

QUrl WmsServerLayout::downloadUrl( const QUrl &prototypeUrl, const Marble::TileId &tileId ) const
{
    GeoDataLatLonBox box = tileId.toLatLonBox( m_textureLayer );

    QUrlQuery url(prototypeUrl.query());
    url.addQueryItem( QStringLiteral("service"), QStringLiteral("WMS") );
    url.addQueryItem( QStringLiteral("request"), QStringLiteral("GetMap") );
    url.addQueryItem( QStringLiteral("version"), QStringLiteral("1.1.1") );
    if ( !url.hasQueryItem( QStringLiteral("styles") ) )
        url.addQueryItem( QStringLiteral("styles"), QLatin1String("") );
    if ( !url.hasQueryItem( QStringLiteral("format") ) ) {
        if ( m_textureLayer->fileFormat().toLower() == QLatin1String("jpg") )
            url.addQueryItem( QStringLiteral("format"), QStringLiteral("image/jpeg") );
        else if ( m_textureLayer->fileFormat().toLower() == QLatin1String("png") )
            url.addQueryItem( QStringLiteral("format"), QStringLiteral("image/png") );
        else
            url.addQueryItem( QStringLiteral("format"), "image/" + m_textureLayer->fileFormat().toLower() );
    }
    if ( !url.hasQueryItem( QStringLiteral("srs") ) ) {
        url.addQueryItem( QStringLiteral("srs"), epsgCode() );
    }
    if ( !url.hasQueryItem( QStringLiteral("layers") ) )
        url.addQueryItem( QStringLiteral("layers"), m_textureLayer->name() );
    url.addQueryItem( QStringLiteral("width"), QString::number( m_textureLayer->tileSize().width() ) );
    url.addQueryItem( QStringLiteral("height"), QString::number( m_textureLayer->tileSize().height() ) );
    url.addQueryItem( QStringLiteral("bbox"), QStringLiteral( "%1,%2,%3,%4" ).arg( QString::number( box.west( GeoDataCoordinates::Degree ), 'f', 12 ) )
                                                      .arg( QString::number( box.south( GeoDataCoordinates::Degree ), 'f', 12 ) )
                                                      .arg( QString::number( box.east( GeoDataCoordinates::Degree ), 'f', 12 ) )
                                                      .arg( QString::number( box.north( GeoDataCoordinates::Degree ), 'f', 12 ) ) );
    QUrl finalUrl = prototypeUrl;
    finalUrl.setQuery(url);

    qWarning() << "finalUrl" << finalUrl;
    return finalUrl;
}

QString WmsServerLayout::name() const
{
    return QStringLiteral("WebMapService");
}

QString WmsServerLayout::epsgCode() const
{
    switch ( m_textureLayer->projection() ) {
        case GeoSceneTileDataset::Equirectangular:
            return QStringLiteral("EPSG:4326");
        case GeoSceneTileDataset::Mercator:
            return QStringLiteral("EPSG:3785");
    }

    Q_ASSERT( false ); // not reached
    return QString();
}

QuadTreeServerLayout::QuadTreeServerLayout( GeoSceneTileDataset *textureLayer )
    : ServerLayout( textureLayer )
{
}

QUrl QuadTreeServerLayout::downloadUrl( const QUrl &prototypeUrl, const Marble::TileId &id ) const
{
    QString urlStr = prototypeUrl.toString( QUrl::DecodeReserved );

    urlStr.replace( QLatin1String("{quadIndex}"), encodeQuadTree( id ) );

    return QUrl( urlStr );
}

QString QuadTreeServerLayout::name() const
{
    return QStringLiteral("QuadTree");
}

QString QuadTreeServerLayout::encodeQuadTree( const Marble::TileId &id )
{
    QString tileNum;

    for ( int i = id.tileLevel(); i >= 0; i-- ) {
        const int tileX = (id.x() >> i) % 2;
        const int tileY = (id.y() >> i) % 2;
        const int num = ( 2 * tileY ) + tileX;

        tileNum += QString::number( num );
    }

    return tileNum;
}

TmsServerLayout::TmsServerLayout(GeoSceneTileDataset *textureLayer )
    : ServerLayout( textureLayer )
{
}

QUrl TmsServerLayout::downloadUrl( const QUrl &prototypeUrl, const TileId &id ) const
{
    const QString suffix = m_textureLayer->fileFormat().toLower();
    // y coordinate in TMS start at the bottom of the map (South) and go upwards,
    // opposed to OSM which start at the top.
    //
    // http://wiki.osgeo.org/wiki/Tile_Map_Service_Specification
    int y_frombottom = ( 1<<id.tileLevel() ) - id.y() - 1 ;

    const QString path = QStringLiteral( "%1/%2/%3.%4" ).arg( id.tileLevel() )
                                                 .arg( id.x() )
                                                 .arg( y_frombottom )
                                                 .arg( suffix );
    QUrl url = prototypeUrl;
    url.setPath( url.path() + path );

    return url;
}

QString TmsServerLayout::name() const
{
    return QStringLiteral("TileMapService");
}

}
