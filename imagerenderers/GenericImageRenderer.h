#ifndef GENERICIMAGERENDERER_H
#define GENERICIMAGERENDERER_H

#include "ImageRenderInterface.h"

namespace Marble
{
class GenericImageRenderer : public ImageRenderInterface
{
public:
    using ImageRenderInterface::ImageRenderInterface;

    
    ~GenericImageRenderer() override;

    void
    renderImage(GeoPainter *painter) override;

private:
   class RenderJob;
};

}


#endif // GENERICIMAGERENDERER_H
