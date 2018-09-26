#define QT_NO_DEBUG_OUTPUT
#include "GenericImageRenderer.h"

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


GenericImageRenderer::~GenericImageRenderer()
{

}

class GenericImageRenderer::RenderJob : public QRunnable
{
public:
    RenderJob( const QImage *image, QImage *canvasImage, const ViewportParams *viewport, MapQuality mapQuality, int yTop, int yBottom, const GeoDataLatLonBox &overlayLatLonBox, const QRect &imageRect );

    void run() override;

private:
    const QImage *m_image;
    QImage *const m_canvasImage;
    const ViewportParams *const m_viewport;
    const MapQuality m_mapQuality;
    const int m_yTop;
    const int m_yBottom;
    const GeoDataLatLonBox &m_overlayLatLonBox;
    const QRect &m_imageRect;
};

GenericImageRenderer::RenderJob::RenderJob(const QImage *image, QImage *canvasImage, const ViewportParams *viewport, MapQuality mapQuality, int yTop, int yBottom , const GeoDataLatLonBox &overlayLatLonBox, const QRect &imageRect)
    : m_image( image ),
      m_canvasImage( canvasImage ),
      m_viewport( viewport ),
      m_mapQuality( mapQuality ),
      m_yTop( yTop ),
      m_yBottom( yBottom ),
      m_overlayLatLonBox( overlayLatLonBox ),
      m_imageRect(imageRect)
{
}

void
GenericImageRenderer::renderImage(GeoPainter *painter)
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


    const int numThreads = m_threadPool.maxThreadCount();
    const int yStep = ( yBottom - yTop ) / numThreads;
    for ( int i = 0; i < numThreads; ++i )
    {
        const int yStart = yTop +  i      * yStep;
        const int yEnd   = i == numThreads -1 ? yBottom : yTop + (i + 1) * yStep;
        QRunnable *const job = new RenderJob( &m_image, &m_canvasImage, m_viewport, painter->mapQuality(), yStart, yEnd, m_overlayLatLonBox , imageRect);
        m_threadPool.start( job );
    }

    m_threadPool.waitForDone();

    imageRect = imageRect.intersected( m_dirtyRect );
    painter->drawImage( imageRect, m_canvasImage, imageRect );
}

void GenericImageRenderer::RenderJob::run()
{
    const int imageWidth  = m_canvasImage->width();
    const int imageHeight  = m_canvasImage->height();
    const qint64  radius  = m_viewport->radius();

    const bool interlaced   = ( m_mapQuality == LowQuality );
    const bool highQuality  = ( m_mapQuality == HighQuality
                             || m_mapQuality == PrintQuality );
    const bool printQuality = ( m_mapQuality == PrintQuality );

    // Evaluate the degree of interpolation
    const int n = ImageMapperContext::interpolationStep( m_viewport, m_mapQuality );

    // Calculate north pole position to decrease pole distortion later on
    qreal northPoleX, northPoleY;
    bool globeHidesNorthPole;
    GeoDataCoordinates northPole(0, m_viewport->currentProjection()->maxLat(), 0);
    m_viewport->screenCoordinates(northPole, northPoleX, northPoleY, globeHidesNorthPole );

    // initialize needed variables that are modified during texture mapping:

    ImageMapperContext context( m_image, m_overlayLatLonBox);

    qreal clipRadius = radius * m_viewport->currentProjection()->clippingRadius();


    // Paint the map.
    for ( int y = m_yTop; y < m_yBottom; ++y ) {

        // rx is the radius component in x direction
        const int rx = (int)sqrt( (qreal)( clipRadius * clipRadius
                                      - ( ( y - imageHeight / 2 )
                                          * ( y - imageHeight / 2 ) ) ) );

        // Calculate the actual x-range of the map within the current scanline.
        //
        // If the circular border of the earth disk is still visible then xLeft
        // equals the scanline position of the most left pixel that gets covered
        // by the earth disk. In terms of math this equals the half image width minus
        // the radius component on the current scanline in x direction ("rx").
        //
        // If the zoom factor is high enough then the whole screen gets covered
        // by the earth and the border of the earth disk isn't visible anymore.
        // In that situation xLeft equals zero.
        // For xRight the situation is similar.

        int xLeft  = ( imageWidth / 2 - rx > 0 ) ? imageWidth / 2 - rx
                                                       : 0;
        int xRight = ( imageWidth / 2 - rx > 0 ) ? xLeft + rx + rx
                                                       : imageWidth;

        xLeft = qMax(xLeft, m_imageRect.left());
        xRight = qMin(xRight, m_imageRect.right());

        QRgb * scanLine = (QRgb*)( m_canvasImage->scanLine( y ) ) + xLeft;

        const int xIpLeft  = ( imageWidth / 2 - rx > 0 ) ? n * (int)( xLeft / n + 1 )
                                                         : 1;
        const int xIpRight = ( imageWidth / 2 - rx > 0 ) ? n * (int)( xRight / n - 1 )
                                                         : n * (int)( xRight / n - 1 ) + 1;

        // Decrease pole distortion due to linear approximation ( y-axis )
        bool crossingPoleArea = false;
        if ( !globeHidesNorthPole
             && northPoleY - ( n * 0.75 ) <= y
             && northPoleY + ( n * 0.75 ) >= y )
        {
            crossingPoleArea = true;
        }

        int ncount = 0;

        bool hasValue = false;

        for ( int x = xLeft; x < xRight; ++x ) {

            // Prepare for interpolation
            const int leftInterval = xIpLeft + ncount * n;

            bool interpolate = false;

            if ( hasValue && x >= xIpLeft && x <= xIpRight )
            {
                // Decrease pole distortion due to linear approximation ( x-axis )
                if ( crossingPoleArea
                     && northPoleX >= leftInterval + n
                     && northPoleX < leftInterval + 2 * n
                     && x < leftInterval + 3 * n )
                {
                    interpolate = false;
                }
                else {
                    x += n - 1;
                    interpolate = !printQuality;
                    ++ncount;
                }
            }
            else
                interpolate = false;

            qreal lon;
            qreal lat;
            m_viewport->geoCoordinates(x,y, lon, lat, GeoDataCoordinates::Radian);

            if ( interpolate ) {
                if ( highQuality )
                    context.pixelValueApproxF( lon, lat, scanLine, n );
                else
                    context.pixelValueApprox( lon, lat, scanLine, n );

                scanLine += ( n - 1 );
            }

            if ( x < imageWidth ) {
                if ( highQuality )
                    hasValue = context.pixelValueF( lon, lat, scanLine );
                else
                    hasValue = context.pixelValue( lon, lat, scanLine );
            }

            ++scanLine;
        }

        // copy scanline to improve performance
        if ( interlaced &&
             y + 1 < m_yBottom  &&
             xRight > xLeft)
        {

            const int pixelByteSize = m_canvasImage->bytesPerLine() / imageWidth;

            memcpy( m_canvasImage->scanLine( y + 1 ) + xLeft * pixelByteSize,
                    m_canvasImage->scanLine( y     ) + xLeft * pixelByteSize,
                    ( xRight - xLeft ) * pixelByteSize );
            ++y;
        }
    }
}
