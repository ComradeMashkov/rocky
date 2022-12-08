/**
 * rocky c++
 * Copyright 2023 Pelican Mapping
 * MIT License
 */
#include "ImageMosaic.h"
#include "Image.h"
#include "TileKey.h"

#define LC "[ImageMosaic] "

using namespace rocky;
using namespace rocky::util;


/***************************************************************************/

ImageMosaic::SourceImage::SourceImage(shared_ptr<Image> image_, const TileKey& key)
{
    image = image_;
    key.getExtent().getBounds(xmin, ymin, xmax, ymax);
    tilex = key.getTileX();
    tiley = key.getTileY();
}

void ImageMosaic::getExtents(double &minX, double &minY, double &maxX, double &maxY)
{
    minX = DBL_MAX;
    maxX = -DBL_MAX;
    minY = DBL_MAX;
    maxY = -DBL_MAX;

    for(auto& tile : _images)
    {
        minX = std::min(tile.xmin, minX);
        minY = std::min(tile.ymin, minY);
        maxX = std::max(tile.xmax, maxX);
        maxY = std::max(tile.ymax, maxY);
    }
}

shared_ptr<Image>
ImageMosaic::createImage() const
{
    if (_images.size() == 0)
    {
        ROCKY_DEBUG << "ImageMosaic has no images..." << std::endl;
        return 0;
    }

    // find the first valid tile and use its size as the mosaic tile size
    const SourceImage* tile = nullptr;
    for (int i = 0; i<_images.size() && !tile; ++i)
        if (_images[i].image && _images[i].image->valid())
            tile = &_images[i];

    if ( !tile )
        return nullptr;

    unsigned int tileWidth = tile->image->width();
    unsigned int tileHeight = tile->image->height();

    //ROCKY_NOTICE << "TileDim " << tileWidth << ", " << tileHeight << std::endl;

    unsigned int minTileX = tile->tilex;
    unsigned int minTileY = tile->tiley;
    unsigned int maxTileX = tile->tilex;
    unsigned int maxTileY = tile->tiley;

    //Compute the tile size.
    for(auto& c : _images)
    {
        if (c.tilex < minTileX) minTileX = c.tilex;
        if (c.tiley < minTileY) minTileY = c.tiley;

        if (c.tilex > maxTileX) maxTileX = c.tilex;
        if (c.tiley > maxTileY) maxTileY = c.tiley;
    }

    unsigned int tilesWide = maxTileX - minTileX + 1;
    unsigned int tilesHigh = maxTileY - minTileY + 1;

    unsigned int pixelsWide = tilesWide * tileWidth;
    unsigned int pixelsHigh = tilesHigh * tileHeight;
    unsigned int tileDepth = tile->image->depth();

    // make the new image and initialize it to transparent-white:
    auto result = Image::create(
        tile->image->pixelFormat(),
        pixelsWide,
        pixelsHigh,
        tileDepth);

    Image::Pixel clear(1, 1, 1, 0);
    auto i = result->get_iterator();
    i.forEachPixel([&]() { result->write(clear, i.s(), i.t()); });

    //osg::ref_ptr<osg::Image> image = new osg::Image;
    //image->allocateImage(pixelsWide, pixelsHigh, tileDepth, tile->_image->getPixelFormat(), tile->_image->getDataType());
    //image->setInternalTextureFormat(tile->_image->getInternalTextureFormat());

    //Initialize the image to be completely white!
    //ImageUtils::PixelWriter write(image.get());
    //write.assign(osg::Vec4(1,1,1,0));

    //Composite the incoming images into the master image
    for (auto& comp : _images)
    {
        //Determine the indices in the master image for this image
        if (comp.image)
        {
            int dstX = (comp.tilex - minTileX) * tileWidth;
            int dstY = (maxTileY - comp.tiley) * tileHeight;

            comp.image->copyAsSubImage(result.get(), dstX, dstY);
        }

    //    auto& sourceImage = comp.image;
    //    const osg::Image* sourceTile = i->getImage();
    //    if (sourceTile)
    //    {
    //        ImageUtils::copyAsSubImage(sourceTile, image.get(), dstX, dstY);
    //    }
    //}
    //for (TileImageList::iterator i = _images.begin(); i != _images.end(); ++i)
    //{
    //    //Determine the indices in the master image for this image
    //    int dstX = (i->_tileX - minTileX) * tileWidth;
    //    int dstY = (maxTileY - i->_tileY) * tileHeight;

    //    const osg::Image* sourceTile = i->getImage();
    //    if ( sourceTile )
    //    {
    //        ImageUtils::copyAsSubImage(sourceTile, image.get(), dstX, dstY);
    //    }
    }

    return result;
}

/***************************************************************************/