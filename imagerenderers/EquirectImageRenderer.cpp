#define QT_NO_DEBUG_OUTPUT
#include "EquirectImageRenderer.h"

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


EquirectImageRenderer::~EquirectImageRenderer()
{

}

class EquirectImageRenderer::RenderJob : public QRunnable
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

EquirectImageRenderer::RenderJob::RenderJob(const QImage *image, QImage *canvasImage, const ViewportParams *viewport, MapQuality mapQuality, int yTop, int yBottom , const GeoDataLatLonBox &overlayLatLonBox, const QRect &imageRect)
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
EquirectImageRenderer::renderImage(GeoPainter *painter)
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

void EquirectImageRenderer::RenderJob::run()
{
    // Scanline based algorithm to do texture mapping

    const int imageHeight = m_canvasImage->height();
    const int imageWidth  = m_canvasImage->width();
    const qint64  radius  = m_viewport->radius();
    // Calculate how many degrees are being represented per pixel.
    const qreal rad2Pixel = (qreal)( 2 * radius ) / M_PI;
    const float pixel2Rad =(float) 1.0/rad2Pixel;  // FIXME chainging to qreal may crash Marble when the equator is visible

    const bool interlaced   = ( m_mapQuality == LowQuality );
    const bool highQuality  = ( m_mapQuality == HighQuality
                             || m_mapQuality == PrintQuality );
    const bool printQuality = ( m_mapQuality == PrintQuality );

    // Evaluate the degree of interpolation
    const int n = ImageMapperContext::interpolationStep( m_viewport, m_mapQuality );

    // Calculate translation of center point
    const qreal centerLon = m_viewport->centerLongitude();
    const qreal centerLat = m_viewport->centerLatitude();

    const int yCenterOffset = (int)( centerLat * rad2Pixel );

    const int yTop = imageHeight / 2 - radius + yCenterOffset;

    qreal leftLon = + centerLon - ( imageWidth / 2 * pixel2Rad );
    while ( leftLon < -M_PI ) leftLon += 2 * M_PI;
    while ( leftLon >  M_PI ) leftLon -= 2 * M_PI;

    const int maxInterpolationPointX = n * (int)( imageWidth / n - 1 ) + 1;


    // initialize needed variables that are modified during texture mapping:

    ImageMapperContext context(m_image, m_overlayLatLonBox);


    int xLeft = qMax(0, m_imageRect.left());
    int xRight = qMin(imageWidth, m_imageRect.right());

    // Scanline based algorithm to do texture mapping

    for ( int y = m_yTop; y < m_yBottom; ++y ) {

        QRgb * scanLine = (QRgb*)( m_canvasImage->scanLine( y ) );

        qreal lon = leftLon;
        const qreal lat = M_PI/2 - (y - yTop )* pixel2Rad;

        bool hasValue = false;

        for ( int x = xLeft; x < xRight; ++x ) {

            // Prepare for interpolation
            bool interpolate = false;
            if ( hasValue &&
                 x > 0 &&
                 x <= maxInterpolationPointX )
            {
                x += n - 1;
                lon += (n - 1) * pixel2Rad;
                interpolate = !printQuality;
            }
            else
            {
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
                    hasValue = context.pixelValue( lon, lat, scanLine );
            }

            ++scanLine;
            lon += pixel2Rad;
        }

        // copy scanline to improve performance
        if ( interlaced &&
             y + 1 < m_yBottom   &&
             xRight > xLeft)
        {

            const int pixelByteSize = m_canvasImage->bytesPerLine() / imageWidth;

            memcpy( m_canvasImage->scanLine( y + 1 ),
                    m_canvasImage->scanLine( y     ),
                    imageWidth * pixelByteSize );
            ++y;
        }
    }
}
