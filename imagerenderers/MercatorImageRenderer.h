#ifndef MERCATORIMAGERENDERER_H
#define MERCATORIMAGERENDERER_H

#include "ImageRenderInterface.h"

namespace Marble
{
class MercatorImageRenderer : public ImageRenderInterface
{
public:
    using ImageRenderInterface::ImageRenderInterface;

    
    ~MercatorImageRenderer() override;

    void
    renderImage(GeoPainter *painter) override;

private:
   class RenderJob;
};

}


#endif // MERCATORIMAGERENDERER_H
