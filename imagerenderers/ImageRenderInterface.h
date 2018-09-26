#ifndef IMAGERENDERINTERFACE_H
#define IMAGERENDERINTERFACE_H

#include <QtCore/QRect>
#include <QtGui/QImage>
#include <QtCore/QThreadPool>
#include "geodata/data/GeoDataLatLonBox.h"

namespace Marble
{

class GeoPainter;
class ViewportParams;


class ImageRenderInterface
{
public:
    ImageRenderInterface(const ViewportParams *viewport,
                         const QRect &dirtyRect,
                         const QImage &image,
                         const GeoDataLatLonBox &overlayLatLonBox);

    virtual
    ~ImageRenderInterface();

    virtual
    void
    renderImage(GeoPainter *painter) = 0;

protected:
    QImage::Format
    optimalCanvasImageFormat();

    QRect
    getImageRect();

    const ViewportParams *m_viewport;
    QRect m_dirtyRect;
    QImage m_image;
    QImage m_canvasImage;
    QThreadPool m_threadPool;
    GeoDataLatLonBox m_overlayLatLonBox;
};

}

#endif // IMAGERENDERINTERFACE_H
