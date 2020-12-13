#include <onut/SpriteBatch.h>
#include <onut/Texture.h>
#include "sprite.h"

Sprite::Sprite()
{
}

Sprite::Sprite(const OTextureRef& in_sprite_sheet, const Point& pos, const Point& size)
    : sprite_sheet(in_sprite_sheet)
    , size((float)size.x, (float)size.y)
{
    auto texture_size = sprite_sheet->getSizef();
    Vector2 inv_texture_size(1.f / texture_size.x, 1.f / texture_size.y);

    uvs.x = (float)pos.x * inv_texture_size.x;
    uvs.y = (float)pos.y * inv_texture_size.y;
    uvs.z = (float)(pos.x + size.x) * inv_texture_size.x;
    uvs.w = (float)(pos.y + size.y) * inv_texture_size.y;
}

void Sprite::draw(const Vector2& position, const Color& color) const
{
    oSpriteBatch->drawRectWithUVs(sprite_sheet, Rect(position, size), uvs, color);
}

void Sprite::drawWithAngle(const Vector2& position, float angle, const Color& color) const
{
    oSpriteBatch->drawSpriteWithUVs(sprite_sheet, position, uvs, color, angle);
}
