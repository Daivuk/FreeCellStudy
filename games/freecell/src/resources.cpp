#include <onut/Texture.h>
#include "resources.h"
#include "sprite.h"

Resources::Resources()
{
    skins.push_back(OGetTexture("skin1.png"));
    skins.push_back(OGetTexture("skin2.png"));

    skin = skins[0];
    
    // Build sprites
    for (int i = 0; i < 52; ++i)
        sprites.push_back(make_shared<Sprite>(skin, Point(1 + (i % 13) * 26, 1 + (i / 13) * 40), Point(25, 39)));

    for (int i = 0; i < 4; ++i)
        sprites.push_back(make_shared<Sprite>(skin, Point(339, 1 + i * 40), Point(25, 39)));
    
    sprites.push_back(make_shared<Sprite>(skin, Point(365, 1), Point(25, 39)));
    sprites.push_back(make_shared<Sprite>(skin, Point(365, 41), Point(25, 39)));
}

void Resources::changeSkin(const OTextureRef& in_skin)
{
    skin = in_skin;

    for (const auto& sprite : sprites)
        sprite->sprite_sheet = skin;
}
