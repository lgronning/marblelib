//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008      Patrick Spendrin <ps_ml@gmx.de>
//

#include "FileLoader.h"

#include <QBuffer>
#include <QDataStream>
#include <QDateTime>
#include <QFile>
#include <QThread>

#include "geodata/data/GeoDataDocument.h"
#include "geodata/data/GeoDataFolder.h"
#include "geodata/data/GeoDataPlacemark.h"
#include "geodata/data/GeoDataData.h"
#include "geodata/data/GeoDataExtendedData.h"
#include "geodata/data/GeoDataStyleMap.h"
#include "geodata/data/GeoDataPolyStyle.h"
#include "geodata/data/GeoDataLineStyle.h"
#include "geodata/data/GeoDataStyle.h"
#include "geodata/parser/GeoDataTypes.h"
#include "MarbleDirs.h"
#include "MarbleDebug.h"
#include "MarbleModel.h"

namespace Marble
{



FileLoader::FileLoader( QObject* parent, const PluginManager *pluginManager, bool recenter,
                       const QString& file, const QString& property, const GeoDataStyle::Ptr &style, DocumentRole role )
    : QThread( parent ),
      d( new FileLoaderPrivate( this, pluginManager, recenter, file, property, style, role ) )
{
}

FileLoader::FileLoader( QObject* parent, const PluginManager *pluginManager,
                        const QString& contents, const QString& file, DocumentRole role )
    : QThread( parent ),
      d( new FileLoaderPrivate( this, pluginManager, contents, file, role ) )
{
}

FileLoader::~FileLoader()
{
    delete d;
}

QString FileLoader::path() const
{
    return d->m_filepath;
}

GeoDataDocument* FileLoader::document()
{
    return d->m_document;
}

QString FileLoader::error() const
{
    return d->m_error;
}

void FileLoader::run()
{
    if ( d->m_contents.isEmpty() ) {
        QString defaultSourceName;

        mDebug() << "starting parser for" << d->m_filepath;

        QFileInfo fileinfo( d->m_filepath );
        QString path = fileinfo.path();
        if ( path == QLatin1String(".") ) path.clear();
        QString name = fileinfo.completeBaseName();
        QString suffix = fileinfo.suffix();

        // determine source, cache names
        if ( fileinfo.isAbsolute() ) {
            // We got an _absolute_ path now: e.g. "/patrick.kml"
            defaultSourceName   = path + '/' + name + '.' + suffix;
        }
        else if ( d->m_filepath.contains( '/' ) ) {
            // _relative_ path: "maps/mars/viking/patrick.kml"
            defaultSourceName   = MarbleDirs::path( path + '/' + name + '.' + suffix );
            if ( !QFile::exists( defaultSourceName ) ) {
                defaultSourceName = MarbleDirs::path( path + '/' + name + ".cache" );
            }
        }
        else {
            // _standard_ shared placemarks: "placemarks/patrick.kml"
            defaultSourceName   = MarbleDirs::path( "placemarks/" + path + name + '.' + suffix );
            if ( !QFile::exists( defaultSourceName ) ) {
                defaultSourceName = MarbleDirs::path( "placemarks/" + path + name + ".cache" );
            }
        }

        if ( QFile::exists( defaultSourceName ) ) {
            mDebug() << "No recent Default Placemark Cache File available!";

            // use runners: pnt, gpx, osm
            connect( &d->m_runner, SIGNAL(parsingFinished(GeoDataDocument*,QString)),
                    this, SLOT(documentParsed(GeoDataDocument*,QString)) );
            d->m_runner.parseFile( defaultSourceName, d->m_documentRole );
        }
        else {
            mDebug() << "No Default Placemark Source File for " << name;
        }
    // content is not empty, we load from data
    }
}

bool FileLoader::recenter() const
{
    return d->m_recenter;
}

void FileLoaderPrivate::documentParsed( GeoDataDocument* doc, const QString& error )
{
    m_error = error;
    if ( doc ) {
        m_document = doc;
        doc->setProperty( m_property );
        if( m_style ) {
            doc->addStyleMap( *m_styleMap );
            doc->addStyle( m_style );
        }

        createFilterProperties( doc );
        emit q->newGeoDataDocumentAdded( m_document );
    }
    emit q->loaderFinished( q );
}

FileLoaderPrivate::FileLoaderPrivate(FileLoader *parent, const PluginManager *pluginManager, bool recenter, QString file, QString property, GeoDataStyle::Ptr style, DocumentRole role)
    : q( parent),
      m_runner( pluginManager ),
      m_recenter( recenter ),
      m_filepath (std::move( file )),
      m_property(std::move( property )),
      m_style(std::move( style )),
      m_documentRole ( role ),
      m_styleMap( new GeoDataStyleMap ),
      m_document( nullptr )
{
    if( m_style ) {
        m_styleMap->setId(QStringLiteral("default-map"));
        m_styleMap->insert(QStringLiteral("normal"), QStringLiteral("#").append(m_style->id()));
    }
}

FileLoaderPrivate::FileLoaderPrivate(FileLoader *parent, const PluginManager *pluginManager, QString contents, QString file, DocumentRole role)
    : q( parent ),
      m_runner( pluginManager ),
      m_recenter( false ),
      m_filepath (std::move( file )),
      m_contents (std::move( contents )),
      m_documentRole ( role ),
      m_styleMap( nullptr ),
      m_document( nullptr )
{
}

FileLoaderPrivate::~FileLoaderPrivate()
{
    delete m_styleMap;
}

void FileLoaderPrivate::createFilterProperties( GeoDataContainer *container )
{
    QVector<GeoDataFeature*>::Iterator i = container->begin();
    QVector<GeoDataFeature*>::Iterator const end = container->end();
    for (; i != end; ++i ) {
        if ( (*i)->nodeType() == GeoDataTypes::GeoDataFolderType
             || (*i)->nodeType() == GeoDataTypes::GeoDataDocumentType ) {
            GeoDataContainer *child = static_cast<GeoDataContainer*>( *i );
            createFilterProperties( child );
        } else if ( (*i)->nodeType() == GeoDataTypes::GeoDataPlacemarkType ) {
            Q_ASSERT( dynamic_cast<GeoDataPlacemark*>( *i ) );

            GeoDataPlacemark* placemark = static_cast<GeoDataPlacemark*>( *i );
            Q_ASSERT( placemark->geometry() );

            bool hasPopularity = false;

            if ( placemark->geometry()->nodeType() != GeoDataTypes::GeoDataPointType
                 && m_documentRole == MapDocument
                 && m_style ) {
                placemark->setStyleUrl( QStringLiteral("#").append( m_styleMap->id() ) );
            }

            // Mountain (H), Volcano (V), Shipwreck (W)
            if ( placemark->role() == QLatin1String("H") || placemark->role() == QLatin1String("V") || placemark->role() == QLatin1String("W") )
            {
                qreal altitude = placemark->coordinate().altitude();
                if ( altitude != 0.0 )
                {
                    hasPopularity = true;
                    placemark->setPopularity( (qint64)(altitude * 1000.0) );
                    placemark->setZoomLevel( cityPopIdx( qAbs( (qint64)(altitude * 1000.0) ) ) );
                }
            }
            // Continent (K), Ocean (O), Nation (S)
            else if ( placemark->role() == QLatin1String("K") || placemark->role() == QLatin1String("O") || placemark->role() == QLatin1String("S") )
            {
                qreal area = placemark->area();
                if ( area >= 0.0 )
                {
                    hasPopularity = true;
                    //                mDebug() << placemark->name() << " " << (qint64)(area);
                    placemark->setPopularity( (qint64)(area * 100) );
                    placemark->setZoomLevel( areaPopIdx( area ) );
                }
            }
            // Pole (P)
            else if ( placemark->role() == QLatin1String("P") )
            {
                placemark->setPopularity( 1000000000 );
                placemark->setZoomLevel( 1 );
            }
            // Magnetic Pole (M)
            else if ( placemark->role() == QLatin1String("M") )
            {
                placemark->setPopularity( 10000000 );
                placemark->setZoomLevel( 3 );
            }
            // MannedLandingSite (h)
            else if ( placemark->role() == QLatin1String("h") )
            {
                placemark->setPopularity( 1000000000 );
                placemark->setZoomLevel( 1 );
            }
            // RoboticRover (r)
            else if ( placemark->role() == QLatin1String("r") )
            {
                placemark->setPopularity( 10000000 );
                placemark->setZoomLevel( 2 );
            }
            // UnmannedSoftLandingSite (u)
            else if ( placemark->role() == QLatin1String("u") )
            {
                placemark->setPopularity( 1000000 );
                placemark->setZoomLevel( 3 );
            }
            // UnmannedSoftLandingSite (i)
            else if ( placemark->role() == QLatin1String("i") )
            {
                placemark->setPopularity( 1000000 );
                placemark->setZoomLevel( 3 );
            }
            // Space Terrain: Craters, Maria, Montes, Valleys, etc.
            else if (    placemark->role() == QLatin1String("m") || placemark->role() == QLatin1String("v")
                         || placemark->role() == QLatin1String("o") || placemark->role() == QLatin1String("c")
                         || placemark->role() == QLatin1String("a") )
            {
                qint64 diameter = placemark->population();
                if ( diameter >= 0 )
                {
                    hasPopularity = true;
                    placemark->setPopularity( diameter );
                    if ( placemark->role() == QLatin1String("c") ) {
                        placemark->setZoomLevel( spacePopIdx( diameter ) );
                        if ( placemark->name() == QLatin1String("Tycho") || placemark->name() == QLatin1String("Copernicus") ) {
                            placemark->setZoomLevel( 1 );
                        }
                    }
                    else {
                        placemark->setZoomLevel( spacePopIdx( diameter ) );
                    }

                    if ( placemark->role() == QLatin1String("a") && diameter == 0 ) {
                        placemark->setPopularity( 1000000000 );
                        placemark->setZoomLevel( 1 );
                    }
                }
            }
            else
            {
                qint64 population = placemark->population();
                if ( population >= 0 )
                {
                    hasPopularity = true;
                    placemark->setPopularity( population );
                    placemark->setZoomLevel( cityPopIdx( population ) );
                }
            }

            //  Then we set the visual category:

            if ( placemark->role() == QLatin1String("H") )      placemark->setVisualCategory( GeoDataPlacemark::Mountain );
            else if ( placemark->role() == QLatin1String("V") ) placemark->setVisualCategory( GeoDataPlacemark::Volcano );

            else if ( placemark->role() == QLatin1String("m") ) placemark->setVisualCategory( GeoDataPlacemark::Mons );
            else if ( placemark->role() == QLatin1String("v") ) placemark->setVisualCategory( GeoDataPlacemark::Valley );
            else if ( placemark->role() == QLatin1String("o") ) placemark->setVisualCategory( GeoDataPlacemark::OtherTerrain );
            else if ( placemark->role() == QLatin1String("c") ) placemark->setVisualCategory( GeoDataPlacemark::Crater );
            else if ( placemark->role() == QLatin1String("a") ) placemark->setVisualCategory( GeoDataPlacemark::Mare );

            else if ( placemark->role() == QLatin1String("P") ) placemark->setVisualCategory( GeoDataPlacemark::GeographicPole );
            else if ( placemark->role() == QLatin1String("M") ) placemark->setVisualCategory( GeoDataPlacemark::MagneticPole );
            else if ( placemark->role() == QLatin1String("W") ) placemark->setVisualCategory( GeoDataPlacemark::ShipWreck );
            else if ( placemark->role() == QLatin1String("F") ) placemark->setVisualCategory( GeoDataPlacemark::AirPort );
            else if ( placemark->role() == QLatin1String("A") ) placemark->setVisualCategory( GeoDataPlacemark::Observatory );
            else if ( placemark->role() == QLatin1String("K") ) placemark->setVisualCategory( GeoDataPlacemark::Continent );
            else if ( placemark->role() == QLatin1String("O") ) placemark->setVisualCategory( GeoDataPlacemark::Ocean );
            else if ( placemark->role() == QLatin1String("S") ) placemark->setVisualCategory( GeoDataPlacemark::Nation );
            else
                if (  placemark->role()==QLatin1String("PPL")
                   || placemark->role()==QLatin1String("PPLF")
                   || placemark->role()==QLatin1String("PPLG")
                   || placemark->role()==QLatin1String("PPLL")
                   || placemark->role()==QLatin1String("PPLQ")
                   || placemark->role()==QLatin1String("PPLR")
                   || placemark->role()==QLatin1String("PPLS")
                   || placemark->role()==QLatin1String("PPLW") ) placemark->setVisualCategory(
                        ( GeoDataPlacemark::GeoDataVisualCategory )( GeoDataPlacemark::SmallCity
                                                                       + (( 20- ( 2*placemark->zoomLevel()) ) / 4 * 4 ) ) );
            else if ( placemark->role() == QLatin1String("PPLA") ) placemark->setVisualCategory(
                    ( GeoDataPlacemark::GeoDataVisualCategory )( GeoDataPlacemark::SmallStateCapital
                                                                   + (( 20- ( 2*placemark->zoomLevel()) ) / 4 * 4 ) ) );
            else if ( placemark->role()==QLatin1String("PPLC") ) placemark->setVisualCategory(
                    ( GeoDataPlacemark::GeoDataVisualCategory )( GeoDataPlacemark::SmallNationCapital
                                                                   + (( 20- ( 2*placemark->zoomLevel()) ) / 4 * 4 ) ) );
            else if ( placemark->role()==QLatin1String("PPLA2") || placemark->role()==QLatin1String("PPLA3") ) placemark->setVisualCategory(
                    ( GeoDataPlacemark::GeoDataVisualCategory )( GeoDataPlacemark::SmallCountyCapital
                                                                   + (( 20- ( 2*placemark->zoomLevel()) ) / 4 * 4 ) ) );
            else if ( placemark->role()==QLatin1String(" ") && !hasPopularity && placemark->visualCategory() == GeoDataPlacemark::Unknown ) {
                placemark->setVisualCategory( GeoDataPlacemark::Unknown ); // default location
                placemark->setZoomLevel(0);
            }
            else if ( placemark->role() == QLatin1String("h") ) placemark->setVisualCategory( GeoDataPlacemark::MannedLandingSite );
            else if ( placemark->role() == QLatin1String("r") ) placemark->setVisualCategory( GeoDataPlacemark::RoboticRover );
            else if ( placemark->role() == QLatin1String("u") ) placemark->setVisualCategory( GeoDataPlacemark::UnmannedSoftLandingSite );
            else if ( placemark->role() == QLatin1String("i") ) placemark->setVisualCategory( GeoDataPlacemark::UnmannedHardLandingSite );

            if ( placemark->role() == QLatin1String("W") && placemark->zoomLevel() < 4 )
                placemark->setZoomLevel( 4 );
            if ( placemark->role() == QLatin1String("O") )
                placemark->setZoomLevel( 2 );
            if ( placemark->role() == QLatin1String("K") )
                placemark->setZoomLevel( 0 );
            if ( !placemark->isVisible() ) {
                placemark->setZoomLevel( 18 );
            }
            // Workaround: Emulate missing "setVisible" serialization by allowing for population
            // values smaller than -1 which are considered invisible.
            if ( placemark->population() < -1 ) {
                placemark->setZoomLevel( 18 );
            }
        } else {
            qWarning() << Q_FUNC_INFO << "Unknown feature" << (*i)->nodeType() << ". Skipping.";
        }
    }
}

int FileLoaderPrivate::cityPopIdx( qint64 population )
{
    int popidx = 3;

    if ( population < 2500 )        popidx=10;
    else if ( population < 5000)    popidx=9;
    else if ( population < 25000)   popidx=8;
    else if ( population < 75000)   popidx=7;
    else if ( population < 250000)  popidx=6;
    else if ( population < 750000)  popidx=5;
    else if ( population < 2500000) popidx=4;

    return popidx;
}

int FileLoaderPrivate::spacePopIdx( qint64 population )
{
    int popidx = 1;

    if ( population < 1000 )        popidx=10;
    else if ( population < 2000)    popidx=9;
    else if ( population < 8000)    popidx=8;
    else if ( population < 20000)   popidx=7;
    else if ( population < 60000)    popidx=6;
    else if ( population < 100000)   popidx=5;
    else if ( population < 200000 )  popidx=4;
    else if ( population < 400000 )  popidx=2;
    else if ( population < 600000 )  popidx=1;

    return popidx;
}

int FileLoaderPrivate::areaPopIdx( qreal area )
{
    int popidx = 1;
    if      ( area <  200000  )      popidx=5;
    else if ( area < 1000000  )      popidx=4;
    else if ( area < 2500000  )      popidx=3;
    else if ( area < 5000000  )      popidx=2;

    return popidx;
}



//#include "moc_FileLoader.cpp"
} // namespace Marble
