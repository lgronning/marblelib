#define QT_NO_DEBUG_OUTPUT
//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011      Konstantin Oblaukhov <oblaukhov.konstantin@gmail.com>
//

#include "GeoGraphicsScene.h"

#include "geodata/data/GeoDataFeature.h"
#include "geodata/data/GeoDataLatLonBox.h"
#include "geodata/data/GeoDataStyle.h"
#include "geodata/data/GeoDataStyleMap.h"
#include "geodata/data/GeoDataPlacemark.h"
#include "geodata/data/GeoDataDocument.h"
#include "geodata/parser/GeoDataTypes.h"
#include "geodata/data/GeoDataFolder.h"
#include "geodata/data/GeoDataLineStyle.h"
#include "geodata/data/GeoDataLineStyle.h"
#include "geodata/data/GeoDataFeature.h"
#include "geodata/data/GeoDataPlacemark.h"
#include "geodata/data/GeoDataPolygon.h"
#include "geodata/data/GeoDataMultiPolygon.h"
#include "geodata/data/GeoDataMultiPoint.h"
#include "geodata/data/GeoDataPolyStyle.h"
#include "geodata/data/GeoDataStyle.h"
#include "geodata/data/GeoDataStyleMap.h"
#include "geodata/data/GeoDataMultiLineString.h"
#include "geodata/data/GeoDataScreenOverlay.h"
#include "geodata/data/GeoDataGroundOverlay.h"

#include "geodata/graphicsitem/GeoMultiLineStringGraphicsItem.h"
#include "geodata/graphicsitem/GeoMultiPolygonGraphicsItem.h"
#include "geodata/graphicsitem/GeoLineStringGraphicsItem.h"
#include "geodata/graphicsitem/GeoPolygonGraphicsItem.h"
#include "geodata/graphicsitem/GeoPointGraphicsItem.h"
#include "geodata/graphicsitem/GeoGroundGraphicsItem.h"
#include "geodata/graphicsitem/GeoMultiGraphicsItem.h"
#include "geodata/graphicsitem/GeoMultiPointGraphicsItem.h"

#include "graphicsview/GeoGraphicsItem.h"
#include "TileId.h"
#include "TileCoordsPyramid.h"
#include "MarbleDebug.h"
#include <QtCore/QMap>
#include <QtCore/QtMath>
#include <QtCore/QElapsedTimer>
#include <QtConcurrent/QtConcurrentRun>

namespace Marble
{




const int MaxItemTileZoomLevel = 11;
class GeoGraphicsScenePrivate
{
public:
    GeoGraphicsScene *q;
    GeoGraphicsScenePrivate(GeoGraphicsScene *parent) :
        q(parent),
        m_tileDataset(nullptr)
    {
        initializeDefaultValues();
    }

    ~GeoGraphicsScenePrivate()
    {
        q->clear();
    }

    QMap<double /* z level */, QMap<double, QMap<double, QMap<double, QMap<double, QHash<GeoGraphicsItemPtr, TileMap> > > > > > m_items;
    QMap<const GeoDataFeature *, QList<GeoGraphicsItemPtr> > m_features_graphics_items;

    QHash<const GeoDataFeature *, AddItem*> jobs;

    // Stores the items which have been clicked;
    QSet<GeoGraphicsItemPtr> m_selectedItems;
    QSet<GeoGraphicsItemPtr> m_highlightedItems;

    GeoSceneTextureTileDataset *m_tileDataset;

    static void initializeDefaultValues();

    static int s_defaultZValues[GeoDataFeature::LastIndex];
    static int s_defaultMinZoomLevels[GeoDataFeature::LastIndex];
    static bool s_defaultValuesInitialized;
    static int s_maximumZoomLevel;
    static const int s_defaultZValue;
};

int GeoGraphicsScenePrivate::s_defaultZValues[GeoDataFeature::LastIndex];
int GeoGraphicsScenePrivate::s_defaultMinZoomLevels[GeoDataFeature::LastIndex];
bool GeoGraphicsScenePrivate::s_defaultValuesInitialized = false;
int GeoGraphicsScenePrivate::s_maximumZoomLevel = 0;
const int GeoGraphicsScenePrivate::s_defaultZValue = 0;




class AddItemJob
{
public:
    AddItemJob(GeoSceneTextureTileDataset *tileDataset, const GeoDataFeature *feature);

    ~AddItemJob();

    void
    run();

    void
    cancel()
    {
        aCancel = true;
    }

    bool
    isCancelled() const
    {
        return aCancel;
    }

    const GeoGraphicsItemPtr &
    item() const
    {
        return aItem;
    }

    const TileMap &
    tiles() const
    {
        return aTiles;
    }

    const GeoDataFeature *
    object() const
    {
        return aFeature;
    }

private:
    friend class TileRenderJob;

    std::atomic<bool> aCancel;
    const GeoDataFeature *aFeature;

    GeoGraphicsItemPtr aItem;
    TileMap aTiles;

    GeoSceneTextureTileDataset *aTileDataset;
};


AddItemJob::AddItemJob(GeoSceneTextureTileDataset *tileDataset, const GeoDataFeature *feature)
    :   aCancel(false),
        aFeature(feature),
        aTileDataset(tileDataset)
{}

AddItemJob::~AddItemJob()
{}

void AddItemJob::run()
{
    if(isCancelled())
    {
        return;
    }

    GeoGraphicsItemPtr tempItem;
    if ( const GeoDataPlacemark *placemark = dynamic_cast<const GeoDataPlacemark*>( aFeature ) )
    {
        const GeoDataGeometry*geometry = placemark->geometry();
        GeoGraphicsItem *geoGraphicsItem = nullptr;
        if ( geometry->nodeType() == GeoDataTypes::GeoDataLineStringType )
        {
            const GeoDataLineString* line = static_cast<const GeoDataLineString*>( geometry );
            geoGraphicsItem = new GeoLineStringGraphicsItem( placemark, line );
        }
        else if ( geometry->nodeType() == GeoDataTypes::GeoDataMultiLineStringType )
        {
            const GeoDataMultiLineString* line = static_cast<const GeoDataMultiLineString*>( geometry );
            geoGraphicsItem = new GeoMultiLineStringGraphicsItem( placemark, line );
        }
        else if ( geometry->nodeType() == GeoDataTypes::GeoDataPointType )
        {
            const GeoDataPoint * point = static_cast<const GeoDataPoint*>( geometry );
            geoGraphicsItem = new GeoPointGraphicsItem( placemark, point );
        }
        else if ( geometry->nodeType() == GeoDataTypes::GeoDataLinearRingType )
        {
            const GeoDataLinearRing *ring = static_cast<const GeoDataLinearRing*>( geometry );
            geoGraphicsItem = new GeoPolygonGraphicsItem( placemark, ring );
        }
        else if ( geometry->nodeType() == GeoDataTypes::GeoDataPolygonType )
        {
            const GeoDataPolygon *poly = static_cast<const GeoDataPolygon*>( geometry );
            geoGraphicsItem = new GeoPolygonGraphicsItem( placemark, poly );
        }
        else if ( geometry->nodeType() == GeoDataTypes::GeoDataMultiPolygonType )
        {
            const GeoDataMultiPolygon *poly = static_cast<const GeoDataMultiPolygon*>( geometry );
            geoGraphicsItem = new GeoMultiPolygonGraphicsItem( placemark, poly );
        }
        else if ( geometry->nodeType() == GeoDataTypes::GeoDataMultiPointType)
        {
            const GeoDataMultiPoint *poly = static_cast<const GeoDataMultiPoint*>( geometry );
            geoGraphicsItem = new GeoMultiPointGraphicsItem( placemark, poly );
        }
        else if ( geometry->nodeType() == GeoDataTypes::GeoDataMultiGeometryType  )
        {
            const GeoDataMultiGeometry *multigeo = static_cast<const GeoDataMultiGeometry*>( geometry );
            geoGraphicsItem = new GeoMultiGraphicsItem( placemark, multigeo );
        }

        if ( !geoGraphicsItem )
            return;

        if(placemark->zLevel() == 0)
        {
            geoGraphicsItem->setZLevel( GeoGraphicsScenePrivate::s_defaultZValues[placemark->visualCategory()] );
        }

        geoGraphicsItem->setMinZoomLevel( GeoGraphicsScenePrivate::s_defaultMinZoomLevels[placemark->visualCategory()] );
        tempItem = GeoGraphicsItemPtr(geoGraphicsItem);
    }
    else if ( aFeature->nodeType() == GeoDataTypes::GeoDataGroundOverlayType )
    {

        const GeoDataGroundOverlay* overlay = dynamic_cast<const GeoDataGroundOverlay*>( aFeature );
        tempItem = ( GeoGraphicsItemPtr(new GeoGroundGraphicsItem( overlay )) );
    }

    if(!tempItem)
    {
        return;
    }

    const TileId tileId = TileId(0, 0, 0,0 );

    QTime timer;
    timer.start();

    tempItem->getTiles(aTileDataset, tileId, MaxItemTileZoomLevel, aTiles, aCancel);

    if(isCancelled())
    {
        return;
    }

    aItem = tempItem;


}


void GeoGraphicsScenePrivate::initializeDefaultValues()
{
    if ( s_defaultValuesInitialized )
        return;

    for (int & i : s_defaultZValues)
        i = s_defaultZValue;

    for (int & s_defaultMinZoomLevel : s_defaultMinZoomLevels)
        s_defaultMinZoomLevel = 15;

    s_defaultZValues[GeoDataFeature::None]                = 0;

    //Amenity
    s_defaultZValues[GeoDataFeature::AmenityGraveyard]    = s_defaultZValue - 12;

    s_defaultZValues[GeoDataFeature::EducationCollege]    = s_defaultZValue - 12;
    s_defaultZValues[GeoDataFeature::EducationSchool]     = s_defaultZValue - 12;
    s_defaultZValues[GeoDataFeature::EducationUniversity] = s_defaultZValue - 12;
    s_defaultZValues[GeoDataFeature::HealthHospital]      = s_defaultZValue - 12;

    //Landuse

    for ( int i = GeoDataFeature::LanduseAllotments; i <= GeoDataFeature::LanduseVineyard; i++ )
        s_defaultZValues[static_cast<GeoDataFeature::GeoDataVisualCategory>(i)] = s_defaultZValue - 17;

    s_defaultZValues[GeoDataFeature::NaturalWood]         = s_defaultZValue - 15;
    s_defaultZValues[GeoDataFeature::NaturalBeach]        = s_defaultZValue - 13;
    s_defaultZValues[GeoDataFeature::NaturalWetland]      = s_defaultZValue - 13;
    s_defaultZValues[GeoDataFeature::NaturalGlacier]      = s_defaultZValue - 10;
    s_defaultZValues[GeoDataFeature::NaturalScrub]        = s_defaultZValue - 13;
    s_defaultZValues[GeoDataFeature::NaturalWater]        = s_defaultZValue - 13;
    s_defaultZValues[GeoDataFeature::NaturalCliff]        = s_defaultZValue - 13;
    s_defaultZValues[GeoDataFeature::NaturalPeak]         = s_defaultZValue - 13;

    //Military

    s_defaultZValues[GeoDataFeature::MilitaryDangerArea]  = s_defaultZValue - 10;

    //Leisure

    s_defaultZValues[GeoDataFeature::LeisurePark]         = s_defaultZValue - 14;
    s_defaultZValues[GeoDataFeature::LeisurePlayground]   = s_defaultZValue - 13;
    s_defaultZValues[GeoDataFeature::LeisurePitch]        = s_defaultZValue - 13;
    s_defaultZValues[GeoDataFeature::LeisureSportsCentre] = s_defaultZValue - 13;
    s_defaultZValues[GeoDataFeature::LeisureStadium]      = s_defaultZValue - 13;
    s_defaultZValues[GeoDataFeature::LeisureTrack]        = s_defaultZValue - 13;

    s_defaultZValues[GeoDataFeature::TransportParking]    = s_defaultZValue - 13;

    s_defaultZValues[GeoDataFeature::ManmadeBridge]       = s_defaultZValue - 12;
    s_defaultZValues[GeoDataFeature::BarrierCityWall]     = s_defaultZValue - 1;

    s_defaultZValues[GeoDataFeature::HighwayUnknown]      = s_defaultZValue - 11;
    s_defaultZValues[GeoDataFeature::HighwayPath]         = s_defaultZValue - 10;
    s_defaultZValues[GeoDataFeature::HighwayTrack]        = s_defaultZValue - 9;
    s_defaultZValues[GeoDataFeature::HighwaySteps]        = s_defaultZValue - 8;
    s_defaultZValues[GeoDataFeature::HighwayFootway]      = s_defaultZValue - 8;
    s_defaultZValues[GeoDataFeature::HighwayCycleway]     = s_defaultZValue - 8;
    s_defaultZValues[GeoDataFeature::HighwayService]      = s_defaultZValue - 7;
    s_defaultZValues[GeoDataFeature::HighwayResidential]  = s_defaultZValue - 7;
    s_defaultZValues[GeoDataFeature::HighwayLivingStreet] = s_defaultZValue - 7;
    s_defaultZValues[GeoDataFeature::HighwayPedestrian]   = s_defaultZValue - 6;
    s_defaultZValues[GeoDataFeature::HighwayRoad]         = s_defaultZValue - 6;
    s_defaultZValues[GeoDataFeature::HighwayUnclassified] = s_defaultZValue - 6;
    s_defaultZValues[GeoDataFeature::HighwayTertiary]     = s_defaultZValue - 5;
    s_defaultZValues[GeoDataFeature::HighwaySecondary]    = s_defaultZValue - 4;
    s_defaultZValues[GeoDataFeature::HighwayPrimary]      = s_defaultZValue - 3;
    s_defaultZValues[GeoDataFeature::HighwayTrunk]        = s_defaultZValue - 2;
    s_defaultZValues[GeoDataFeature::HighwayMotorway]     = s_defaultZValue - 1;
    s_defaultZValues[GeoDataFeature::RailwayRail]         = s_defaultZValue - 1;

    s_defaultZValues[GeoDataFeature::HighwayTertiaryLink] = s_defaultZValues[GeoDataFeature::HighwayTertiary];
    s_defaultZValues[GeoDataFeature::HighwaySecondaryLink]= s_defaultZValues[GeoDataFeature::HighwaySecondary];
    s_defaultZValues[GeoDataFeature::HighwayPrimaryLink]  = s_defaultZValues[GeoDataFeature::HighwayPrimary];
    s_defaultZValues[GeoDataFeature::HighwayTrunkLink]    = s_defaultZValues[GeoDataFeature::HighwayTrunk];
    s_defaultZValues[GeoDataFeature::HighwayMotorwayLink] = s_defaultZValues[GeoDataFeature::HighwayMotorway];

    s_defaultMinZoomLevels[GeoDataFeature::Default]             = 3;
    s_defaultMinZoomLevels[GeoDataFeature::NaturalWater]        = 8;
    s_defaultMinZoomLevels[GeoDataFeature::NaturalWood]         = 8;
    s_defaultMinZoomLevels[GeoDataFeature::NaturalBeach]        = 10;
    s_defaultMinZoomLevels[GeoDataFeature::NaturalWetland]      = 10;
    s_defaultMinZoomLevels[GeoDataFeature::NaturalGlacier]      = 8;
    s_defaultMinZoomLevels[GeoDataFeature::NaturalScrub]        = 10;
    s_defaultMinZoomLevels[GeoDataFeature::NaturalCliff]        = 15;
    s_defaultMinZoomLevels[GeoDataFeature::NaturalPeak]         = 11;
    s_defaultMinZoomLevels[GeoDataFeature::BarrierCityWall]     = 15;
    s_defaultMinZoomLevels[GeoDataFeature::Building]            = 15;

    s_defaultMinZoomLevels[GeoDataFeature::ManmadeBridge]       = 15;

        // OpenStreetMap highways
    s_defaultMinZoomLevels[GeoDataFeature::HighwaySteps]        = 15;
    s_defaultMinZoomLevels[GeoDataFeature::HighwayUnknown]      = 13;
    s_defaultMinZoomLevels[GeoDataFeature::HighwayPath]         = 13;
    s_defaultMinZoomLevels[GeoDataFeature::HighwayTrack]        = 13;
    s_defaultMinZoomLevels[GeoDataFeature::HighwayPedestrian]   = 13;
    s_defaultMinZoomLevels[GeoDataFeature::HighwayFootway]      = 13;
    s_defaultMinZoomLevels[GeoDataFeature::HighwayCycleway]     = 13;
    s_defaultMinZoomLevels[GeoDataFeature::HighwayService]      = 13;
    s_defaultMinZoomLevels[GeoDataFeature::HighwayRoad]         = 13;
    s_defaultMinZoomLevels[GeoDataFeature::HighwayTertiaryLink] = 10;
    s_defaultMinZoomLevels[GeoDataFeature::HighwayTertiary]     = 10;
    s_defaultMinZoomLevels[GeoDataFeature::HighwaySecondaryLink]= 10;
    s_defaultMinZoomLevels[GeoDataFeature::HighwaySecondary]    = 9;
    s_defaultMinZoomLevels[GeoDataFeature::HighwayPrimaryLink]  = 10;
    s_defaultMinZoomLevels[GeoDataFeature::HighwayPrimary]      = 8;
    s_defaultMinZoomLevels[GeoDataFeature::HighwayTrunkLink]    = 10;
    s_defaultMinZoomLevels[GeoDataFeature::HighwayTrunk]        = 7;
    s_defaultMinZoomLevels[GeoDataFeature::HighwayMotorwayLink] = 10;
    s_defaultMinZoomLevels[GeoDataFeature::HighwayMotorway]     = 6;

    //FIXME: Bad, better to expand this
    for(int i = GeoDataFeature::AccomodationCamping; i <= GeoDataFeature::ReligionSikh; i++)
        s_defaultMinZoomLevels[i] = 15;

    s_defaultMinZoomLevels[GeoDataFeature::AmenityGraveyard]    = 14;
    s_defaultMinZoomLevels[GeoDataFeature::AmenityFountain]     = 17;

    s_defaultMinZoomLevels[GeoDataFeature::MilitaryDangerArea]  = 11;

    s_defaultMinZoomLevels[GeoDataFeature::LeisurePark]         = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LeisurePlayground]   = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseAllotments]   = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseBasin]        = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseCemetery]     = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseCommercial]   = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseConstruction] = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseFarmland]     = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseFarmyard]     = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseGarages]      = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseGrass]        = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseIndustrial]   = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseLandfill]     = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseMeadow]       = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseMilitary]     = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseQuarry]       = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseRailway]      = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseReservoir]    = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseResidential]  = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseRetail]       = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseOrchard]      = 14;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseVineyard]     = 14;

    s_defaultMinZoomLevels[GeoDataFeature::RailwayRail]         = 6;
    s_defaultMinZoomLevels[GeoDataFeature::RailwayNarrowGauge]  = 6;
    s_defaultMinZoomLevels[GeoDataFeature::RailwayTram]         = 14;
    s_defaultMinZoomLevels[GeoDataFeature::RailwayLightRail]    = 12;
    s_defaultMinZoomLevels[GeoDataFeature::RailwayAbandoned]    = 10;
    s_defaultMinZoomLevels[GeoDataFeature::RailwaySubway]       = 13;
    s_defaultMinZoomLevels[GeoDataFeature::RailwayPreserved]    = 13;
    s_defaultMinZoomLevels[GeoDataFeature::RailwayMiniature]    = 13;
    s_defaultMinZoomLevels[GeoDataFeature::RailwayConstruction] = 10;
    s_defaultMinZoomLevels[GeoDataFeature::RailwayMonorail]     = 12;
    s_defaultMinZoomLevels[GeoDataFeature::RailwayFunicular]    = 13;
    s_defaultMinZoomLevels[GeoDataFeature::TransportPlatform]   = 16;

    s_defaultMinZoomLevels[GeoDataFeature::Satellite]           = 0;

    for (int & s_defaultMinZoomLevel : s_defaultMinZoomLevels) {
        s_maximumZoomLevel = qMax( s_maximumZoomLevel, s_defaultMinZoomLevel );
    }

    s_defaultValuesInitialized = true;
}


GeoGraphicsScene::GeoGraphicsScene( QObject* parent ):
    QObject( parent ),
    d( new GeoGraphicsScenePrivate(this) )
{
    connect(this, SIGNAL(startAddItem(const GeoDataFeature *)), SLOT(doAddItem(const GeoDataFeature *)), Qt::QueuedConnection);
}

GeoGraphicsScene::~GeoGraphicsScene()
{
    delete d;
}

QList< GeoGraphicsItemPtr >
GeoGraphicsScene::items( const GeoDataLatLonBox &box, bool highlightedItems ) const
{
    QList< GeoGraphicsItemPtr > tempItems;
    if ( box.west() > box.east() )
    {
        // Handle boxes crossing the IDL by splitting it into two separate boxes
        GeoDataLatLonBox left;
        left.setWest( -M_PI );
        left.setEast( box.east() );
        left.setNorth( box.north() );
        left.setSouth( box.south() );

        GeoDataLatLonBox right;
        right.setWest( box.west() );
        right.setEast( M_PI );
        right.setNorth( box.north() );
        right.setSouth( box.south() );

        tempItems = itemsImpl( left, highlightedItems ) + itemsImpl( right, highlightedItems);
    }
    else
    {
        tempItems = itemsImpl(box, highlightedItems);
    }

    return tempItems;
}

namespace
{

int pow2(int levelDiff)
{
    if(levelDiff == 0)
    {
        return 1;
    }

    int value = 2;
    for(int i = 1; i < levelDiff; ++i)
    {
        value *= 2;
    }

    return value;

}

}
QList<GeoGraphicsItemPtr> GeoGraphicsScene::items(const GeoSceneTileDataset *tileData, const TileId tileId, bool highlightedItems) const
{
    QMap<int, QPair<QPoint, QPoint> > tileRects;

    for(int tempZoomLevel = tileId.tileLevel(); tempZoomLevel <= MaxItemTileZoomLevel; ++tempZoomLevel)
    {
        tileRects.insert(tempZoomLevel, qMakePair(QPoint(tileId.x()*pow2(tempZoomLevel - tileId.tileLevel()), tileId.y()*pow2(tempZoomLevel - tileId.tileLevel())),
                                                       QPoint((tileId.x()+1)*pow2(tempZoomLevel - tileId.tileLevel())-1, (tileId.y() + 1)*pow2(tempZoomLevel - tileId.tileLevel())-1)));
    }

    for(int tempZoomLevel = qMin(tileId.tileLevel()-1, MaxItemTileZoomLevel); tempZoomLevel > 0; --tempZoomLevel)
    {
        QPoint point(qFloor(static_cast<double>(tileId.x())/static_cast<double>(pow2(tileId.tileLevel()-tempZoomLevel))),
                     qFloor(static_cast<double>(tileId.y())/static_cast<double>(pow2(tileId.tileLevel()-tempZoomLevel))));
        tileRects.insert(tempZoomLevel, qMakePair(point,point));
    }

    GeoDataLatLonBox latLonBox = tileId.toLatLonBox(tileData);


    return getItems(tileRects, latLonBox, highlightedItems);
}

QList<GeoGraphicsItemPtr> GeoGraphicsScene::itemsImpl( const GeoDataLatLonBox &box, bool highlightedItems ) const
{
    qreal north, south, east, west;
    box.boundaries( north, south, east, west );

    QMap<int, QPair<QPoint, QPoint> > tileRects;

    for(int tempTileLevel = MaxItemTileZoomLevel; tempTileLevel >= 0; tempTileLevel--)
    {
        TileId topLeftKey = TileId::fromCoordinates( d->m_tileDataset, GeoDataCoordinates(west, north, 0), tempTileLevel );
        TileId bottomRightKey = TileId::fromCoordinates( d->m_tileDataset, GeoDataCoordinates(east, south, 0), tempTileLevel );

        int minY = qMin(topLeftKey.y(), bottomRightKey.y());
        int maxY = qMax(topLeftKey.y(), bottomRightKey.y());
        int minX = qMin(topLeftKey.x(), bottomRightKey.x());
        int maxX = qMax(topLeftKey.x(), bottomRightKey.x());

        tileRects.insert(tempTileLevel, qMakePair(QPoint(minX, minY), QPoint(maxX, maxY)));
    }

    return getItems(tileRects, box, highlightedItems);
}



QList<GeoGraphicsItemPtr> GeoGraphicsScene::getItems(const QMap<int, QPair<QPoint, QPoint> > &tileRects,  const GeoDataLatLonBox &box, bool highlightedItems ) const
{
    qreal north, south, east, west;
    box.boundaries( north, south, east, west );


    QList< GeoGraphicsItemPtr > result;
    auto zLevelIt = d->m_items.constBegin();
    auto zLevelItEnd = d->m_items.constEnd();

    for(; zLevelIt != zLevelItEnd; ++zLevelIt)
    {
        const auto & hash = zLevelIt.value();

        auto itemItLeft = hash.begin();
        auto itemItLeftEnd = hash.upperBound(east);

        for(; itemItLeft != itemItLeftEnd && itemItLeft.key() <= east; ++itemItLeft)
        {
            const auto & hash = itemItLeft.value();

            auto itemItRight = hash.lowerBound(west);
            auto itemItRightEnd = hash.constEnd();

            for(; itemItRight != itemItRightEnd; ++itemItRight)
            {
                if(itemItRight.key() < west)
                {
                    continue;
                }

                const auto & hash = itemItRight.value();

                auto itemItBottom = hash.begin();
                auto itemItBottomEnd = hash.upperBound(north);

                for(; itemItBottom != itemItBottomEnd && itemItBottom.key() <= north; ++itemItBottom)
                {
                    const auto & hash = itemItBottom.value();

                    auto itemItTop = hash.lowerBound(south);
                    auto itemItTopEnd = hash.constEnd();

                    for(; itemItTop != itemItTopEnd; ++itemItTop)
                    {
                        if(itemItTop.key() < south)
                        {
                            continue;
                        }

                        auto itemIt = itemItTop.value().constBegin();
                        auto itemItEnd = itemItTop.value().constEnd();

                        for(; itemIt != itemItEnd; ++itemIt)
                        {
                            const GeoGraphicsItemPtr &item = itemIt.key();
                            if (!item->visible() || (highlightedItems && (!d->m_highlightedItems.contains(item) && !d->m_selectedItems.contains(item))) )
                            {
                                continue;
                            }

                            bool found = false;

                            for(int tempTileLevel = MaxItemTileZoomLevel; tempTileLevel >= 0 && !found; tempTileLevel--)
                            {
                                const auto & hash = itemIt.value().value(tempTileLevel);

                                auto itX = hash.lowerBound(tileRects[tempTileLevel].first.x());
                                auto itEnd = hash.constEnd();

                                for(; itX != itEnd && itX.key() <= tileRects[tempTileLevel].second.x() && !found; ++itX)
                                {
                                    if(itX.key() < tileRects[tempTileLevel].first.x())
                                    {
                                        continue;
                                    }


                                    auto itY = itX.value().lowerBound(tileRects[tempTileLevel].first.y());
                                    auto itYEnd = itX.value().constEnd();

                                    for(; itY != itYEnd && itY.key() <= tileRects[tempTileLevel].second.y(); ++itY)
                                    {
                                        if(itY.key() < tileRects[tempTileLevel].first.y())
                                        {
                                            continue;
                                        }

                                        found = true;
                                        break;
                                    }
                                }
                            }

                            if(found)
                            {
                                result.append(item);
                            }

                        }
                    }
                }
            }
        }
    }

    return result;
}

void GeoGraphicsScene::applyImpl(const QVector<GeoDataFeature*>& features, QSet<GeoGraphicsItemPtr> &oldItems)
{
    qDebug() << "GeoGraphicsScene::applyHighlight" << features.size();

    /**
     * First set the items, which were selected previously, to
     * use normal style
     */


    // Also clear the list to store the new selected items
    QSet<GeoGraphicsItemPtr> tempOldItems = oldItems;
    oldItems.clear();

    /**
     * Process the placemark. which were under mouse
     * while clicking, and update corresponding graphics
     * items to use highlight style
     */

    foreach(const GeoDataFeature *feature, features)
    {
        if(!d->m_features_graphics_items.contains(feature))
        {
            continue;
        }

        foreach ( const GeoGraphicsItemPtr &item, d->m_features_graphics_items[feature])
        {
            oldItems.insert(item);
        }
    }

    if(tempOldItems != oldItems)
    {
        emit repaintNeeded();
    }
}

void
GeoGraphicsScene::applyHighlight( const QVector< GeoDataFeature* > &selectedFeatures )
{
    applyImpl(selectedFeatures, d->m_highlightedItems);
}

void
GeoGraphicsScene::applySelected(const QVector<GeoDataFeature *> &selectedFeatures)
{
    applyImpl(selectedFeatures, d->m_selectedItems);
}

void GeoGraphicsScene::doAddItem(const GeoDataFeature *feature)
{
    AddItem *addItem = new AddItem(feature, d->m_tileDataset, this);
    d->jobs.insert(feature, addItem);

    connect(addItem, SIGNAL(finished(const GeoDataFeature *, const GeoGraphicsItemPtr &, const TileMap &)), this, SLOT(handleFinished(const GeoDataFeature *, const GeoGraphicsItemPtr &, const TileMap &)));
    connect(addItem, SIGNAL(finished(const GeoDataFeature *, const GeoGraphicsItemPtr &, const TileMap &)), addItem, SLOT(deleteLater()));
    addItem->run();
}

void
GeoGraphicsScene::handleFinished(const GeoDataFeature *feature, const GeoGraphicsItemPtr &item, const TileMap &tiles)
{
    qDebug() << "GeoGraphicsScene::handleFinished" << tiles.size() << feature->nodeType();
    d->jobs.remove(feature);
    d->m_features_graphics_items[feature].append(item);

    if ( item->latLonAltBox().west() > item->latLonAltBox().east() )
    {
        d->m_items[feature->zLevel()][-M_PI][item->latLonAltBox().east()][item->latLonAltBox().south()][item->latLonAltBox().north()][item] = tiles;
        d->m_items[feature->zLevel()][item->latLonAltBox().west()][M_PI][item->latLonAltBox().south()][item->latLonAltBox().north()][item] = tiles;
    }
    else
    {
        d->m_items[feature->zLevel()][item->latLonAltBox().west()][item->latLonAltBox().east()][item->latLonAltBox().south()][item->latLonAltBox().north()][item] = tiles;
    }

    emit updatedTiles(tiles);

//    Marble::GeoDataStyle::Ptr style(new Marble::GeoDataStyle);

//    Marble::GeoDataPolyStyle polyStyle = style->polyStyle();
//    polyStyle.setFill( false );
//    polyStyle.setColor(Qt::black);
//    style->setPolyStyle( polyStyle );


//    Marble::GeoDataLineStyle lineStyle = style->lineStyle();
//    lineStyle.setColor(Qt::black);
//    lineStyle.setWidth(2);
//    style->setLineStyle(lineStyle);


//    auto it = tiles.constBegin();
//    auto itEnd = tiles.constEnd();

//    for(; it != itEnd; ++it)
//    {
//        auto itX = it.value().constBegin();
//        auto itXEnd = it.value().constEnd();

//        for(; itX != itXEnd; ++itX)
//        {
//            auto itY = itX.value().constBegin();
//            auto itYEnd = itX.value().constEnd();

//            for(; itY != itYEnd; ++itY)
//            {
//                TileId tile = TileId(0, it.key(), itX.key(), itY.key());
//                double radius = ( 1 << tile.tileLevel() ) *1 / 2.0;

//                double lonLeft   = ( tile.x() - radius ) / radius * M_PI;
//                double lonRight  = ( tile.x() - radius + 1 ) / radius * M_PI;

//                radius = ( 1 << tile.tileLevel() ) * 1 / 2.0;
//                double latBottom = atan( sinh( ( radius - tile.y() ) / radius * M_PI ) );
//                double latTop = atan( sinh( ( radius - tile.y() - 1 ) / radius * M_PI ) );

//                QRectF rect(lonLeft, latTop, lonRight - lonLeft, latBottom-latTop);
//                QPolygonF polygon;
//                polygon << rect.topLeft()*180/M_PI << rect.topRight()*180/M_PI << rect.bottomRight()*180/M_PI << rect.bottomLeft()*180/M_PI << rect.topLeft()*180/M_PI;

//                GeoDataLinearRing ring(polygon);

//                GeoDataPolygon *poly = new GeoDataPolygon();
//                poly->setOuterBoundary(ring);



//                Marble::GeoDataPlacemark* placemark = new Marble::GeoDataPlacemark();
//                placemark->setGeometry(poly);

//                placemark->setStyle(style);

//                QStringList values;
//                values.append(QObject::tr("X: %1").arg(tile.x()));
//                values.append(QObject::tr("Y: %1").arg(tile.y()));
//                placemark->setDescription(values.join(QStringLiteral("<br/>\n")));


//                GeoPolygonGraphicsItem *geoGraphicsItem = new GeoPolygonGraphicsItem( placemark, poly );

//                TileMap tileMap;
//                tileMap[it.key()][itX.key()][itY.key()] = TileStatus::Partially;

//                d->m_items[feature->zLevel()][lonLeft][lonRight][latBottom][latTop].insert(GeoGraphicsItemPtr(geoGraphicsItem), tiles);
//            }
//        }
//    }
}

TileMap
removeGeoItem(const GeoGraphicsItemPtr &geoItem,
              QMap<double /* z level */, QMap<double, QMap<double, QMap<double, QMap<double, QHash<GeoGraphicsItemPtr, TileMap> > > > > > &items,
              const GeoDataLatLonBox &latLonBox)
{
    TileMap tiles;
    double zLevel = geoItem->feature()->zLevel();

    tiles = items[zLevel][latLonBox.west()][latLonBox.east()][latLonBox.south()][latLonBox.north()][geoItem];
    items[zLevel][latLonBox.west()][latLonBox.east()][latLonBox.south()][latLonBox.north()].remove(geoItem);
    if(items[zLevel][latLonBox.west()][latLonBox.east()][latLonBox.south()][latLonBox.north()].isEmpty())
    {
        items[zLevel][latLonBox.west()][latLonBox.east()][latLonBox.south()].remove(latLonBox.north());
    }

    if(items[zLevel][latLonBox.west()][latLonBox.east()][latLonBox.south()].isEmpty())
    {
        items[zLevel][latLonBox.west()][latLonBox.east()].remove(latLonBox.south());
    }

    if(items[zLevel][latLonBox.west()][latLonBox.east()].isEmpty())
    {
        items[zLevel][latLonBox.west()].remove(latLonBox.east());
    }

    if(items[zLevel][latLonBox.west()].isEmpty())
    {
        items[zLevel].remove(latLonBox.west());
    }

    if(items[zLevel].isEmpty())
    {
        items.remove(zLevel);
    }

    return tiles;
}

namespace
{


void
uniteY(QMap<int /* y */, TileStatus> &map, const QMap<int /* y */, TileStatus> &otherMap)
{
    auto it = otherMap.constBegin();
    auto itEnd = otherMap.constEnd();
    for(; it != itEnd; ++it)
    {
        TileStatus status = map[it.key()];
        TileStatus otherStatus = it.value();
        if(status == TileStatus::Full || otherStatus == TileStatus::Full)
        {
            status = TileStatus::Full;
        }
        else if (status == TileStatus::Partially || otherStatus == TileStatus::Partially)
        {
            status = TileStatus::Partially;
        }

        map[it.key()] = status;
    }
}


void
uniteX(QMap<int /* x */, QMap<int /* y */, TileStatus> > &map, const QMap<int /* x */, QMap<int /* y */, TileStatus> > &otherMap)
{
    auto it = otherMap.constBegin();
    auto itEnd = otherMap.constEnd();
    for(; it != itEnd; ++it)
    {
        uniteY(map[it.key()], it.value());
    }

}


void
unite(TileMap &map, const TileMap &otherMap)
{
    auto it = otherMap.constBegin();
    auto itEnd = otherMap.constEnd();
    for(; it != itEnd; ++it)
    {
        uniteX(map[it.key()], it.value());
    }
}

}

bool
GeoGraphicsScene::removeItem( const GeoDataFeature* feature, TileMap &tiles )
{
    if(d->jobs.contains(feature))
    {
        delete d->jobs[feature];
        d->jobs.remove(feature);
    }

    QElapsedTimer timer;
    timer.start();

    foreach(const auto &geoItem, d->m_features_graphics_items[feature])
    {
        d->m_highlightedItems.remove(geoItem);
        d->m_selectedItems.remove(geoItem);

        TileMap tempTiles;

        GeoDataLatLonBox box = geoItem->latLonAltBox();

        if ( box.west() > box.east() )
        {
            // Handle boxes crossing the IDL by splitting it into two separate boxes
            GeoDataLatLonBox left;
            left.setWest( -M_PI );
            left.setEast( box.east() );
            left.setNorth( box.north() );
            left.setSouth( box.south() );

            GeoDataLatLonBox right;
            right.setWest( box.west() );
            right.setEast( M_PI );
            right.setNorth( box.north() );
            right.setSouth( box.south() );

            tempTiles = removeGeoItem(geoItem, d->m_items, left);
            unite(tempTiles, removeGeoItem(geoItem, d->m_items, right));
        }
        else
        {
            tempTiles = removeGeoItem(geoItem, d->m_items, box);
        }

        unite(tiles, tempTiles);

    }

    d->m_features_graphics_items.remove( feature );
    qWarning() << "removeItem finised elapsed" << timer.nsecsElapsed();

    return true;
}

bool GeoGraphicsScene::removeGraphicsItemsImpl(const GeoDataFeature *feature, TileMap &tiles)
{
    bool doUpdate = false;
    if( feature->nodeType() == GeoDataTypes::GeoDataPlacemarkType ||
        feature->nodeType() == GeoDataTypes::GeoDataGroundOverlayType)
    {
        doUpdate = removeItem( feature, tiles );
    }
    else if( feature->nodeType() == GeoDataTypes::GeoDataFolderType
             || feature->nodeType() == GeoDataTypes::GeoDataDocumentType )
    {
        const GeoDataContainer *container = static_cast<const GeoDataContainer*>( feature );
        foreach( const GeoDataFeature *child, container->featureList() )
        {
             doUpdate |= removeGraphicsItemsImpl( child, tiles );
        }
    }

    return doUpdate;

}

void GeoGraphicsScene::clear()
{
    emit aboutToClear();

    d->m_items.clear();
    d->m_features_graphics_items.clear();
    auto it = d->jobs.begin();
    auto itEnd = d->jobs.end();
    for(; it != itEnd; ++it)
    {
        delete it.value();
    }
    qDeleteAll(d->jobs);
    d->jobs.clear();
    d->m_selectedItems.clear();
    d->m_highlightedItems.clear();

    emit cleared();
}


bool
GeoGraphicsScene::removeGraphicsItems( const GeoDataFeature *feature )
{
    TileMap tiles;

    bool doUpdate = removeGraphicsItemsImpl(feature, tiles);

    if(doUpdate)
    {
        emit updatedTiles(tiles);
    }

    return doUpdate;
}

void
GeoGraphicsScene::createGraphicsItems( const GeoDataFeature *feature )
{
    // parse all child objects of the container
    if ( const GeoDataContainer *container = dynamic_cast<const GeoDataContainer*>( feature ) )
    {
        int rowCount = container->size();
        for ( int row = 0; row < rowCount; ++row )
        {
            createGraphicsItems( container->child( row ) );
        }
    }
    else
    {
        emit startAddItem(feature);
    }
}

void GeoGraphicsScene::setTextureLayer(GeoSceneTextureTileDataset *tileDataset)
{
    d->m_tileDataset = tileDataset;
}


AddItem::AddItem(const GeoDataFeature *feature, GeoSceneTextureTileDataset *tileDataset, QObject *parent)
    :   QObject(parent),
        job(new AddItemJob(tileDataset, feature))
{
    connect(&watcher, SIGNAL(finished()), this, SLOT(handleFinished()));
}

AddItem::~AddItem()
{
    disconnect(&watcher, SIGNAL(finished()), this, SLOT(handleFinished()));
    if(!watcher.isFinished())
    {
        job->cancel();
        watcher.waitForFinished();
    }
    delete job;
}

void AddItem::run()
{
    QFuture<void> future = QtConcurrent::run(job, &AddItemJob::run);
    watcher.setFuture(future);
}

void AddItem::handleFinished()
{
    qDebug() << "AddItem::handleFinished";

    emit finished(job->object(), job->item(), job->tiles());
}






}

//#include "moc_GeoGraphicsScene.cpp"
