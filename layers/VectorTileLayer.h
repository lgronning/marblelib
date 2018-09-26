/*
 This file is part of the Marble Virtual Globe.

 This program is free software licensed under the GNU LGPL. You can
 find a copy of this license in LICENSE.txt in the top directory of
 the source code.

 Copyright 2008      Patrick Spendrin  <ps_ml@gmx.de>
 Copyright 2010      Thibaut Gridel  <tgridel@free.fr>
 Copyright 2012      Ander Pijoan <ander.pijoan@deusto.es>
 Copyright 2013      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
*/

#ifndef MARBLE_VECTORTILELAYER_H
#define MARBLE_VECTORTILELAYER_H

#include "LayerInterface.h"
#include <QObject>
#include <QRunnable>

#include "MarbleGlobal.h"
#include "MarbleModel.h"
#include "geodata/data/GeoDataDocument.h"
#include "geodata/data/GeoDataLatLonAltBox.h"
#include "TileId.h"
#include "TileLoader.h"

#include <QSize>
#include <QThreadPool>


class QImage;
class QRegion;
class QRect;

namespace Marble
{

class GeoPainter;
class GeoSceneGroup;
class GeoSceneVectorTileDataset;
class HttpDownloadManager;
class SunLocator;
class ViewportParams;
class VectorTileModel;

class VectorTileLayer : public QObject, public LayerInterface
{
    Q_OBJECT

 public:
    VectorTileLayer( HttpDownloadManager *downloadManager,
                  const PluginManager *pluginManager,
                  GeoDataTreeModel *treeModel);

    ~VectorTileLayer() override;

    QStringList renderPosition() const override;

    int tileZoomLevel() const;

    QString runtimeTrace() const override;

Q_SIGNALS:
    void tileLevelChanged(int tileLevel);

 public Q_SLOTS:
    bool render( GeoPainter *painter, ViewportParams *viewport,
                 const QString &renderPos = QStringLiteral("NONE"),
                 GeoSceneLayer *layer = 0 ) override;

    void setMapTheme( const QVector<const GeoSceneVectorTileDataset *> &textures, const GeoSceneGroup *textureLayerSettings );

    void reset();

 private:

    class Private
    {
    public:
        Private(HttpDownloadManager *downloadManager,
                const PluginManager *pluginManager,
                VectorTileLayer *parent,
                GeoDataTreeModel *treeModel);

        ~Private();

        void updateTile(const TileId &tileId, GeoDataDocument* document);
        void updateTextureLayers();

    public:
        VectorTileLayer  *const m_parent;
        TileLoader m_loader;
        QVector<VectorTileModel *> m_texmappers;
        QVector<VectorTileModel *> m_activeTexmappers;
        const GeoSceneGroup *m_textureLayerSettings;

        // TreeModel for displaying GeoDataDocuments
        GeoDataTreeModel *const m_treeModel;

        QThreadPool m_threadPool; // a shared thread pool for all layers to keep CPU usage sane
    };

    Q_PRIVATE_SLOT( d, void updateTextureLayers() )
    Q_PRIVATE_SLOT( d, void updateTile(const TileId &tileId, GeoDataDocument* document) )

 private:
    Private *const d;

};

}

#endif // MARBLE_VECTORTILELAYER_H
