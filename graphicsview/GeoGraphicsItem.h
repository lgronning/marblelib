//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Bastian Holst <bastianholst@gmx.de>
// Copyright 2009      Andrew Manson <g.real.ate@gmail.com>
//

#ifndef MARBLE_GEOGRAPHICSITEM_H
#define MARBLE_GEOGRAPHICSITEM_H

// Marble
#include "marble_export.h"
#include "TileId.h"
#include "geodata/data/GeoDataStyle.h"
#include <atomic>

class QString;

namespace Marble
{
class GeoLabelPlaceHandler;
class GeoDataFeature;
class GeoDataLatLonAltBox;
class GeoGraphicsItemPrivate;
class GeoPainter;
class ViewportParams;
class GeoGraphicsItem;

typedef QSharedPointer<Marble::GeoGraphicsItem> GeoGraphicsItemPtr;

enum TileStatus
{
    Full,
    Partially,
};

typedef QMap<int /* zoom level */, QMap<int /* x */, QMap<int /* y */, TileStatus> > >  TileMap;


class MARBLE_EXPORT GeoGraphicsItem
{
 public:
    explicit GeoGraphicsItem( const GeoDataFeature *feature );
    virtual ~GeoGraphicsItem();

    enum GeoGraphicsItemFlag {
        NoOptions = 0x0,
        ItemIsMovable = 0x1,
        ItemIsSelectable = 0x2
    };

    Q_DECLARE_FLAGS(GeoGraphicsItemFlags, GeoGraphicsItemFlag)

    bool
    visible() const;

    /**
     * Get the GeoGraphicItemFlags value that describes which flags are set on
     * this item. @see QFlags
     */
    GeoGraphicsItemFlags flags() const;

    /**
     * Set or unset a single flag
     * @param enabled sets if the flag is to be set or unset
     */
    void setFlag( GeoGraphicsItemFlag flag, bool enabled = true );

    /**
     * Replace all of the current flags.
     * @param flags is the new value for this item's flags.
     */
    void setFlags( GeoGraphicsItemFlags flags );

    /**
     * Returns the minim zoom level on which item will be active.
     */
    int minZoomLevel() const;

    /**
     * Sets the minimum zoom level
     */
    void setMinZoomLevel( int zoomLevel );

    /**
     * Returns the placemark for that item.
     */
    const GeoDataFeature*
    feature() const;

    /**
     * Returns the bounding box covered by the item.
     */
    virtual
    const GeoDataLatLonAltBox&
    latLonAltBox() const;

    /**
     * Set the box used to determine if an item is active or inactive. If an empty box is passed
     * the item will be shown in every case.
     */
    void setLatLonAltBox( const GeoDataLatLonAltBox& latLonAltBox );
    
    /**
     * Returns the style of item.
     */
    GeoDataStyle::ConstPtr style() const;

    /**
     * Returns the z value of the item
     */
    qreal zLevel() const;

    /**
     * Set the z value of the item
     */
    void setZLevel( qreal z );

    virtual
    void
    renderGeometry( GeoPainter* painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style ) = 0;

    virtual
    void
    renderIcons( GeoPainter* painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style ) = 0;

    virtual
    void
    renderLabels( GeoPainter* painter, const ViewportParams *viewport, GeoDataStyle::ConstPtr style, GeoLabelPlaceHandler &placeHandler ) = 0;

    virtual
    void
    getTiles(GeoSceneTextureTileDataset *tileDataset, const TileId &tile, int zoomLevel, TileMap &tiles, std::atomic<bool> &aCancel)
    {}

 protected:
    GeoGraphicsItemPrivate *const d;
};

} // Namespace Marble
Q_DECLARE_OPERATORS_FOR_FLAGS(Marble::GeoGraphicsItem::GeoGraphicsItemFlags)

#endif
