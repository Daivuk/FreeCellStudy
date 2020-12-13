#pragma once

#include <onut/Color.h>
#include <onut/ForwardDeclaration.h>
#include <onut/Point.h>
#include <onut/Vector2.h>
#include <onut/Vector4.h>

OForwardDeclare(Texture);

class Sprite
{
public:
    Sprite();
    Sprite(const OTextureRef& sprite_sheet, const Point& pos, const Point& size);

    void draw(const Vector2& position, const Color& color = Color::White) const;
    void drawWithAngle(const Vector2& position, float angle, const Color& color = Color::White) const;

public:
    OTextureRef sprite_sheet;
    Vector2 size;
    Vector4 uvs;
};
