#pragma once

#include "bitmap.h"

class Texture {
public:
    Texture(): _x(0), _y(0), _w(0), _h(0), _bitmap(nullptr)
    { }

    Texture(Bitmap * bitmap)
    {
        BindBitmap(bitmap);
    }

    Texture(Bitmap * bitmap, float x, float y)
    {
        BindBitmap(bitmap, x, y);
    }

    Texture(Bitmap * bitmap, float x, float y, float w, float h)
    {
        BindBitmap(bitmap, x, y, w, h);
    }

    ~Texture()
    { }

    float GetX() const { return _x; }
    float GetY() const { return _y; }
    float GetW() const { return _w; }
    float GetH() const { return _h; }
    Bitmap * GetBitmap() { return _bitmap; }

    void BindBitmap(Bitmap * bitmap)
    {
        BindBitmap(bitmap, 0, 0);
    }

    void BindBitmap(Bitmap * bitmap, float x, float y)
    {
        BindBitmap(bitmap, 0, 0, bitmap->GetW() - x, bitmap->GetH() - y);
    }

    void BindBitmap(Bitmap * bitmap, float x, float y, float w, float h)
    {
        _x = x; _y = y;
        _w = w; _h = h;
        _bitmap = bitmap;
    }

private:
    float _x, _y;
    float _w, _h;
    Bitmap * _bitmap;
};