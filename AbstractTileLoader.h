#ifndef ABSTRACTTILELOADER_H
#define ABSTRACTTILELOADER_H

#include <QObject>
#include <QString>
#include <QImage>

#include "TileId.h"
#include "geodata/data/GeoDataContainer.h"
#include "PluginManager.h"
#include "MarbleGlobal.h"

namespace Marble
{

class GeoDataDocument;
class GeoSceneTileDataset;
class GeoSceneTextureTileDataset;
class ParsingRunnerManager;

class AbstractTileLoader: public QObject
{
    Q_OBJECT

 public:
    enum TileStatus {
        Missing,
        Expired,
        Available
    };

    AbstractTileLoader()
    {}

    
    ~AbstractTileLoader() override
    {

    }

    virtual
    QImage
    loadTileImage( GeoSceneTextureTileDataset const *textureData, TileId const & tileId, DownloadUsage const ) = 0;

    virtual
    void
    createTile( GeoSceneTileDataset const *tileData, TileId const &, DownloadUsage ) = 0;

    virtual
    TileStatus
    tileStatus( GeoSceneTileDataset const *tileData, const TileId &tileId ) = 0;

    int
    maximumTileLevel( GeoSceneTileDataset const & tileData );

Q_SIGNALS:
    void tileCompleted( TileId const & tileId, QImage const & tileImage );

protected:
    virtual
    QImage
    scaledLowerLevelTile( GeoSceneTextureTileDataset const * textureData, TileId const & );
};

}

#endif // ABSTRACTTILELOADER_H
