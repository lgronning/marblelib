#ifndef SPHERICALIMAGERENDERER_H
#define SPHERICALIMAGERENDERER_H

#include "ImageRenderInterface.h"

namespace Marble
{
class SphericalImageRenderer : public ImageRenderInterface
{
public:
    using ImageRenderInterface::ImageRenderInterface;

    
    ~SphericalImageRenderer() override;

    void
    renderImage(GeoPainter *painter) override;

private:
   class RenderJob;
};

}

#endif // SPHERICALIMAGERENDERER_H
