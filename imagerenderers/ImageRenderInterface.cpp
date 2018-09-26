#define QT_NO_DEBUG_OUTPUT
#include "ImageRenderInterface.h"
#include "ViewportParams.h"
#include "geodata/data/GeoDataLineString.h"
#include <QTime>

using namespace Marble;

ImageRenderInterface::ImageRenderInterface(const ViewportParams *viewport,
                                           const QRect &dirtyRect,
                                           const QImage &image,
                                           const Marble::GeoDataLatLonBox &overlayLatLonBox)
    :   m_viewport(viewport),
        m_dirtyRect(dirtyRect),
        m_image(image),
        m_overlayLatLonBox(overlayLatLonBox)
{

}

ImageRenderInterface::~ImageRenderInterface()
{

}

QImage::Format ImageRenderInterface::optimalCanvasImageFormat()
{
    return QImage::Format_ARGB32_Premultiplied;
}

QRect
ImageRenderInterface::getImageRect()
{
    QTime timer;
    timer.start();

    if(m_viewport->viewLatLonAltBox().united(m_overlayLatLonBox) == m_viewport->viewLatLonAltBox() ||
       m_overlayLatLonBox.united(m_viewport->viewLatLonAltBox()) == m_overlayLatLonBox ||
       m_overlayLatLonBox.width() > M_PI_4 ||
       m_overlayLatLonBox.height() > M_PI_4)
    {
        return QRect(QPoint(0,0), m_viewport->size());
    }

    GeoDataLineString geoDataString(QVector<GeoDataCoordinates>()
                                    << GeoDataCoordinates(m_overlayLatLonBox.west(), m_overlayLatLonBox.north())
                                     << GeoDataCoordinates(m_overlayLatLonBox.east(), m_overlayLatLonBox.north())
                                     << GeoDataCoordinates(m_overlayLatLonBox.east(), m_overlayLatLonBox.south())
                                     << GeoDataCoordinates(m_overlayLatLonBox.west(), m_overlayLatLonBox.south())
                                    << GeoDataCoordinates(m_overlayLatLonBox.west(), m_overlayLatLonBox.north()), TessellationFlag::Tessellate | TessellationFlag::RespectLatitudeCircle);

    bool globeHidesPoint = false;
    for(int i = 0; i < geoDataString.size() && !globeHidesPoint; ++i)
    {
        const GeoDataCoordinates & coordinate = geoDataString.at(i);
        double x, y;

        globeHidesPoint = m_viewport->screenCoordinates(coordinate, x, y, globeHidesPoint);
    }

    QRect imageRect;
    if(!globeHidesPoint)
    {
        QVector<QPolygonF> polygons;
        bool ok = m_viewport->screenCoordinates(geoDataString, polygons);
        qDebug() << "ok" << ok;

        QRectF rect;
        foreach(auto polygon, polygons)
        {
            if(rect.isNull())
            {
                rect = polygon.boundingRect();
            }
            else
            {
                rect |= polygon.boundingRect();
            }
        }

        polygons.clear();

        imageRect = rect.toRect();
    }
    else
    {
        imageRect = QRect(QPoint(0,0), m_viewport->size());
    }

    qDebug() << "ImageRenderInterface::getImageRect" << timer.elapsed();
    return imageRect;
}
