#ifndef HEADER_ICON
#define HEADER_ICON

#include <PxMatrix.h>
#include <string>

class Icon
{
public:
    Icon(std::vector<uint16_t> iconBytes, int width, int height);
    void draw(PxMATRIX *display, int x, int y);
    int width;
    int height;
private:
  std::vector<uint16_t> iconBytes;
};

#endif