#pragma once

#include "bitmap.h"

class Texture {
public:
    Texture(): _x(0), _y(0), _w(0), _h(0), _bitmap(nullptr)
    { }

    Texture(Bitmap * bitmap)
    {
        SetTexture(bitmap);
    }

    Texture(Bitmap * bitmap, float x, float y)
    {
        SetTexture(bitmap, x, y);
    }

    Texture(Bitmap * bitmap, float x, float y, float w, float h)
    {
        SetTexture(bitmap, x, y, w, h);
    }

    ~Texture()
    { }

    float GetX() const { return _x; }
    float GetY() const { return _y; }
    float GetW() const { return _w; }
    float GetH() const { return _h; }
    const Bitmap * GetBitmap() const { return _bitmap; }

    void SetTexture(Bitmap * bitmap)
    {
        SetTexture(bitmap, 0, 0);
    }

    void SetTexture(Bitmap * bitmap, float x, float y)
    {
        SetTexture(bitmap, 0, 0, bitmap->GetW() - x, bitmap->GetH() - y);
    }

    void SetTexture(Bitmap * bitmap, float x, float y, float w, float h)
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