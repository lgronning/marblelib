//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008      Patrick Spendrin  <ps_ml@gmx.de>
// Copyright 2010      Thibaut Gridel  <tgridel@free.fr>
// Copyright 2011-2012 Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#ifndef MARBLE_GEOMETRYLAYER_H
#define MARBLE_GEOMETRYLAYER_H

#include <QObject>
#include <QModelIndex>
#include "LayerInterface.h"
#include "geodata/data/GeoDataCoordinates.h"
#include "graphicsview/GeoGraphicsItem.h"

class QAbstractItemModel;
class QModelIndex;
class QPoint;
class QColor;

namespace Marble
{
class GeoPainter;
class GeoDataFeature;
class ViewportParams;
class GeometryLayerPrivate;
class GeoDataPlacemark;
class SunLocator;
class TileId;

class GeometryLayer : public QObject, public LayerInterface
{
    Q_OBJECT
public:
    explicit GeometryLayer( const QAbstractItemModel *model,
                            const SunLocator *sunLocator );
    ~GeometryLayer() override;

    QStringList renderPosition() const override;

    
    bool
    render( GeoPainter *painter, ViewportParams *viewport,
                         const QString& renderPos = QStringLiteral("NONE"),
                         GeoSceneLayer * layer = 0 ) override;

    void
    setProjection(Projection projection);

    
    QString
    runtimeTrace() const override;

    const GeoDataFeature *
    whichFeatureAt(const QPoint& curpos, const ViewportParams * viewport);

    void
    setNeedsUpdate();

public Q_SLOTS:
    void addPlacemarks( QModelIndex index, int first, int last );
    void removePlacemarks( QModelIndex index, int first, int last );
    void resetCacheData();

    /**
     * Finds all placemarks that contain the clicked point.
     *
     * The placemarks under the clicked position may
     * have their styleUrl set to a style map which
     * doesn't specify any highlight styleId. Such
     * placemarks will be fletered out in GeoGraphicsScene
     * and will not be highlighted.
     */
    void
    handleHighlight(const GeoDataCoordinates &coordinate, const ViewportParams * viewport);

    void
    setHighlightFeatures(const QVector<GeoDataFeature *> &features);

    void
    handleSelected(const GeoDataCoordinates &coordinate, const ViewportParams * viewport);

    void
    setSelectedFeatures(const QVector<GeoDataFeature *> &features);

    void
    visibleLatLonAltBoxChanged(const GeoDataLatLonAltBox &latLonBox);

    void
    radiusChanged(int radius);

Q_SIGNALS:
    void tileLevelChanged( int level );

    void repaintNeeded();


    /**
     * @p selectedPlacemarks may contain placemarks which don't have
     * their styleUrl set to id of the style map which specifies
     * a highlight styleId. Such placemarks will be filtered out
     * in GeoGraphicsScene which will query for placemark->styleUrl()
     * to decide whether the placemark should be highlighted ot not.
     */
    void
    highlightedFeaturesChanged( const QVector<GeoDataFeature*>& clickedPlacemarks );

    void
    selectedFeaturesChanged( const QVector<GeoDataFeature*>& clickedPlacemarks );

private slots:
    void
    updateTile(const TileId &tileId, const QImage &image);

    void
    updateTileStatus(const TileMap &tiles);

    void
    startGenerateNextLevel();

private:
    const GeoDataFeature *
    getFeature(const GeoDataCoordinates &clickedPoint, const ViewportParams *viewport);

    QVector<GeoDataFeature*>
    getFeatures( const GeoDataCoordinates &coordinate, const ViewportParams *viewport);

    GeometryLayerPrivate *d;
};

} // namespace Marble
#endif // MARBLE_GEOMETRYLAYER_H
