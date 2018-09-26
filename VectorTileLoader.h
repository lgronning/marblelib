#ifndef VECTORTILELOADER_H
#define VECTORTILELOADER_H

#include "AbstractTileLoader.h"
#include "graphicsview/GeoGraphicsItem.h"
#include <QtCore/QCache>
#include <QtCore/QMap>
#include <QtGui/QPixmap>
#include <QtCore/QFutureWatcher>
#include <QtCore/QTemporaryDir>
#include <QtCore/QQueue>
#include <atomic>

class QByteArray;
class QFutureWatcherBase;
class QThreadPool;

namespace Marble
{

class GeoGraphicsScene;

class GeoGraphicsItem;
typedef QSharedPointer<GeoGraphicsItem> GeoGraphicsItemPtr;

class GeometryTile;
typedef QSharedPointer<GeometryTile> GeometryTilePtr;


class RenderJob
{
public:
    RenderJob(Projection projection, qreal centerLongitude, qreal centerLatitude, int radius,  const QSize &size , QList<GeoGraphicsItemPtr> items, const TileId &tileId, QString tileDir);

    ~RenderJob();

    void
    run();

    const QImage&
    tile() const
    {
        return aTile;
    }


    const TileId&
    tileId() const
    {
        return aTileId;
    }

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
private:
    friend class TileRenderJob;

    std::atomic<bool> aCancel;
    const Projection aProjection;
    const double aCenterLongitude;
    const double aCenterLatitude;
    const int aRadius;
    const QSize aSize;
    const QList< GeoGraphicsItemPtr > aItems;
    const TileId aTileId;
    const QString aTileDir;

    QImage aTile;

};

class TileRenderJob  : public QObject
{
    Q_OBJECT

public:
    TileRenderJob(Projection projection, qreal centerLongitude, qreal centerLatitude, int radius,  const QSize &size, QList< GeoGraphicsItemPtr > items , const TileId &tileId, const QString &tileDir, QObject *parent);

    
    ~TileRenderJob() override;

    void
    run();

    bool
    isCancelled() const
    {
        return job->isCancelled();
    }

public slots:
    void
    handleFinished();

signals:
    void
    finished(const TileId &tile, const QImage &image);

private:
    friend class VectorTileLoader;

    QFutureWatcher<void> watcher;
    RenderJob *job;

};

class GeometryTile;

class VectorTileLoader: public AbstractTileLoader
{
    Q_OBJECT
public:
    explicit VectorTileLoader(GeoGraphicsScene *scene );
    ~VectorTileLoader() override;

    QImage
    loadTileImage( GeoSceneTextureTileDataset const *textureData, TileId const & tileId, DownloadUsage const ) override;

    void
    createTile( GeoSceneTileDataset const *tileData, TileId const &, DownloadUsage ) override;

    TileStatus
    tileStatus( GeoSceneTileDataset const *tileData, const TileId &tileId ) override;

    void
    clear();

    void
    resetTilehash();

    void
    setTilesUsed(const QList<TileId> &tiles);

    void
    cleanupTilehash();

public slots:
    void
    handleFinished(const TileId &tileId, const QImage &tile);

    void
    renderTile( GeoSceneTileDataset const *tileData, TileId const &);

    void
    setTileExpired(const GeoSceneTileDataset *tileData, TileMap tileMap);

signals:
    void
    startRenderTile( GeoSceneTileDataset const *tileData, TileId const &);

private:
    void
    triggerCreate( GeoSceneTileDataset const *tileData, TileId const &, DownloadUsage const );

    void
    addTile(const TileId &tileId, const QImage &tile);

    GeoGraphicsScene *m_scene;
    QHash<TileId, GeometryTile*>  m_tilesOnDisplay;
    QHash<TileId, GeometryTile*>  m_tileCache;
    QQueue<TileId> m_cacheTiles;
    QSet<TileId> m_emptyTiles;
    qint64 m_cacheSize;

    QMap<TileId, TileRenderJob*> tileRenderJobMap;

    QTemporaryDir *dir;
    QMutex *mutex;
};


}

#endif // VECTORTILELOADER_H
