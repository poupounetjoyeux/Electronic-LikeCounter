# include "Icon.h"

Icon::Icon(std::vector<uint16_t> iconBytes, int width, int height) {
    this->iconBytes = iconBytes;
    this->width = width;
    this->height = height;
}

void Icon::draw(PxMATRIX *display, int x, int y)
{
    (*display).drawRGBBitmap(x, y, &iconBytes[0], width, height);
}