//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008      Patrick Spendrin <ps_ml@gmx.de>
//


#ifndef MARBLE_GEODATALINESTYLE_H
#define MARBLE_GEODATALINESTYLE_H


#include <QFont>
#include <QVector>

#include "geodata/data/GeoDataColorStyle.h"

#include "geodata/geodata_export.h"

namespace Marble
{

class GeoDataLineStylePrivate;

/**
 * @short specifies the style how lines are drawn
 *
 * A GeoDataLineStyle specifies how the <b>name</b> of a
 * GeoDataFeature is drawn in the viewer. A custom color, color mode
 * (both inherited from GeoDataColorStyle) and width for the width
 * of the line.
 */
class GEODATA_EXPORT GeoDataLineStyle : public GeoDataColorStyle
{
  public:
    /// Construct a new GeoDataLineStyle
    GeoDataLineStyle();
    GeoDataLineStyle( const GeoDataLineStyle& other );
    /**
     * @brief Construct a new GeoDataLineStyle
     * @param  color  the color to use when showing the name @see GeoDataColorStyle
     */
    explicit GeoDataLineStyle( const QColor &color );

    ~GeoDataLineStyle() override;

    /**
    * @brief assignment operator
    */
    GeoDataLineStyle& operator=( const GeoDataLineStyle& other );

    bool operator==( const GeoDataLineStyle &other ) const;
    bool operator!=( const GeoDataLineStyle &other ) const;

    /// Provides type information for downcasting a GeoData
    const char* nodeType() const override;

    /**
     * @brief Set the width of the line
     * @param  width  the new width
     */
    void setWidth( const double &width );
    /**
     * @brief Return the current width of the line
     * @return the current width
     */
    double width() const;
    
    /**
     * @brief Set the physical width of the line (in meters)
     * @param  width  the new width
     */
    void setPhysicalWidth( const double &realWidth );
    /**
     * @brief Return the current physical width of the line
     * @return the current width
     */
    double physicalWidth() const;

    /**
     * @brief Set whether the line has a cosmetic 1 pixel outline
     */
    void setCosmeticOutline( bool enabled );
    /**
     * @brief Return whether the line has a cosmetic 1 pixel outline
     */
    bool cosmeticOutline() const;

    
    /**
     * @brief Set pen cap style
     * @param  style cap style
     */
    void setCapStyle( Qt::PenCapStyle style );
    
    /**
     * @brief Return the current pen cap style
     * @return the current pen cap style
     */
    Qt::PenCapStyle capStyle() const;
    
    /**
     * @brief Set pen cap style
     * @param  style cap style
     */
    void setPenStyle( Qt::PenStyle style );
    
    /**
     * @brief Return the current pen cap style
     * @return the current pen cap style
     */
    Qt::PenStyle penStyle() const;
    
     /**
     * @brief Set whether to draw the solid background
     * @param bool
     */
    void setBackground( bool background );
    
    /**
     * @brief Return true if background get drawn
     * @return 
     */
    bool background() const;

    /**
     * @brief Sets the dash pattern
     * @param pattern dash pattern
     */
    void setDashPattern( const QVector<qreal>& pattern );
    
    /**
     * @brief Return the current dash pattern
     * @return the current dash pattern
     */
    QVector<qreal> dashPattern() const;
    
    /**
     * @brief Sets the point pixmap
     * @param pixmap point pixmap
     */
    void
    setPointPixmap( const QPixmap &pixmap );

    /**
     * @brief Return the point pixmap
     * @return the point pixmap
     */
    const QPixmap &
    pointPixmap() const;

    /**
     * @brief Return the scaled point pixmap
     * @return the scaled point pixmap
     */
    const QPixmap &
    scaledPointPixmap() const;

    bool
    alwaysUsePointPixmap() const;

    void
    setAlwaysUsePointPixmap(bool value) const;

  private:
    GeoDataLineStylePrivate * const d;
};

}

#endif
