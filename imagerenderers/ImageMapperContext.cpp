//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2007      Carlos Licea     <carlos _licea@hotmail.com>
// Copyright 2011      Bernhard Beschow <bbeschow@cs.tu-berlin.de
//

#include "ImageMapperContext.h"

#include <QImage>

#include "MarbleDebug.h"
#include "StackedTile.h"
#include "StackedTileLoader.h"
#include "TileId.h"
#include "ViewParams.h"
#include "ViewportParams.h"

namespace
{
static const uint **jumpTableFromQImage32( const QImage *img )
{
    if ( img->depth() != 48 && img->depth() != 32 )
        return nullptr;

    const int  height = img->height();
    const int  bpl    = img->bytesPerLine() / 4;
    const uint  *data = reinterpret_cast<const QRgb*>(img->bits());
    const uint **jumpTable = new const uint*[height];

    for ( int y = 0; y < height; ++y ) {
        jumpTable[ y ] = data;
        data += bpl;
    }

    return jumpTable;
}


static const uchar **jumpTableFromQImage8( const QImage *img )
{
    if ( img->depth() != 8 && img->depth() != 1 )
        return nullptr;

    const int  height = img->height();
    const int  bpl    = img->bytesPerLine();
    const uchar  *data = img->bits();
    const uchar **jumpTable = new const uchar*[height];

    for ( int y = 0; y < height; ++y ) {
        jumpTable[ y ] = data;
        data += bpl;
    }

    return jumpTable;
}

}
using namespace Marble;



ImageMapperContext::ImageMapperContext(const QImage *image, const GeoDataLatLonBox &overlayLatLonBox)
    : m_image(image ),
      m_overlayLatLonBox(overlayLatLonBox),
      latToPixel(image->height() / overlayLatLonBox.height()),
      lonToPixel(image->width() / overlayLatLonBox.width()),
      m_prevLat( 0.0 ),
      m_prevLon( 0.0 ),
      m_prevPixelX( 0.0 ),
      m_prevPixelY( 0.0 ),
      m_depth( image->depth() ),
      jumpTable8( m_depth == 8 ? jumpTableFromQImage8( image ) : 0),
      jumpTable32( m_depth == 32 ? jumpTableFromQImage32( image ) : 0)
{
}

ImageMapperContext::~ImageMapperContext()
{
    delete [] jumpTable8;
    delete [] jumpTable32;
}

bool
ImageMapperContext::pixelValueF( const qreal lon, const qreal lat, QRgb* const scanLine )
{
    // The same method using integers performs about 33% faster.
    // However we need the qreal version to create the high quality mode.

    // Convert the lon and lat coordinates of the position on the scanline
    // measured in radian to the pixel position of the requested
    // coordinate on the current image.

    if(!m_overlayLatLonBox.contains(lon, lat))
    {
//        qWarning() << "ImageMapperContext::pixelValue lon lat not inside" << m_overlayLatLonBox.toString(GeoDataCoordinates::Unit::Degree) << lon * RAD2DEG << lat * RAD2DEG;

        *scanLine = 0;
        return false;
    }

    qreal posX = GeoDataLatLonBox::width( lon, m_overlayLatLonBox.west() ) * lonToPixel;
    qreal posY = (qreal)( m_image->height() ) - ( GeoDataLatLonBox::height( lat, m_overlayLatLonBox.south() ) * latToPixel ) - 1;

    *scanLine = pixelF(posX, posY);

    m_prevPixelX = posX;
    m_prevPixelY = posY;
    m_prevLon = lon;
    m_prevLat = lat; // preparing for interpolation
    return true;
}

bool
ImageMapperContext::pixelValue( const qreal lon, const qreal lat, QRgb* const scanLine )
{
    // The same method using integers performs about 33% faster.
    // However we need the qreal version to create the high quality mode.

    // Convert the lon and lat coordinates of the position on the scanline
    // measured in radian to the pixel position of the requested
    // coordinate on the current tile.

    if(!m_overlayLatLonBox.contains(lon, lat))
    {
//        qWarning() << "ImageMapperContext::pixelValue lon lat not inside" << m_overlayLatLonBox.toString(GeoDataCoordinates::Unit::Degree) << lon * RAD2DEG << lat * RAD2DEG;
        *scanLine = 0;
        return false;
    }

    int posX = static_cast<int>(GeoDataLatLonBox::width( lon, m_overlayLatLonBox.west() ) * lonToPixel);
    int posY = static_cast<int>((qreal)( m_image->height() ) - ( GeoDataLatLonBox::height( lat, m_overlayLatLonBox.south() ) * latToPixel ) - 1);

    *scanLine = pixel( posX, posY);

    m_prevPixelX = posX;
    m_prevPixelY = posY;
    m_prevLon = lon;
    m_prevLat = lat; // preparing for interpolation

    return true;
}

// This method interpolates color values for skipped pixels in a scanline.
//
// While moving along the scanline we don't move from pixel to pixel but
// leave out n pixels each time and calculate the exact position and
// color value for the new pixel. The pixel values in between get
// approximated through linear interpolation across the direct connecting
// line on the original tiles directly.
// This method will do by far most of the calculations for the
// texturemapping, so we move towards integer math to improve speed.

void
ImageMapperContext::pixelValueApproxF( const qreal lon, const qreal lat, QRgb *scanLine, const int n )
{
    // stepLon/Lat: Distance between two subsequent approximated positions

    qreal stepLat = lat - m_prevLat;
    qreal stepLon = lon - m_prevLon;

    // As long as the distance is smaller than 180 deg we can assume that
    // we didn't cross the dateline.

    const qreal nInverse = 1.0 / (qreal)(n);

    if ( fabs(stepLon) < M_PI )
    {
        qreal posX = GeoDataLatLonBox::width( lon, m_overlayLatLonBox.west() ) * lonToPixel;
        qreal posY = (qreal)( m_image->height() ) - ( GeoDataLatLonBox::height( lat, m_overlayLatLonBox.south() ) * latToPixel ) - 1;

        const qreal itStepPosX = (posX - m_prevPixelX ) * nInverse;
        const qreal itStepPosY = (posY - m_prevPixelY ) * nInverse;

        // To improve speed we unroll
        // AbstractScanlineTextureMapper::pixelValue(...) here and
        // calculate the performance critical issues via integers

        qreal itPosX = m_prevPixelX;
        qreal itPosY = m_prevPixelY;

        const int tileWidth = m_image->width();
        const int tileHeight = m_image->height();

        // int oldR = 0;
        // int oldG = 0;
        // int oldB = 0;

        QRgb oldRgb = qRgba( 0, 0, 0, 0);

        qreal oldPosX = -1;
        qreal oldPosY = 0;

        const bool alwaysCheckTileRange = isOutOfTileRangeF( itPosX, itPosY, itStepPosX, itStepPosY, n );

        for ( int j=1; j < n; ++j, ++scanLine )
        {
            qreal posX = itPosX + itStepPosX * j;
            qreal posY = itPosY + itStepPosY * j;
            if ( alwaysCheckTileRange &&
                 (posX >= tileWidth ||
                  posX < 0.0 ||
                  posY >= tileHeight ||
                  posY < 0.0 ) )
            {
                *scanLine = 0;
                oldPosX = -1;
            }
            else
            {
                *scanLine = pixel( posX, posY);

                // Just perform bilinear interpolation if there's a color change compared to the
                // last pixel that was evaluated. This speeds up things greatly for maps like OSM
                if ( *scanLine != oldRgb )
                {
                    if ( oldPosX != -1 )
                    {
                        *(scanLine - 1) = pixelF( oldPosX, oldPosY, *(scanLine - 1) );
                        oldPosX = -1;
                    }

                    oldRgb = pixelF(posX, posY, *scanLine );
                    *scanLine = oldRgb;
                }
                else
                {
                    oldPosX = posX;
                    oldPosY = posY;
                }
            }




            // if ( needsFilter( *scanLine, oldR, oldB, oldG  ) ) {
            //     *scanLine = m_tile->pixelF( posX, posY );
            // }
        }
    }

    // For the case where we cross the dateline between (lon, lat) and
    // (prevlon, prevlat) we need a more sophisticated calculation.
    // However as this will happen rather rarely, we use
    // pixelValue(...) directly to make the code more readable.

    else
    {
        stepLon = ( TWOPI - fabs(stepLon) ) * nInverse;
        stepLat = stepLat * nInverse;
        // We need to distinguish two cases:
        // crossing the dateline from east to west ...

        if ( m_prevLon < lon ) {

            for ( int j = 1; j < n; ++j ) {
                m_prevLat += stepLat;
                m_prevLon -= stepLon;
                if ( m_prevLon <= -M_PI )
                    m_prevLon += TWOPI;
                pixelValueF( m_prevLon, m_prevLat, scanLine );
                ++scanLine;
            }
        }

        // ... and vice versa: from west to east.

        else
        {
            qreal curStepLon = lon - n * stepLon;

            for ( int j = 1; j < n; ++j ) {
                m_prevLat += stepLat;
                curStepLon += stepLon;
                qreal  evalLon = curStepLon;
                if ( curStepLon <= -M_PI )
                    evalLon += TWOPI;
                pixelValueF( evalLon, m_prevLat, scanLine );
                ++scanLine;
            }
        }
    }
}


bool
ImageMapperContext::isOutOfTileRangeF( const qreal itPosX, const qreal itPosY,
                                       const qreal itStepPosX, const qreal itStepPosY,
                                       const int n ) const
{
    const qreal minIPosX = itPosX + itStepPosX;
    const qreal minIPosY = itPosY + itStepPosY;
    const qreal maxIPosX = itPosX + itStepPosX * ( n - 1 );
    const qreal maxIPosY = itPosY + itStepPosY * ( n - 1 );
    return ( maxIPosX >= m_image->width()  || maxIPosX < 0 ||
             maxIPosY >= m_image->height() || maxIPosY < 0 ||
             minIPosX >= m_image->width()  || minIPosX < 0 ||
             minIPosY >= m_image->height() || minIPosY < 0 );
}


void ImageMapperContext::pixelValueApprox( const qreal lon, const qreal lat, QRgb *scanLine, const int n )
{
    // stepLon/Lat: Distance between two subsequent approximated positions

    qreal stepLat = lat - m_prevLat;
    qreal stepLon = lon - m_prevLon;

    // As long as the distance is smaller than 180 deg we can assume that
    // we didn't cross the dateline.

    const qreal nInverse = 1.0 / (qreal)(n);

    if ( fabs(stepLon) < M_PI )
    {
        int iPosX = static_cast<int>(GeoDataLatLonBox::width( lon, m_overlayLatLonBox.west() ) * lonToPixel);
        int iPosY = static_cast<int>(static_cast<qreal>( m_image->height() ) - ( GeoDataLatLonBox::height( lat, m_overlayLatLonBox.south() ) * latToPixel ) - 1);

        const int itStepPosX = static_cast<int>( ( iPosX - m_prevPixelX ) * nInverse * 128.0 );
        const int itStepPosY = static_cast<int>( ( iPosY - m_prevPixelY ) * nInverse * 128.0);

        // To improve speed we unroll
        // AbstractScanlineTextureMapper::pixelValue(...) here and
        // calculate the performance critical issues via integers

        int itPosX = static_cast<int>(m_prevPixelX * 128.0);
        int itPosY = static_cast<int>(m_prevPixelY * 128.0);

        const int imageWidth = m_image->width();
        const int imageHeight = m_image->height();

        const bool alwaysCheckTileRange = isOutOfTileRange( itPosX, itPosY, itStepPosX, itStepPosY, n );

        if ( !alwaysCheckTileRange )
        {
            int iPosXf = itPosX;
            int iPosYf = itPosY;
            for ( int j = 1; j < n; ++j )
            {
                iPosXf += itStepPosX;
                iPosYf += itStepPosY;
                *scanLine = pixel(iPosXf >> 7, iPosYf >> 7);
                ++scanLine;
            }
        }
        else
        {
            for ( int j = 1; j < n; ++j )
            {
                int iPosX = ( itPosX + itStepPosX * j ) >> 7;
                int iPosY = ( itPosY + itStepPosY * j ) >> 7;

                if ( iPosX >= imageWidth
                    || iPosX < 0
                    || iPosY >= imageHeight
                    || iPosY < 0 )
                {
                    *scanLine = 0;
                }
                else
                {
                    *scanLine = pixel(iPosX, iPosY );
                }

                ++scanLine;
            }
        }
    }

    // For the case where we cross the dateline between (lon, lat) and
    // (prevlon, prevlat) we need a more sophisticated calculation.
    // However as this will happen rather rarely, we use
    // pixelValue(...) directly to make the code more readable.

    else
    {
        stepLon = ( TWOPI - fabs(stepLon) ) * nInverse;
        stepLat = stepLat * nInverse;
        // We need to distinguish two cases:
        // crossing the dateline from east to west ...

        if ( m_prevLon < lon ) {

            for ( int j = 1; j < n; ++j ) {
                m_prevLat += stepLat;
                m_prevLon -= stepLon;
                if ( m_prevLon <= -M_PI )
                    m_prevLon += TWOPI;
                pixelValue( m_prevLon, m_prevLat, scanLine );
                ++scanLine;
            }
        }

        // ... and vice versa: from west to east.

        else {
            qreal curStepLon = lon - n * stepLon;

            for ( int j = 1; j < n; ++j ) {
                m_prevLat += stepLat;
                curStepLon += stepLon;
                qreal  evalLon = curStepLon;
                if ( curStepLon <= -M_PI )
                    evalLon += TWOPI;
                pixelValue( evalLon, m_prevLat, scanLine );
                ++scanLine;
            }
        }
    }
}


bool ImageMapperContext::isOutOfTileRange( const int itLon, const int itLat,
                                                     const int itStepLon, const int itStepLat,
                                                     const int n ) const
{
    const int minIPosX = (itLon + itStepLon) >> 7;
    const int minIPosY = (itLat + itStepLat) >> 7;
    const int maxIPosX = (itLon + itStepLon * ( n - 1 )) >> 7;
    const int maxIPosY = (itLat + itStepLat * ( n - 1 )) >> 7;
    return (    maxIPosX >= m_image->width()  || maxIPosX < 0
             || maxIPosY >= m_image->height() || maxIPosY < 0
             || minIPosX >= m_image->width()  || minIPosX < 0
             || minIPosY >= m_image->height() || minIPosY < 0 );
}


int ImageMapperContext::interpolationStep( const ViewportParams *viewport, MapQuality mapQuality )
{
    if ( mapQuality == PrintQuality ) {
        return 1;    // Don't interpolate for print quality.
    }

    if ( ! viewport->mapCoversViewport() ) {
        return 8;
    }

    // Find the optimal interpolation interval m_nBest for the
    // current image canvas width
    const int width = viewport->width();

    int nBest = 2;
    int nEvalMin = width - 1;
    for ( int it = 1; it < 48; ++it ) {
        int nEval = ( width - 1 ) / it + ( width - 1 ) % it;
        if ( nEval < nEvalMin ) {
            nEvalMin = nEval;
            nBest = it;
        }
    }

    return nBest;
}


uint ImageMapperContext::pixelF( qreal x, qreal y ) const
{
    int iX = (int)(x);
    int iY = (int)(y);

    QRgb topLeftValue  =  pixel( iX, iY );

    return pixelF( x, y, topLeftValue );
}


uint ImageMapperContext::pixelF( qreal x, qreal y, const QRgb& topLeftValue ) const
{
    // Bilinear interpolation to determine the color of a subpixel

    int iX = (int)(x);
    int iY = (int)(y);

    qreal fY = y - iY;

    // Interpolation in y-direction
    if ( ( iY + 1 ) < m_image->height() )
    {

        QRgb bottomLeftValue  =  pixel( iX, iY + 1 );
#define CHEAPHIGH
#ifdef CHEAPHIGH
        QRgb leftValue;
        if ( fY < 0.33 )
            leftValue = topLeftValue;
        else if ( fY < 0.66 )
            leftValue = (((bottomLeftValue ^ topLeftValue) & 0xfefefefeUL) >> 1)
                            + (bottomLeftValue & topLeftValue);
        else
            leftValue = bottomLeftValue;
#else
        // blending the color values of the top left and bottom left point
        qreal ml_red   = ( 1.0 - fY ) * qRed  ( topLeftValue  ) + fY * qRed  ( bottomLeftValue  );
        qreal ml_green = ( 1.0 - fY ) * qGreen( topLeftValue  ) + fY * qGreen( bottomLeftValue  );
        qreal ml_blue  = ( 1.0 - fY ) * qBlue ( topLeftValue  ) + fY * qBlue ( bottomLeftValue  );
#endif
        // Interpolation in x-direction
        if ( iX + 1 < m_image->width() ) {

            qreal fX = x - iX;

            QRgb topRightValue    =  pixel( iX + 1, iY );
            QRgb bottomRightValue =  pixel( iX + 1, iY + 1 );

#ifdef CHEAPHIGH
            QRgb rightValue;
            if ( fY < 0.33 )
                rightValue = topRightValue;
            else if ( fY < 0.66 )
                rightValue = (((bottomRightValue ^ topRightValue) & 0xfefefefeUL) >> 1)
                                + (bottomRightValue & topRightValue);
            else
                rightValue = bottomRightValue;

            QRgb averageValue;
            if ( fX < 0.33 )
                averageValue = leftValue;
            else if ( fX < 0.66 )
                averageValue = (((leftValue ^ rightValue) & 0xfefefefeUL) >> 1)
                                + (leftValue & rightValue);
            else
                averageValue = rightValue;

            return averageValue;
#else
            // blending the color values of the top right and bottom right point
            qreal mr_red   = ( 1.0 - fY ) * qRed  ( topRightValue ) + fY * qRed  ( bottomRightValue );
            qreal mr_green = ( 1.0 - fY ) * qGreen( topRightValue ) + fY * qGreen( bottomRightValue );
            qreal mr_blue  = ( 1.0 - fY ) * qBlue ( topRightValue ) + fY * qBlue ( bottomRightValue );

            // blending the color values of the resulting middle left
            // and middle right points
            int mm_red   = (int)( ( 1.0 - fX ) * ml_red   + fX * mr_red   );
            int mm_green = (int)( ( 1.0 - fX ) * ml_green + fX * mr_green );
            int mm_blue  = (int)( ( 1.0 - fX ) * ml_blue  + fX * mr_blue  );

            return qRgb( mm_red, mm_green, mm_blue );
#endif
        }
        else {
#ifdef CHEAPHIGH
            return leftValue;
#else
            return qRgb( ml_red, ml_green, ml_blue );
#endif
        }
    }
    else {
        // Interpolation in x-direction
        if ( iX + 1 < m_image->width() ) {

            qreal fX = x - iX;

            if ( fX == 0.0 )
                return topLeftValue;

            QRgb topRightValue    =  pixel( iX + 1, iY );
#ifdef CHEAPHIGH
            QRgb topValue;
            if ( fX < 0.33 )
                topValue = topLeftValue;
            else if ( fX < 0.66 )
                topValue = (((topLeftValue ^ topRightValue) & 0xfefefefeUL) >> 1)
                                + (topLeftValue & topRightValue);
            else
                topValue = topRightValue;

            return topValue;
#else
            // blending the color values of the top left and top right point
            int tm_red   = (int)( ( 1.0 - fX ) * qRed  ( topLeftValue ) + fX * qRed  ( topRightValue ) );
            int tm_green = (int)( ( 1.0 - fX ) * qGreen( topLeftValue ) + fX * qGreen( topRightValue ) );
            int tm_blue  = (int)( ( 1.0 - fX ) * qBlue ( topLeftValue ) + fX * qBlue ( topRightValue ) );

            return qRgb( tm_red, tm_green, tm_blue );
#endif
        }
    }

    return topLeftValue;
}
