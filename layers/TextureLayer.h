//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010,2011 Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#ifndef MARBLE_MARBLETEXTURELAYER_H
#define MARBLE_MARBLETEXTURELAYER_H

#include "LayerInterface.h"
#include <QObject>

#include "MarbleGlobal.h"
#include "geodata/scene/GeoSceneTileDataset.h"
#include "TileLoader.h"
#include "StackedTileLoader.h"

#include <QSize>
#include <QTimer>
#include <QSortFilterProxyModel>

class QAbstractItemModel;
class QImage;
class QRegion;
class QRect;

namespace Marble
{

class GeoPainter;
class GeoDataDocument;
class GeoSceneGroup;
class GeoSceneTextureTileDataset;
class HttpDownloadManager;
class SunLocator;
class ViewportParams;
class PluginManager;
class TextureMapperInterface;
class TextureColorizer;


class MARBLE_EXPORT TextureLayer : public QObject, public LayerInterface
{
    Q_OBJECT

 public:
    TextureLayer( HttpDownloadManager *downloadManager,
                  PluginManager* pluginManager,
                  const SunLocator *sunLocator);

    ~TextureLayer() override;

    QStringList renderPosition() const override;

    void addSeaDocument( const GeoDataDocument *seaDocument );

    void addLandDocument( const GeoDataDocument *landDocument );

    int textureLayerCount() const;

    bool showSunShading() const;
    bool showCityLights() const;

    /**
     * @brief Return the current tile zoom level. For example for OpenStreetMap
     *        possible values are 1..18, for BlueMarble 0..6.
     */
    int tileZoomLevel() const;

    QSize tileSize() const;

    GeoSceneTileDataset::Projection tileProjection() const;

    int tileColumnCount( int level ) const;
    int tileRowCount( int level ) const;

    qint64 volatileCacheLimit() const;

    int preferredRadiusCeil( int radius ) const;
    int preferredRadiusFloor( int radius ) const;

    QString runtimeTrace() const override;

    bool render( GeoPainter *painter, ViewportParams *viewport,
                         const QString &renderPos = QStringLiteral("NONE"),
                         GeoSceneLayer *layer = 0 ) override;

public Q_SLOTS:
    void setShowRelief( bool show );

    void setShowSunShading( bool show );

    void setShowCityLights( bool show );

    void setShowTileId( bool show );

    /**
     * @brief  Set the Projection used for the map
     * @param  projection projection type (e.g. Spherical, Equirectangular, Mercator)
     */
    void setProjection( Projection projection );

    void setNeedsUpdate();

    void setMapTheme( const QVector<const GeoSceneTextureTileDataset *> &textures, const GeoSceneGroup *textureLayerSettings, const QString &seaFile, const QString &landFile );

    void setVolatileCacheLimit( quint64 kilobytes );

    void reset();

    void reload();

    void downloadStackedTile( const TileId &stackedTileId );

 Q_SIGNALS:
    void tileLevelChanged( int );
    void repaintNeeded();

 private:


    class Private
    {
    public:
        Private( HttpDownloadManager *downloadManager,
                 PluginManager* pluginManager,
                 const SunLocator *sunLocator,
                 TextureLayer *parent );

        void
        requestDelayedRepaint();

        void
        updateTextureLayers();

        void
        updateTile( const TileId &tileId, const QImage &tileImage );

        void
        addGroundOverlays( QModelIndex parent, int first, int last );

        void
        removeGroundOverlays( QModelIndex parent, int first, int last );

        void
        changedGroundOverlays(const QModelIndex &start, const QModelIndex &);

        void
        resetGroundOverlaysCache();

    public:
        TextureLayer  *const m_parent;
        const SunLocator *const m_sunLocator;
        TileLoader m_loader;
        MergedLayerDecorator m_layerDecorator;
        StackedTileLoader    m_tileLoader;
        GeoDataCoordinates m_centerCoordinates;
        int m_tileZoomLevel;
        TextureMapperInterface *m_texmapper;
        TextureColorizer *m_texcolorizer;
        QVector<const GeoSceneTextureTileDataset *> m_textures;
        const GeoSceneGroup *m_textureLayerSettings;
        QString m_runtimeTrace;
        // For scheduling repaints
        QTimer           m_repaintTimer;
    };

    Q_PRIVATE_SLOT( d, void requestDelayedRepaint() )
    Q_PRIVATE_SLOT( d, void updateTextureLayers() )
    Q_PRIVATE_SLOT( d, void updateTile( const TileId &tileId, const QImage &tileImage ) )

 private:
    Private *const d;
};

}

#endif
