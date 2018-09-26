
//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2007      Andrew Manson    <g.real.ate@gmail.com>
// Copyright 2011      Bernhard Beschow <bbeschow@cs.tu-berlin.de
//

#ifndef IMAGEMAPPERCONTEXT_H
#define IMAGEMAPPERCONTEXT_H

#include <QSize>
#include <QImage>

#include "geodata/scene/GeoSceneTileDataset.h"
#include "geodata/scene/GeoSceneTextureTileDataset.h"
#include "MarbleMath.h"
#include "MathHelper.h"

namespace Marble
{

class ViewportParams;


class ImageMapperContext
{
public:
    ImageMapperContext(const QImage *image, const Marble::GeoDataLatLonBox &overlayLatLonBox);

    ~ImageMapperContext();

    bool pixelValueF( const qreal lon, const qreal lat,
                 QRgb* const scanLine );
    bool pixelValue( const qreal lon, const qreal lat,
                QRgb* const scanLine );

    void
    pixelValueApproxF( const qreal lon, const qreal lat,
                       QRgb *scanLine, const int n );

    void
    pixelValueApprox( const qreal lon, const qreal lat,
                      QRgb *scanLine, const int n );

    static
    int
    interpolationStep( const ViewportParams *viewport, MapQuality mapQuality );

    int
    globalWidth() const;

    int
    globalHeight() const;

private:
    // Checks whether the pixelValueApprox method will make use of more than
    // one tile
    bool isOutOfTileRange( const int itLon, const int itLat,
                           const int itStepLon, const int itStepLat,
                           const int n ) const;

    bool isOutOfTileRangeF( const qreal itLon, const qreal itLat,
                            const qreal itStepLon, const qreal itStepLat,
                            const int n ) const;

    /*!
        \brief Returns the color value of the result tile at the given integer position.
        \return The uint that describes the color value of the given pixel

        Note: for gray scale images the color value of a single pixel is described
        via a uchar (1 byte) while for RGB(A) images uint (4 bytes) are used.
    */
    inline
    uint
    pixel( int x, int y ) const
    {
        if(x < 0 || y < 0)
        {
            return 0;
        }

        if ( m_depth == 8 )
        {
            return m_image->color( (jumpTable8)[y][x] );
        }

        if ( m_depth == 32 )
        {
            return (jumpTable32)[y][x];
        }

        return m_image->pixel( x, y );
    }

    /*!
        \brief Returns the color value of the result tile at a given floating point position.
        \return The uint that describes the color value of the given pixel

        Subpixel calculation is done via bilinear interpolation.

        Note: for gray scale images the color value of a single pixel is described
        via a uchar (1 byte) while for RGB(A) images uint (4 bytes) are used.
    */
    uint pixelF( qreal x, qreal y ) const;
    // This method passes the top left pixel (if known already) for better performance
    uint pixelF( qreal x, qreal y, const QRgb& pixel ) const;

private:
    const QImage *m_image;
    Marble::GeoDataLatLonBox m_overlayLatLonBox;

    const qreal latToPixel;
    const qreal lonToPixel;

    // Previous coordinates
    qreal  m_prevLat;
    qreal  m_prevLon;
    qreal  m_prevPixelX;
    qreal  m_prevPixelY;

    const int m_depth;
    const uchar **const jumpTable8;
    const uint **const jumpTable32;
};


}


#endif // IMAGEMAPPERCONTEXT_H
