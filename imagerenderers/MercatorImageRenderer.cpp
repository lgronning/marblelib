//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2007      Carlos Licea     <carlos _licea@hotmail.com>
// Copyright 2008      Inge Wallin      <inge@lysator.liu.se>
// Copyright 2011      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#include "MercatorImageRenderer.h"

// posix
#include <cmath>

// Qt
#include <QRunnable>
#include <QTime>

// Marble
#include "GeoPainter.h"
#include "MarbleDebug.h"
#include "ImageMapperContext.h"
#include "ViewportParams.h"
#include "MathHelper.h"
#include "projections/AbstractProjection.h"

using namespace Marble;


MercatorImageRenderer::~MercatorImageRenderer()
{

}

class MercatorImageRenderer::RenderJob : public QRunnable
{
public:
    RenderJob( const QImage *image, QImage *canvasImage, const ViewportParams *viewport, MapQuality mapQuality, int yTop, int yBottom, const GeoDataLatLonBox &overlayLatLonBox, const QRect &imageRect  );

    void run() override;

private:
    const QImage *m_image;
    QImage *const m_canvasImage;
    const ViewportParams *const m_viewport;
    const MapQuality m_mapQuality;
    const int m_yPaintedTop;
    const int m_yPaintedBottom;
    const GeoDataLatLonBox &m_overlayLatLonBox;
    const QRect &m_imageRect;
};

MercatorImageRenderer::RenderJob::RenderJob(const QImage *image, QImage *canvasImage, const ViewportParams *viewport, MapQuality mapQuality, int yTop, int yBottom , const GeoDataLatLonBox &overlayLatLonBox, const QRect &imageRect)
    : m_image( image ),
      m_canvasImage( canvasImage ),
      m_viewport( viewport ),
      m_mapQuality( mapQuality ),
      m_yPaintedTop( yTop ),
      m_yPaintedBottom( yBottom ),
      m_overlayLatLonBox( overlayLatLonBox ),
      m_imageRect(imageRect)
{
}

void
MercatorImageRenderer::renderImage(GeoPainter *painter)
{
    const GeoDataLatLonBox &tileLatLonBox = m_viewport->viewLatLonAltBox();
    if ( !tileLatLonBox.intersects( m_overlayLatLonBox.toCircumscribedRectangle() ) )
    {
        qDebug() << "Not intersecing" << tileLatLonBox.toString(GeoDataCoordinates::Degree)  << m_overlayLatLonBox.toString(GeoDataCoordinates::Degree);
        return;
    }

    QTime timer;
    timer.start();

    const QImage::Format optimalFormat = optimalCanvasImageFormat();

    if ( m_canvasImage.size() != m_viewport->size() || m_canvasImage.format() != optimalFormat ) {
        m_canvasImage = QImage( m_viewport->size(), optimalFormat );
    }

    m_canvasImage.fill(Qt::transparent);

    QRect imageRect = getImageRect();

    int yTop = qMax(imageRect.top(), 0);
    int yBottom = qMin(imageRect.bottom()+1, m_canvasImage.height());

    const int yStart = yTop;
    const int yEnd   = yBottom;
    RenderJob job( &m_image, &m_canvasImage, m_viewport, painter->mapQuality(), yStart, yEnd, m_overlayLatLonBox , imageRect);
    job.run();

    painter->drawImage( m_dirtyRect, m_canvasImage, m_dirtyRect );
}

void MercatorImageRenderer::RenderJob::run()
{
    // Scanline based algorithm to do texture mapping

    const int imageHeight = m_canvasImage->height();
    const int imageWidth  = m_canvasImage->width();
    const qint64  radius  = m_viewport->radius();
    // Calculate how many degrees are being represented per pixel.
    const float rad2Pixel = (float)( 2 * radius ) / M_PI;
    const qreal pixel2Rad = 1.0/rad2Pixel;

    const bool interlaced   = ( m_mapQuality == LowQuality );
    const bool highQuality  = ( m_mapQuality == HighQuality
                             || m_mapQuality == PrintQuality );
    const bool printQuality = ( m_mapQuality == PrintQuality );

    // Evaluate the degree of interpolation
    const int n = ImageMapperContext::interpolationStep( m_viewport, m_mapQuality );

    // Calculate translation of center point
    const qreal centerLon = m_viewport->centerLongitude();
    const qreal centerLat = m_viewport->centerLatitude();

    const int yCenterOffset = (int)( asinh( tan( centerLat ) ) * rad2Pixel  );

    qreal leftLon = + centerLon - ( imageWidth / 2 * pixel2Rad );
    const int maxInterpolationPointX = n * (int)( imageWidth / n - 1 ) + 1;


    // initialize needed variables that are modified during texture mapping:

    ImageMapperContext context(m_image, m_overlayLatLonBox);


    // Scanline based algorithm to do texture mapping
    int xLeft = qMax(0, m_imageRect.left());
    int xRight = qMin(imageWidth, m_imageRect.right());

    leftLon +=  (pixel2Rad*xLeft);

    while ( leftLon < -M_PI ) leftLon += 2 * M_PI;
    while ( leftLon >  M_PI ) leftLon -= 2 * M_PI;

    for ( int y = m_yPaintedTop; y < m_yPaintedBottom; ++y ) {

        QRgb * scanLine = (QRgb*)( m_canvasImage->scanLine( y ) );
        scanLine += xLeft;

        qreal lon = leftLon;
        const qreal lat = atan ( sinh ( ( (imageHeight / 2 + yCenterOffset) - y )
                    * pixel2Rad ));

        bool hasValue = false;

        for ( int x = xLeft; x < xRight; ++x ) {
            // Prepare for interpolation
            bool interpolate = false;
            if ( hasValue && x > 0 && x <= maxInterpolationPointX ) {
                x += n - 1;
                lon += (n - 1) * pixel2Rad;
                interpolate = !printQuality;
            }
            else {
                interpolate = false;
            }

            if ( lon < -M_PI ) lon += 2 * M_PI;
            if ( lon >  M_PI ) lon -= 2 * M_PI;

            if ( interpolate ) {
                if (highQuality)
                    context.pixelValueApproxF( lon, lat, scanLine, n );
                else
                    context.pixelValueApprox( lon, lat, scanLine, n );

                scanLine += ( n - 1 );
            }

            if ( x < imageWidth ) {
                if ( highQuality )
                    hasValue = context.pixelValueF( lon, lat, scanLine );
                else
                    hasValue = context.pixelValue( lon, lat, scanLine);
            }

            ++scanLine;
            lon += pixel2Rad;
        }

        // copy scanline to improve performance
        if ( interlaced &&
             y + 1 < m_yPaintedBottom   &&
             xRight > xLeft) {

            const int pixelByteSize = m_canvasImage->bytesPerLine() / imageWidth;

            memcpy( m_canvasImage->scanLine( y + 1 ),
                    m_canvasImage->scanLine( y     ),
                    imageWidth * pixelByteSize );
            ++y;
        }
    }
}
