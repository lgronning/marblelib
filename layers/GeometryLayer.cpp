#define QT_NO_DEBUG_OUTPUT
//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008-2009      Patrick Spendrin  <ps_ml@gmx.de>
// Copyright 2010           Thibaut Gridel <tgridel@free.fr>
// Copyright 2011-2012      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
// Copyright 2014           Gábor Péterffy   <peterffy95@gmail.com>
//

#include "GeometryLayer.h"

// Marble
#include "SphericalScanlineTextureMapper.h"
#include "EquirectScanlineTextureMapper.h"
#include "MercatorScanlineTextureMapper.h"
#include "GenericScanlineTextureMapper.h"
#include "TileScalingTextureMapper.h"

#include "geodata/graphicsitem/GeoLabelPlaceHandler.h"
#include "geodata/data/GeoDataDocument.h"
#include "geodata/data/GeoDataFolder.h"
#include "geodata/data/GeoDataLineStyle.h"
#include "geodata/data/GeoDataLineStyle.h"
#include "geodata/data/GeoDataObject.h"
#include "geodata/data/GeoDataPlacemark.h"
#include "geodata/data/GeoDataPolygon.h"
#include "geodata/data/GeoDataMultiPolygon.h"
#include "geodata/data/GeoDataMultiPoint.h"
#include "geodata/data/GeoDataPolyStyle.h"
#include "geodata/data/GeoDataStyle.h"
#include "geodata/data/GeoDataStyleMap.h"
#include "geodata/data/GeoDataMultiLineString.h"
#include "geodata/data/GeoDataFeature.h"
#include "geodata/data/GeoDataScreenOverlay.h"
#include "geodata/parser/GeoDataTypes.h"
#include "MarbleDebug.h"
#include "MarbleMath.h"
#include "GeoPainter.h"
#include "ViewportParams.h"
#include "GeoGraphicsScene.h"
#include "ImageF.h"
#include "StackedTileLoader.h"
#include "VectorTileLoader.h"
#include "graphicsview/GeoGraphicsItem.h"
#include "TileId.h"
#include "graphicsview/MarbleGraphicsItem.h"
#include "MarblePlacemarkModel.h"
#include "GeoDataTreeModel.h"
#include "TextureMapperInterface.h"
#include <QtCore/QModelIndex>

#include "service/plot/GeometryHelper.h"
// Qt
#include <QtCore/qmath.h>
#include <QtGui/QGuiApplication>
#include <QtGui/QColor>
#include <QtCore/QTimer>
#include <QtCore/QModelIndex>
#include <QtCore/QAbstractItemModel>
#include <QtCore/QVector>
#include <QtCore/QElapsedTimer>
#include <QtCore/QRunnable>
#include <QtCore/QThreadPool>


namespace
{

const double EPSILON = 0.001;
const double PIXEL_EPSILON = 3;
const double MAX_SQR_DIFF_FOR_HIGHER_Z_LEVEL = 4.0;

QPointF
getPointRadian(const Marble::GeoDataCoordinates &coord)
{
    double lat;
    double lon;
    coord.geoCoordinates(lon, lat, Marble::GeoDataCoordinates::Radian);

    return QPointF(lon, lat);
}

double
distanceToLineSqr(const QVector<Marble::GeoDataLineString> &lineStrings, const Marble::GeoDataCoordinates &coordinate, double eps)
{
    QElapsedTimer timer;
    timer.start();

    QPointF pos = getPointRadian(coordinate);
    double prevPointDistance = 1.0e10;

    foreach(const Marble::GeoDataLineString &lineString, lineStrings)
    {
        int size = lineString.size();

        for(int i = 0; i < size; ++i)
        {
            QPointF v2 = getPointRadian(lineString.at(i));

            /* Just a quick check if it is wurth doing the calc */
            if(QPointF(v2-pos).manhattanLength() < eps)
            {
                double pointDistance = GeometryHelper::sqr( v2.x()-pos.x() ) + GeometryHelper::sqr( v2.y()-pos.y() );
                prevPointDistance = qMin(prevPointDistance, pointDistance);

                qDebug() << "distanceToLine" << pointDistance << v2.x() << v2.y() << pos.x() << pos.y() << (v2-pos);
            }


            if(i != 0)
            {
                QPointF v1 = getPointRadian(lineString.at(i-1));

                if (!( (v1.x() <= pos.x() && pos.x() <= v2.x()) || (v2.x() <= pos.x() && pos.x() <= v1.x()) ))
                {
                  // test point not in x-range
                  continue;
                }
                if (!( (v1.y() <= pos.y() && pos.y() <= v2.y()) || (v2.y() <= pos.y() && pos.y() <= v1.y()) ))
                {
                  // test point not in y-range
                    continue;
                }

                double prodToLine = GeometryHelper::perpDotProduct(v1, v2, pos);
                prevPointDistance = qMin(prodToLine, prevPointDistance);
            }
        }
    }

    qDebug() << "distanceToLineSqr elapsed" << timer.nsecsElapsed();

    return prevPointDistance;
}

double
sqrDistanceToGeometry(const Marble::GeoDataGeometry* geometry, const Marble::GeoDataCoordinates &clickedPoint, double eps, double epsSqr)
{
    qDebug() << "clickedPoint" << clickedPoint.latitude(Marble::GeoDataCoordinates::Degree)  << clickedPoint.longitude(Marble::GeoDataCoordinates::Degree) << eps;
    static const double MaxSqrPointDistance = 1.0e10;

    if(!geometry)
    {
        return MaxSqrPointDistance;
    }

    if(!geometry->latLonAltBox().contains(clickedPoint))
    {
        const Marble::GeoDataLatLonAltBox& latLonAltBox = geometry->latLonAltBox();
        if(!(clickedPoint.longitude(Marble::GeoDataCoordinates::Radian) > Marble::GeoDataCoordinates::normalizeLon(latLonAltBox.west(Marble::GeoDataCoordinates::Radian)-eps) &&
             clickedPoint.longitude(Marble::GeoDataCoordinates::Radian) < Marble::GeoDataCoordinates::normalizeLon(latLonAltBox.east(Marble::GeoDataCoordinates::Radian)+eps)))
        {
            qDebug() << "clickedPoint outside eps east west"
                       << clickedPoint.longitude(Marble::GeoDataCoordinates::Radian)
                       << eps
                       << Marble::GeoDataCoordinates::normalizeLon(latLonAltBox.west(Marble::GeoDataCoordinates::Radian)-eps)
                       << Marble::GeoDataCoordinates::normalizeLon(latLonAltBox.east(Marble::GeoDataCoordinates::Radian)+eps);

            return MaxSqrPointDistance;
        }

        if(!(clickedPoint.latitude(Marble::GeoDataCoordinates::Radian) > Marble::GeoDataCoordinates::normalizeLat(latLonAltBox.south(Marble::GeoDataCoordinates::Radian)-eps) &&
             clickedPoint.latitude(Marble::GeoDataCoordinates::Radian) < Marble::GeoDataCoordinates::normalizeLat(latLonAltBox.north(Marble::GeoDataCoordinates::Radian)+eps)))
        {
            qDebug() << "clickedPoint outside eps north south"
                       << clickedPoint.latitude(Marble::GeoDataCoordinates::Radian)
                       << eps
                       << Marble::GeoDataCoordinates::normalizeLat(latLonAltBox.south(Marble::GeoDataCoordinates::Radian)-eps)
                       << Marble::GeoDataCoordinates::normalizeLat(latLonAltBox.north(Marble::GeoDataCoordinates::Radian)+eps);

            return MaxSqrPointDistance;
        }
    }

    if ( geometry->nodeType() == Marble::GeoDataTypes::GeoDataPolygonType)
    {
        const Marble::GeoDataPolygon *polygon = dynamic_cast<const Marble::GeoDataPolygon*>(geometry);
        double prevPointDistance = MaxSqrPointDistance;
        if(polygon->contains(clickedPoint))
        {
            prevPointDistance = epsSqr*0.9;
        }

        prevPointDistance = qMin(distanceToLineSqr(QVector<Marble::GeoDataLineString>() << polygon->outerBoundary(), clickedPoint, eps), prevPointDistance);
        foreach(const auto& inner, polygon->innerBoundaries())
        {
            prevPointDistance = qMin(distanceToLineSqr(QVector<Marble::GeoDataLineString>() << inner, clickedPoint, eps), prevPointDistance);
        }

        return prevPointDistance;
    }
    else if ( geometry->nodeType() == Marble::GeoDataTypes::GeoDataMultiLineStringType)
    {
        const Marble::GeoDataMultiLineString *multiLineString = dynamic_cast<const Marble::GeoDataMultiLineString*>( geometry );

        return distanceToLineSqr(multiLineString->lineStrings(), clickedPoint, eps);
    }
    else if ( geometry->nodeType() == Marble::GeoDataTypes::GeoDataLineStringType)
    {
        const Marble::GeoDataLineString *lineString = dynamic_cast<const Marble::GeoDataLineString*>( geometry );
        return distanceToLineSqr(QVector<Marble::GeoDataLineString>() << *lineString, clickedPoint, eps);
    }
    else if ( geometry->nodeType() == Marble::GeoDataTypes::GeoDataPointType )
    {
        const Marble::GeoDataPoint *point = dynamic_cast<const Marble::GeoDataPoint*>(geometry );
        QPointF clickedPos = getPointRadian(clickedPoint);
        QPointF tempPos = getPointRadian(point->coordinates());

        return GeometryHelper::sqr( tempPos.x()-clickedPos.x() ) + GeometryHelper::sqr( tempPos.y()-clickedPos.y() );
    }
    else
    {
        const Marble::GeoDataMultiGeometry *multiGeometry = dynamic_cast<const Marble::GeoDataMultiGeometry*>(geometry);
        if ( multiGeometry )
        {
            double prevPointDistance = MaxSqrPointDistance;
            QVector<const Marble::GeoDataGeometry*>::ConstIterator multiIter = multiGeometry->constBegin();
            QVector<const Marble::GeoDataGeometry*>::ConstIterator const multiEnd = multiGeometry->constEnd();

            for ( ; multiIter != multiEnd; ++multiIter )
            {
                prevPointDistance = qMin(sqrDistanceToGeometry(*multiIter, clickedPoint, eps, epsSqr), prevPointDistance);
            }

            return prevPointDistance;
        }
    }


    return MaxSqrPointDistance;
}

double
sqrDistanceToPlacemark(Marble::GeoDataPlacemark *placemark, const Marble::GeoDataCoordinates &clickedPoint, double eps, double epsSqr)
{
    return sqrDistanceToGeometry(placemark->geometry(), clickedPoint, eps, epsSqr);
}


class FindFeatureRunnable : public QRunnable
{
public:
    FindFeatureRunnable( QList< Marble::GeoGraphicsItemPtr > items, Marble::GeoDataCoordinates clickedPoint, double epsSqr, double eps)
        :   aItems(std::move(items)),
            aClickedPoint(std::move(clickedPoint)),
            aEpsSqr(epsSqr),
            aEps(eps),
            aFeature(nullptr),
            aPrevDistSqr(1.0e10)
    {

    }

    
    void
    run() override;

    const Marble::GeoDataFeature*
    feature()
    {
        return aFeature;
    }

    double
    prevDistSqr() const
    {
        return aPrevDistSqr;
    }

private:
    QList< Marble::GeoGraphicsItemPtr > aItems;
    Marble::GeoDataCoordinates aClickedPoint;
    double aEpsSqr;
    double aEps;

    const Marble::GeoDataFeature* aFeature;
    double aPrevDistSqr;

};


void FindFeatureRunnable::run()
{
    foreach ( const Marble::GeoGraphicsItemPtr &item,  aItems)
    {
        if(item->feature()->nodeType() == Marble::GeoDataTypes::GeoDataPlacemarkType)
        {
            if(!item->feature()->isVisible() ||
               !item->style() ||
               !item->feature()->hightlighStyle())
            {
                qDebug() << "No style" << item->feature()->isVisible() << item->style() << item->feature()->name();
                continue;
            }

            Marble::GeoDataPlacemark *placemark = const_cast<Marble::GeoDataPlacemark*>(static_cast<const Marble::GeoDataPlacemark*>( item->feature()));
            double tempDistSqr = sqrDistanceToPlacemark(placemark, aClickedPoint, aEps, aEpsSqr);
            qDebug() << "distanceToPlacemark" << tempDistSqr << placemark->name() << placemark->description();

            if ( tempDistSqr <= aEpsSqr &&
                 (tempDistSqr < aPrevDistSqr || (aFeature &&
                                                      (placemark->zLevel() > aFeature->zLevel()) &&
                                                      (qFuzzyCompare(tempDistSqr, aPrevDistSqr) ||
                                                       (!qFuzzyIsNull(tempDistSqr) && !qFuzzyIsNull(aPrevDistSqr) && tempDistSqr/aPrevDistSqr < MAX_SQR_DIFF_FOR_HIGHER_Z_LEVEL)))))
            {
                aPrevDistSqr = tempDistSqr;
                aFeature = placemark;
            }
        }
    }

}



} //namespace

namespace Marble
{
class GeometryLayerPrivate
{
public:
    typedef QList<GeoDataPlacemark const *> OsmQueue;

    GeometryLayerPrivate( const QAbstractItemModel *model,
                          const SunLocator *sunLocator );

    ~GeometryLayerPrivate();

    void createGraphicsItems( const GeoDataFeature *feature);
    bool removeGraphicsItems( const GeoDataFeature *feature );
    void requestDelayedRepaint();
    void reqeustStartGenerateNextLevel(const TileId &tileId);
    int getTileLevel(double radius);

    QList<TileId>
    getTiles(int tileLevel, const GeoDataLatLonBox &latLongBox);

    const QAbstractItemModel *const m_model;
    GeoGraphicsScene m_scene;
    QString m_runtimeTrace;

    const GeoDataFeature *m_previousFeature;
    GeoDataCoordinates m_previousFeatureCoorinate;
    const ViewportParams *m_previousViewport;

    mutable QThreadPool m_threadPool;
    mutable GeoLabelPlaceHandler placeHandler;

    GeoDataLatLonAltBox m_latLonBox;
    int m_radius;

    GeoDataCoordinates m_centerCoordinates;
    int m_tileZoomLevel;

    VectorTileLoader m_loader;
    MergedLayerDecorator m_layerDecorator;
    StackedTileLoader    m_tileLoader;
    TextureMapperInterface *m_texmapper;
    GeoSceneTextureTileDataset *m_tileDataset;
    QTimer m_repaintTimer;

    QTimer m_generateNextLevelTimer;
    QSet<TileId> m_generateNextLevelTiles;

};

const int GEOMETRY_REPAINT_SCHEDULING_INTERVAL = 10;
const int GEOMETRY_GENERATE_NEXT_LEVELT_SCHEDULING_INTERVAL = 100;

GeometryLayerPrivate::GeometryLayerPrivate( const QAbstractItemModel *model,
                                            const SunLocator *sunLocator )
    : m_model( model ),
      m_previousFeature(nullptr),
      m_previousViewport(nullptr),
      m_centerCoordinates(),
      m_tileZoomLevel( -1 ),
      m_loader(&m_scene),
      m_layerDecorator(&m_loader, sunLocator),
      m_tileLoader(&m_layerDecorator),
      m_texmapper( nullptr )
{
    m_tileDataset = new GeoSceneTextureTileDataset(QStringLiteral("Geometry"));
    m_tileDataset->setProjection(GeoSceneTileDataset::Mercator);
    m_tileDataset->setTileSize(QSize(2048, 2048));
    m_tileDataset->setLevelZeroColumns(1);
    m_tileDataset->setLevelZeroRows(1);

    m_layerDecorator.setTextureLayers( QVector<const GeoSceneTextureTileDataset *>() << m_tileDataset );
    m_scene.setTextureLayer(m_tileDataset);
}

GeometryLayerPrivate::~GeometryLayerPrivate()
{
    delete m_tileDataset;
}


void GeometryLayerPrivate::createGraphicsItems(const GeoDataFeature *feature )
{
    m_scene.createGraphicsItems(feature);
}

bool GeometryLayerPrivate::removeGraphicsItems( const GeoDataFeature *feature )
{
    placeHandler.removeItem( feature );

    return m_scene.removeGraphicsItems(feature);
}

void GeometryLayerPrivate::requestDelayedRepaint()
{
    if ( m_texmapper )
    {
        m_texmapper->setRepaintNeeded();
    }

    m_repaintTimer.start();
}

void GeometryLayerPrivate::reqeustStartGenerateNextLevel(const TileId &tileId)
{
    if(tileId.tileLevel() != 0)
    {
        TileId nextTileId(tileId.mapThemeIdHash(), tileId.tileLevel()-1, tileId.x()/2, tileId.y()/2);
        if(!m_generateNextLevelTiles.contains(nextTileId) && m_loader.tileStatus(m_tileDataset, nextTileId) != VectorTileLoader::TileStatus::Available)
        {
            m_generateNextLevelTimer.start();
            m_generateNextLevelTiles.insert(nextTileId);
        }

        /* Just to ensure we always have zero zoom level */
        TileId topTileId(tileId.mapThemeIdHash(), 0, 0, 0);
        if(!m_generateNextLevelTiles.contains(topTileId) && m_loader.tileStatus(m_tileDataset, topTileId) != VectorTileLoader::TileStatus::Available)
        {
            m_generateNextLevelTimer.start();
            m_generateNextLevelTiles.insert(topTileId);
        }
    }
}


QList<TileId>
GeometryLayerPrivate::getTiles(int tileLevel, const GeoDataLatLonBox &latLongBox)
{
    QList< TileId > tempItems;
    if ( latLongBox.west() > latLongBox.east() )
    {
        // Handle boxes crossing the IDL by splitting it into two separate boxes
        GeoDataLatLonBox left;
        left.setWest( -M_PI );
        left.setEast( latLongBox.east() );
        left.setNorth( latLongBox.north() );
        left.setSouth( latLongBox.south() );

        GeoDataLatLonBox right;
        right.setWest( latLongBox.west() );
        right.setEast( M_PI );
        right.setNorth( latLongBox.north() );
        right.setSouth( latLongBox.south() );

        tempItems = getTiles( tileLevel, left ) + getTiles( tileLevel, right);
    }
    else
    {
        qreal north, south, east, west;
        latLongBox.boundaries( north, south, east, west );

        TileId topLeftKey = TileId::fromCoordinates(m_tileDataset, GeoDataCoordinates(west, north, 0), tileLevel );
        TileId bottomRightKey = TileId::fromCoordinates(m_tileDataset, GeoDataCoordinates(east, south, 0), tileLevel );

        QRect rect(topLeftKey.x(), topLeftKey.y(), bottomRightKey.x() - topLeftKey.x() + 1, bottomRightKey.y() - topLeftKey.y() + 1);

        for(int x = rect.left(); x <= rect.right(); ++x)
        {
            for(int y = rect.top(); y <= rect.top(); ++y)
            {
                tempItems.append(TileId(0, tileLevel, x, y));
            }
        }
    }

    return tempItems;
}

int GeometryLayerPrivate::getTileLevel(double radius)
{

    // choose the smaller dimension for selecting the tile level, leading to higher-resolution results
    const int levelZeroWidth = m_layerDecorator.tileSize().width() * m_layerDecorator.tileColumnCount( 0 );
    const int levelZeroHight = m_layerDecorator.tileSize().height() * m_layerDecorator.tileRowCount( 0 );
    const int levelZeroMinDimension = qMin( levelZeroWidth, levelZeroHight );

    const qreal linearLevel = qMax<qreal>( 1.0, radius * 4.0 / levelZeroMinDimension );

    // As our tile resolution doubles with each level we calculate
    // the tile level from tilesize and the globe radius via log(2)
    const qreal tileLevelF = qLn( linearLevel ) / qLn( 2.0 );

    return qCeil(tileLevelF); // snap to the sharper tile level a tiny bit earlier
    // to work around rounding errors when the radius
    // roughly equals the global texture width
}

GeometryLayer::GeometryLayer( const QAbstractItemModel *model,
                              const SunLocator *sunLocator )
        : d( new GeometryLayerPrivate( model, sunLocator ) )
{
    const GeoDataObject *object = static_cast<GeoDataObject*>( d->m_model->index( 0, 0, QModelIndex() ).internalPointer() );
    if ( object && object->parent() )
    {
        const GeoDataFeature *feature = dynamic_cast<const GeoDataFeature*>(object);
        if(feature)
        {
            d->createGraphicsItems(feature);
        }
    }

    connect( model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
             this, SLOT(resetCacheData()) );
    connect( model, SIGNAL(rowsInserted(QModelIndex,int,int)),
             this, SLOT(addPlacemarks(QModelIndex,int,int)) );
    connect( model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
             this, SLOT(removePlacemarks(QModelIndex,int,int)) );
    connect( model, SIGNAL(modelReset()),
             this, SLOT(resetCacheData()) );
    connect( this, SIGNAL(highlightedFeaturesChanged(QVector<GeoDataFeature*>)),
             &d->m_scene, SLOT(applyHighlight(QVector<GeoDataFeature*>)) );
    connect( this, SIGNAL(selectedFeaturesChanged(QVector<GeoDataFeature*>)),
             &d->m_scene, SLOT(applySelected(QVector<GeoDataFeature*>)) );
    connect( &d->m_scene, SIGNAL(repaintNeeded()),
             this, SIGNAL(repaintNeeded()) );
    connect( &d->m_scene, SIGNAL(updatedTiles(const TileMap &)),
             this, SLOT(updateTileStatus(const TileMap &)) );
    connect( &d->m_loader, SIGNAL(tileCompleted(TileId,QImage)),
             this, SLOT(updateTile(TileId,QImage)) );

    // Repaint timer
    d->m_repaintTimer.setSingleShot( true );
    d->m_repaintTimer.setInterval( GEOMETRY_REPAINT_SCHEDULING_INTERVAL );

    // Repaint timer
    d->m_generateNextLevelTimer.setSingleShot( true );
    d->m_generateNextLevelTimer.setInterval( GEOMETRY_GENERATE_NEXT_LEVELT_SCHEDULING_INTERVAL );

    connect( &d->m_repaintTimer, SIGNAL(timeout()), this, SIGNAL(repaintNeeded()) );
    connect( &d->m_generateNextLevelTimer, SIGNAL(timeout()), this, SLOT(startGenerateNextLevel()) );
}

GeometryLayer::~GeometryLayer()
{
    delete d->m_texmapper;
    delete d;
}

QStringList GeometryLayer::renderPosition() const
{
    return QStringList( QStringLiteral("HOVERS_ABOVE_SURFACE") );
}



bool GeometryLayer::render( GeoPainter *painter, ViewportParams *viewport,
                            const QString& renderPos, GeoSceneLayer * layer )
{
    QTime timer;
    timer.start();

    Q_UNUSED( renderPos )
    Q_UNUSED( layer )

    // Stop repaint timer if it is already running
    d->m_repaintTimer.stop();

    if ( d->m_layerDecorator.textureLayersSize() == 0 )
        return false;

    if ( !d->m_texmapper )
        return false;

    if ( !qFuzzyCompare(d->m_centerCoordinates.longitude(), viewport->centerLongitude()) ||
         !qFuzzyCompare(d->m_centerCoordinates.latitude(), viewport->centerLatitude()))
    {
        d->m_centerCoordinates.setLongitude( viewport->centerLongitude() );
        d->m_centerCoordinates.setLatitude( viewport->centerLatitude() );
        d->m_texmapper->setRepaintNeeded();
    }


    int tileLevel = d->getTileLevel(viewport->radius());
    if ( tileLevel != d->m_tileZoomLevel ) {
        d->m_tileZoomLevel = tileLevel;
        emit tileLevelChanged( d->m_tileZoomLevel );
    }

    d->m_latLonBox = viewport->viewLatLonAltBox();
    d->m_radius = viewport->radius();

    d->m_loader.resetTilehash();
    
    MapQuality originalMapQuality = painter->mapQuality();
    painter->setMapQuality(HighQuality);

    const QRect dirtyRect = QRect( QPoint( 0, 0), viewport->size() );
    painter->save();
    d->m_texmapper->mapTexture( painter, viewport, d->m_tileZoomLevel, dirtyRect, nullptr );
    painter->restore();

    d->m_loader.cleanupTilehash();

    d->m_runtimeTrace = QStringLiteral("Texture Cache: %1 ").arg(d->m_tileLoader.tileCount());

    painter->save();

    QList< GeoGraphicsItemPtr > selectedItems = d->m_scene.items( viewport->viewLatLonAltBox(), true);
//    qWarning() << "selectedItems" << selectedItems.size();
    foreach( const GeoGraphicsItemPtr& item, selectedItems )
    {
        QSharedPointer<const GeoDataStyle> style = item->feature()->hightlighStyle();
        if(style)
        {
            item->renderGeometry(painter, viewport, style);
        }
    }
    painter->restore();


    painter->save();
    d->placeHandler.clear();

    QSet<GeoGraphicsItemPtr> selectedItemsSet = selectedItems.toSet();
    QList< GeoGraphicsItemPtr > visualItems = d->m_scene.items( viewport->viewLatLonAltBox());

    foreach( const GeoGraphicsItemPtr& item, visualItems )
    {
        QSharedPointer<const GeoDataStyle> style = item->style();
        if(selectedItemsSet.contains(item))
        {
            style = item->feature()->hightlighStyle();
        }

        item->renderIcons(painter, viewport, style);
    }

    painter->restore();




    painter->save();
    d->placeHandler.clear();

    foreach( const GeoGraphicsItemPtr& item, visualItems )
    {
        QSharedPointer<const GeoDataStyle> style = item->style();
        if(selectedItemsSet.contains(item))
        {
            style = item->feature()->hightlighStyle();
        }

        item->renderLabels(painter, viewport, style, d->placeHandler);
    }

    painter->restore();

    painter->setMapQuality(originalMapQuality);


//    qWarning() << "GeometryLayer::render finished" << timer.elapsed();
    return true;
}

QString GeometryLayer::runtimeTrace() const
{
    return d->m_runtimeTrace;
}


void GeometryLayer::addPlacemarks( QModelIndex parent, int first, int last )
{
    Q_ASSERT( first < d->m_model->rowCount( parent ) );
    Q_ASSERT( last < d->m_model->rowCount( parent ) );
    for( int i=first; i<=last; ++i )
    {
        QModelIndex index = d->m_model->index( i, 0, parent );
        Q_ASSERT( index.isValid() );
        const GeoDataObject *object = qvariant_cast<GeoDataObject*>(index.data( MarblePlacemarkModel::ObjectPointerRole ) );
        Q_ASSERT( object );

        const GeoDataFeature *feature = dynamic_cast<const GeoDataFeature*>( object );
        if( feature != nullptr )
        {
           d->createGraphicsItems( feature );
        }

    }
}

void GeometryLayer::removePlacemarks( QModelIndex parent, int first, int last )
{
    Q_ASSERT( last < d->m_model->rowCount( parent ) );
    bool isRepaintNeeded = false;
    for( int i=first; i<=last; ++i )
    {
        QModelIndex index = d->m_model->index( i, 0, parent );
        Q_ASSERT( index.isValid() );
        const GeoDataObject *object = qvariant_cast<GeoDataObject*>(index.data( MarblePlacemarkModel::ObjectPointerRole ) );
        const GeoDataFeature *feature = dynamic_cast<const GeoDataFeature*>( object );
        if( feature != nullptr )
        {
           isRepaintNeeded = d->removeGraphicsItems( feature );
        }
    }

    if( isRepaintNeeded )
    {
        d->m_previousFeature = nullptr;
        d->m_previousFeatureCoorinate = GeoDataCoordinates();
        d->m_previousViewport = nullptr;

        d->requestDelayedRepaint();
    }

}

void GeometryLayer::resetCacheData()
{
    d->m_previousFeature = nullptr;
    d->m_previousFeatureCoorinate = GeoDataCoordinates();
    d->m_previousViewport = nullptr;
    d->m_tileLoader.clear();
    d->m_loader.clear();

    d->m_scene.clear();

    const GeoDataObject *object = static_cast<GeoDataObject*>( d->m_model->index( 0, 0, QModelIndex() ).internalPointer() );
    if ( object && object->parent() )
    {
        const GeoDataFeature *feature = dynamic_cast<const GeoDataFeature*>(object);
        if(feature)
        {
            d->createGraphicsItems(feature);
        }
    }

    emit repaintNeeded();
}

const GeoDataFeature*
GeometryLayer::whichFeatureAt(const QPoint& curpos , const ViewportParams *viewport)
{
    double lon;
    double lat;
    bool ok = viewport->geoCoordinates(curpos.x(), curpos.y(), lon, lat, GeoDataCoordinates::Radian);
    if(!ok)
    {
        return nullptr;
    }

    GeoDataCoordinates clickedPoint(lon, lat, 0, GeoDataCoordinates::Radian);

    return getFeature(clickedPoint, viewport);
}


const GeoDataFeature*
GeometryLayer::getFeature(const GeoDataCoordinates &clickedPoint, const ViewportParams *viewport)
{
    if(d->m_previousFeatureCoorinate.isValid() &&
            d->m_previousViewport == viewport &&
            clickedPoint == d->m_previousFeatureCoorinate)
    {
        return d->m_previousFeature;
    }

    qreal y( 0.0 );
    QVector<double> x;
    bool globeHidesPoint;
    bool visible = viewport->screenCoordinates( clickedPoint, x, y, QSizeF(), globeHidesPoint );
    bool epsOk = false;
    double eps = 0;
    GeoDataLatLonBox box = viewport->viewLatLonAltBox();

    if ( visible )
    {
        for( int it = 0; it < x.size(); ++it )
        {
            QPointF point( x[it], y );
            const GeoDataFeature* feature = d->placeHandler.getFeature(point);
            if(feature)
            {
                return feature;
            }
        }

        /* Could find label. Calculate eps */
        for( int it = 0; it < x.size() && !epsOk; ++it )
        {
            QPointF point( x[it], y );

            if(!epsOk)
            {
                double addedLonTemp;
                double addedLatTemp;

                bool epsOkAdded = viewport->geoCoordinates(qRound(point.x()+PIXEL_EPSILON), qRound(point.y()+PIXEL_EPSILON), addedLonTemp, addedLatTemp, GeoDataCoordinates::Radian);

                double minusLonTemp;
                double minusLatTemp;

                bool epsOkNegateive = viewport->geoCoordinates(qRound(point.x()-PIXEL_EPSILON), qRound(point.y()-PIXEL_EPSILON), minusLonTemp, minusLatTemp, GeoDataCoordinates::Radian);


                if(epsOkAdded)
                {
                    eps = qSqrt(((clickedPoint.longitude(GeoDataCoordinates::Radian)-addedLonTemp)*(clickedPoint.longitude(GeoDataCoordinates::Radian)-addedLonTemp)) +
                                ((clickedPoint.latitude(GeoDataCoordinates::Radian)-addedLatTemp)*(clickedPoint.latitude(GeoDataCoordinates::Radian)-addedLatTemp)));
                    epsOk = true;
                }
                else if(epsOkNegateive)
                {
                    eps = qSqrt(((clickedPoint.longitude(GeoDataCoordinates::Radian)-minusLonTemp)*(clickedPoint.longitude(GeoDataCoordinates::Radian)-minusLonTemp)) +
                                ((clickedPoint.latitude(GeoDataCoordinates::Radian)-minusLatTemp)*(clickedPoint.latitude(GeoDataCoordinates::Radian)-minusLatTemp)));
                    epsOk = true;
                }

                if(epsOkAdded && epsOkNegateive)
                {
                    QVector<GeoDataCoordinates> points =  QVector<GeoDataCoordinates>() << GeoDataCoordinates(minusLonTemp, minusLatTemp) << GeoDataCoordinates(addedLonTemp, addedLatTemp);
                    GeoDataLineString lineString = GeoDataLineString(points, Tessellate | RespectLatitudeCircle);

                    box = GeoDataLatLonBox::fromLineString(lineString);
                }
            }
        }
    }

    if(!epsOk)
    {
        eps = EPSILON;

        QVector<GeoDataCoordinates> points =  QVector<GeoDataCoordinates>() << GeoDataCoordinates(clickedPoint.latitude(GeoDataCoordinates::Radian) - eps, clickedPoint.longitude(GeoDataCoordinates::Radian) - eps)
                                                                            << GeoDataCoordinates(clickedPoint.latitude(GeoDataCoordinates::Radian) + eps, clickedPoint.longitude(GeoDataCoordinates::Radian) + eps);

        GeoDataLineString lineString = GeoDataLineString(points, Tessellate | RespectLatitudeCircle);

        box = GeoDataLatLonBox::fromLineString(lineString);
    }


    QElapsedTimer timer;
    timer.start();

    qDebug() << "eps" << eps;

    double epsSqr = eps*eps;

    QList< GeoGraphicsItemPtr > items = d->m_scene.items(box);

    qWarning() << "whichFeatureAt elapsed getting items" << timer.nsecsElapsed() << items.size();

    if(items.isEmpty())
    {
        return nullptr;
    }

    const int numThreads = qMax(d->m_threadPool.maxThreadCount(), 2);
    const int yStep = qCeil(static_cast<double>(items.size()) / numThreads);
    QList<FindFeatureRunnable *> jobs;
    qDebug() << "whichFeatureAt starting job" << yStep << numThreads;

    for ( int i = 0; i < numThreads; ++i )
    {
        QList< GeoGraphicsItemPtr > tempItems = items.mid(i*yStep, i == numThreads -1 ? -1 : yStep);
        if(tempItems.size() == 0)
        {
            continue;
        }

        FindFeatureRunnable *job = new FindFeatureRunnable( tempItems, clickedPoint, epsSqr, eps );

        qDebug() << "whichFeatureAt starting job" << tempItems.size();


        job->setAutoDelete(false);
        jobs.append(job);
        d->m_threadPool.start( job );
    }

    d->m_threadPool.waitForDone();

    const GeoDataFeature* feature = 0;
    double prevDistSqr = 1.0e10;
    foreach(auto job, jobs)
    {
        double tempDistSqr = job->prevDistSqr();
        const Marble::GeoDataFeature* tempFeature = job->feature();
        if ( tempFeature &&
             tempDistSqr < epsSqr &&
             (tempDistSqr < prevDistSqr || (feature &&
                                                  (tempFeature->zLevel() > feature->zLevel()) &&
                                                  (qFuzzyCompare(tempDistSqr, prevDistSqr) ||
                                                   (!qFuzzyIsNull(tempDistSqr) && !qFuzzyIsNull(prevDistSqr) && tempDistSqr/prevDistSqr < MAX_SQR_DIFF_FOR_HIGHER_Z_LEVEL)))))
        {
            prevDistSqr = tempDistSqr;
            feature = tempFeature;
        }

        delete job;
    }

    d->m_previousFeature = feature;
    d->m_previousFeatureCoorinate = clickedPoint;
    d->m_previousViewport = viewport;

    qDebug() << "GeometryLayer::whichFeatureAt elapsed" << timer.nsecsElapsed();

    return feature;
}

QVector<GeoDataFeature *>
GeometryLayer::getFeatures(const GeoDataCoordinates &coordinate, const ViewportParams *viewport)
{
    const GeoDataFeature* feature = getFeature(coordinate, viewport);

    QVector<GeoDataFeature*> selectedFeatures;

    if(feature)
    {
        selectedFeatures.append(const_cast<GeoDataFeature *>(feature));
    }

    return selectedFeatures;
}

void
GeometryLayer::handleHighlight( const GeoDataCoordinates &coordinate, const ViewportParams * viewport )
{
    emit highlightedFeaturesChanged( getFeatures(coordinate, viewport) );
}

void
GeometryLayer::setHighlightFeatures(const QVector<GeoDataFeature *> &features)
{
    d->m_scene.applyHighlight(features);
}

void
GeometryLayer::handleSelected(const GeoDataCoordinates &coordinate, const ViewportParams *viewport)
{
    emit selectedFeaturesChanged( getFeatures(coordinate, viewport) );
}

void
GeometryLayer::setSelectedFeatures(const QVector<GeoDataFeature *> &features)
{
    d->m_scene.applySelected(features);
}

void GeometryLayer::visibleLatLonAltBoxChanged(const GeoDataLatLonAltBox &latLonBox)
{
    d->m_latLonBox = latLonBox;
}

void GeometryLayer::radiusChanged(int radius)
{
    d->m_radius = radius;
}

void GeometryLayer::updateTile(const TileId &tileId, const QImage &image)
{
    if ( image.isNull() )
        return; // keep tiles in cache to improve performance

    d->m_tileLoader.updateTile( tileId, image );
    d->reqeustStartGenerateNextLevel(tileId);

    if(tileId.tileLevel() == d->m_tileZoomLevel)
    {
        d->requestDelayedRepaint();
    }
}

void GeometryLayer::updateTileStatus(const TileMap &tiles)
{
    d->m_loader.setTileExpired(d->m_tileDataset, tiles);
    d->m_tileLoader.clear();
    d->requestDelayedRepaint();
}

void GeometryLayer::startGenerateNextLevel()
{
    foreach(const TileId &tileId, d->m_generateNextLevelTiles)
    {
        VectorTileLoader::TileStatus status = d->m_loader.tileStatus( d->m_tileDataset, tileId );
        if(status != VectorTileLoader::TileStatus::Available)
        {
            d->m_loader.createTile(d->m_tileDataset, tileId, DownloadUsage::DownloadBrowse);
        }
    }
}

void GeometryLayer::setProjection( Projection projection )
{
    // FIXME: replace this with an approach based on the factory method pattern.
    delete d->m_texmapper;

    switch( projection ) {
        case Spherical:
            d->m_texmapper = new SphericalScanlineTextureMapper( &d->m_tileLoader );
            break;
        case Equirectangular:
            d->m_texmapper = new EquirectScanlineTextureMapper( &d->m_tileLoader );
            break;
        case Mercator:
            if ( d->m_tileLoader.tileProjection() == GeoSceneTileDataset::Mercator ) {
                d->m_texmapper = new TileScalingTextureMapper( &d->m_tileLoader );
            } else {
                d->m_texmapper = new MercatorScanlineTextureMapper( &d->m_tileLoader );
            }
            break;
        case Gnomonic:
        case Stereographic:
        case LambertAzimuthal:
        case AzimuthalEquidistant:
        case VerticalPerspective:
            d->m_texmapper = new GenericScanlineTextureMapper( &d->m_tileLoader );
            break;
        default:
            d->m_texmapper = nullptr;
    }
    Q_ASSERT( d->m_texmapper );
}

void GeometryLayer::setNeedsUpdate()
{
    if ( d->m_texmapper ) {
        d->m_texmapper->setRepaintNeeded();
    }

    emit repaintNeeded();
}
}

//#include "moc_GeometryLayer.cpp"
