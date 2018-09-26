//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010      Bastian Holst <bastianholst@gmx.de>
// Copyright 2011      Konstantin Oblaukhov <oblaukhov.konstantin@gmail.com>
//

#ifndef MARBLE_GEOGRAPHICSSCENE_H
#define MARBLE_GEOGRAPHICSSCENE_H

#include "marble_export.h"
#include "MarbleGlobal.h"
#include "graphicsview/GeoGraphicsItem.h"
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtGui/QColor>
#include <QtCore/QSharedPointer>
#include <QtCore/QFutureWatcher>
#include <atomic>

namespace Marble
{

class GeoDataFeature;
class GeoDataLatLonBox;
class GeoGraphicsScenePrivate;
class GeoDataDocument;
class GeoDataStyleMap;
class GeoDataPlacemark;
class GeoDataOverlay;
class GeoDataGeometry;
class GeoDataFeature;
class TileId;
class AddItemJob;

class AddItem : public QObject
{
    Q_OBJECT

public:
    AddItem(const GeoDataFeature *feature, GeoSceneTextureTileDataset *tileDataset, QObject *parent);

    
    ~AddItem() override;

    void
    run();

public slots:
    void
    handleFinished();

signals:
    void
    finished(const GeoDataFeature *feature, const GeoGraphicsItemPtr &item, const TileMap &tiles);

private:
    QFutureWatcher<void> watcher;
    AddItemJob *job;
};


/**
 * @short This is the home of all GeoGraphicsItems to be shown on the map.
 */
class MARBLE_EXPORT GeoGraphicsScene : public QObject
{
    Q_OBJECT

public:
    /**
     * Creates a new instance of GeoGraphicsScene
     * @param parent the QObject parent of the Scene
     */
    explicit GeoGraphicsScene( QObject *parent = 0 );
    ~GeoGraphicsScene() override;

    /**
     * @brief Remove all items from the GeoGraphicsScene
     */
    void clear();

    /**
     * @brief Get the list of items in the specified Box
     *
     * @param box The box around the items.
     * @param maxZoomLevel The max zoom level of tiling
     * @return The list of items in the specified box in no specific order.
     */
    QList<GeoGraphicsItemPtr>
    items(const GeoDataLatLonBox &box , bool highlightedItems = false) const;

    QList<GeoGraphicsItemPtr>
    items(const GeoSceneTileDataset *tileData, const TileId tileId, bool highlightedItems = false) const;

    bool
    removeGraphicsItems(const GeoDataFeature *feature);

    void
    createGraphicsItems(const GeoDataFeature *feature);

    void
    setTextureLayer(GeoSceneTextureTileDataset *tileDataset);

public Q_SLOTS:
    void
    applyHighlight( const QVector<GeoDataFeature*>& );

    void
    applySelected( const QVector<GeoDataFeature*>& );

private slots:
    void
    doAddItem(const GeoDataFeature *feature);

    void
    handleFinished(const GeoDataFeature *feature, const GeoGraphicsItemPtr &item, const TileMap &tiles);

Q_SIGNALS:
    void
    repaintNeeded();

    void
    updatedTiles(const TileMap &tiles);

    void
    aboutToClear();

    void
    cleared();

    void
    startAddItem(const GeoDataFeature *feature);

private:
    /**
     * @brief Add an item to the GeoGraphicsScene
     * Adds the item @p item to the GeoGraphicsScene
     */
    void addItem( const GeoGraphicsItemPtr &item );

    /**
     * @brief Remove all concerned items from the GeoGraphicsScene
     * Removes all items which are associated with @p object from the GeoGraphicsScene
     */
    bool removeItem( const GeoDataFeature *feature, TileMap &tiles );

    bool
    removeGraphicsItemsImpl( const GeoDataFeature *feature, TileMap &tiles );

    QList<GeoGraphicsItemPtr>
    itemsImpl(const GeoDataLatLonBox &box, bool highlightedItems) const;

    QSet<GeoGraphicsItemPtr>
    itemsTileImpl(const GeoDataLatLonBox &box, int tileLevel) const;

    void
    applyImpl(const QVector<GeoDataFeature*>& features, QSet<GeoGraphicsItemPtr> &oldItems);

    QList<GeoGraphicsItemPtr>
    getItems(const QMap<int, QPair<QPoint, QPoint> > &tileRects, const GeoDataLatLonBox &box, bool highlightedItems) const;

    GeoGraphicsScenePrivate * const d;
};
}
#endif // MARBLE_GEOGRAPHICSSCENE_H
