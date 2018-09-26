//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2016      Dennis Nienhüser <nienhueser@kde.org>
//

//Self
#include "OsmConverter.h"

//Marble
#include "geodata/data/GeoDataDocument.h"
#include "geodata/data/GeoDataPlacemark.h"
#include "geodata/data/GeoDataGeometry.h"
#include "geodata/data/GeoDataPoint.h"
#include "geodata/data/GeoDataPolygon.h"
#include "geodata/data/GeoDataRelation.h"
#include "geodata/data/GeoDataLinearRing.h"
#include "geodata/data/GeoDataMultiGeometry.h"
#include "osm/OsmPlacemarkData.h"
#include "osm/OsmObjectManager.h"

#include <QDebug>

namespace Marble
{

void OsmConverter::read(const GeoDataDocument *document)
{
    m_nodes.clear();
    m_ways.clear();
    m_relations.clear();

    // Writing all the component nodes ( points, nodes of polylines, nodes of polygons )
    for (auto feature: document->featureList()) {
        if (auto placemark = geodata_cast<GeoDataPlacemark>(feature)) {
            // If the placemark's osmData is not complete, it is initialized by the OsmObjectManager
            OsmObjectManager::initializeOsmData( placemark );
            const OsmPlacemarkData & osmData = placemark->osmData();

            if (geodata_cast<GeoDataPoint>(placemark->geometry())) {
                m_nodes << OsmConverter::Node(placemark->coordinate(), osmData);
            } else if (const auto lineString = geodata_cast<GeoDataLineString>(placemark->geometry())) {
                for(int i = 0; i < lineString->size(); ++i)
                {
                    auto const &coordinates = lineString->at(i);
                    m_nodes << OsmConverter::Node(coordinates, osmData.nodeReference(coordinates));

                }
                m_ways << OsmConverter::Way(lineString, osmData);
            } else if (const auto linearRing = geodata_cast<GeoDataLinearRing>(placemark->geometry())) {
                processLinearRing(linearRing, osmData);
            } else if (const auto polygon = geodata_cast<GeoDataPolygon>(placemark->geometry())) {
                processPolygon(polygon, osmData, placemark);
            }
        } else if (const auto placemark = geodata_cast<GeoDataRelation>(feature)) {
            m_relations.append(OsmConverter::Relation(placemark, placemark->osmData()));
        }
    }

    // Sort by id ascending since some external tools rely on that
    std::sort(m_nodes.begin(), m_nodes.end(), [] (const Node &a, const Node &b) { return a.second.id() < b.second.id(); });
    std::sort(m_ways.begin(), m_ways.end(), [] (const Way &a, const Way &b) { return a.second.id() < b.second.id(); });
    std::sort(m_relations.begin(), m_relations.end(), [] (const Relation &a, const Relation &b) { return a.second.id() < b.second.id(); });
}

const OsmConverter::Nodes &OsmConverter::nodes() const
{
    return m_nodes;
}

const OsmConverter::Ways &OsmConverter::ways() const
{
    return m_ways;
}

const OsmConverter::Relations &OsmConverter::relations() const
{
    return m_relations;
}

void OsmConverter::processLinearRing(GeoDataLinearRing *linearRing,
                                     const OsmPlacemarkData& osmData)
{
    for(int i = 0; i < linearRing->size(); ++i)
    {
        auto coordinates = linearRing->at(i);
        m_nodes << OsmConverter::Node(coordinates, osmData.nodeReference(coordinates));
    }
    m_ways << OsmConverter::Way(linearRing, osmData);
}

void OsmConverter::processPolygon(GeoDataPolygon *polygon,
                                  const OsmPlacemarkData& osmData,
                                  GeoDataPlacemark* placemark)
{
    int index = -1;

    // Writing all the outerRing's nodes
    const GeoDataLinearRing &outerRing = polygon->outerBoundary();
    const OsmPlacemarkData outerRingOsmData = osmData.memberReference( index );
    for(int i = 0; i < outerRing.size(); ++i)
    {
        GeoDataCoordinates coordinates = outerRing.at(i);
        m_nodes << qMakePair(coordinates, outerRingOsmData.nodeReference(coordinates));
    }
    // Writing all nodes for each innerRing
    for (auto const &innerRing: polygon->innerBoundaries() ) {
        ++index;
        const OsmPlacemarkData innerRingOsmData = osmData.memberReference( index );
        for(int i = 0; i < innerRing.size(); ++i)
        {
            auto coordinates = innerRing.at(i);
            m_nodes << OsmConverter::Node(coordinates, innerRingOsmData.nodeReference(coordinates));
        }

        m_ways << OsmConverter::Way(&innerRing, innerRingOsmData);
    }
    m_relations.append(OsmConverter::Relation(placemark, osmData));
}

}

