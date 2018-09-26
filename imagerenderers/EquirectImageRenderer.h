#ifndef EQUIRECTIMAGERENDERER_H
#define EQUIRECTIMAGERENDERER_H

#include "ImageRenderInterface.h"

namespace Marble
{
class EquirectImageRenderer : public ImageRenderInterface
{
public:
    using ImageRenderInterface::ImageRenderInterface;

    
    ~EquirectImageRenderer() override;

    void
    renderImage(GeoPainter *painter) override;

private:
   class RenderJob;
};

}


#endif // EQUIRECTIMAGERENDERER_H
