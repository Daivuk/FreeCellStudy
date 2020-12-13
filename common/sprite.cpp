#include <onut/SpriteBatch.h>
#include <onut/Texture.h>
#include "sprite.h"

Sprite::Sprite()
{
}

Sprite::Sprite(const OTextureRef& sprite_sheet, const Point& pos, const Point& size)
    : m_sprite_sheet(sprite_sheet)
    , m_size((float)size.x, (float)size.y)
{
    auto texture_size = sprite_sheet->getSizef();
    Vector2 inv_texture_size(1.f / texture_size.x, 1.f / texture_size.y);

    m_uvs.x = (float)pos.x * inv_texture_size.x;
    m_uvs.y = (float)pos.y * inv_texture_size.y;
    m_uvs.z = (float)(pos.x + size.x) * inv_texture_size.x;
    m_uvs.w = (float)(pos.y + size.y) * inv_texture_size.y;
}

void Sprite::draw(const Vector2& position, const Color& color) const
{
    oSpriteBatch->drawRectWithUVs(m_sprite_sheet, Rect(position, m_size), m_uvs, color);
}

void Sprite::drawWithAngle(const Vector2& position, float angle, const Color& color) const
{
    oSpriteBatch->drawSpriteWithUVs(m_sprite_sheet, position, m_uvs, color, angle);
}
