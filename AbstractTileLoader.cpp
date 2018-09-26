#define QT_NO_DEBUG_OUTPUT
#include "AbstractTileLoader.h"
#include "geodata/scene/GeoSceneTileDataset.h"
#include "geodata/parser/GeoSceneTypes.h"
#include <QtCore/QDebug>

Q_DECLARE_METATYPE( Marble::DownloadUsage )

namespace Marble
{



int
AbstractTileLoader::maximumTileLevel(const GeoSceneTileDataset &tileData)
{
    return tileData.maximumTileLevel();
}



QImage
AbstractTileLoader::scaledLowerLevelTile( const GeoSceneTextureTileDataset * textureData, TileId const & id )
{
    qDebug() << Q_FUNC_INFO << id;

    int const minimumLevel = textureData->minimumTileLevel();
    for ( int level = qMax<int>( 0, id.tileLevel() - 1 ); level >= 0; --level ) {
        if (level > 0 && level < minimumLevel) {
            continue;
        }
        int const deltaLevel = id.tileLevel() - level;
        QImage toScale;
        if(deltaLevel != 0)
        {
            TileId const replacementTileId( id.mapThemeIdHash(), level,
                                            id.x() >> deltaLevel, id.y() >> deltaLevel );

            if(tileStatus(textureData, replacementTileId) == AbstractTileLoader::Available)
            {
                toScale = loadTileImage(textureData, replacementTileId, DownloadBrowse);
            }
        }

        if ( deltaLevel == 0 || (level == 0 && (toScale.isNull() || toScale.size() != textureData->tileSize()) ) ) {
            qDebug() << "No level zero tile installed in map theme dir. Falling back to a transparent image for now.";
            QSize tileSize = textureData->tileSize();
            Q_ASSERT( !tileSize.isEmpty() ); // assured by textureLayer
            toScale = QImage( tileSize, QImage::Format_ARGB32_Premultiplied );
            toScale.fill(Qt::transparent );

            return toScale;
        }

        if ( !toScale.isNull() && toScale.size() == textureData->tileSize()) {
            // which rect to scale?
            int const restTileX = id.x() % ( 1 << deltaLevel );
            int const restTileY = id.y() % ( 1 << deltaLevel );
            int const partWidth = qMax(1, toScale.width() >> deltaLevel);
            int const partHeight = qMax(1, toScale.height() >> deltaLevel);
            int const startX = restTileX * partWidth;
            int const startY = restTileY * partHeight;
            qDebug() << "QImage::copy:" << startX << startY << partWidth << partHeight;
            QImage const part = toScale.copy( startX, startY, partWidth, partHeight );
            qDebug() << "QImage::scaled:" << toScale.size();
            return part.scaled( toScale.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
        }
    }

    return QImage();
}

}
