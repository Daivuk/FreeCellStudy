#include <onut/Texture.h>
#include "resources.h"
#include "sprite.h"

Resources::Resources()
{
    auto sprite_sheet = OGetTexture("sprite_sheet.png");
    
    // Build sprites
    for (int i = 0; i < 52; ++i)
        sprites.push_back(make_shared<Sprite>(sprite_sheet, Point(1 + (i % 13) * 26, 1 + (i / 13) * 40), Point(25, 39)));

    for (int i = 0; i < 4; ++i)
        sprites.push_back(make_shared<Sprite>(sprite_sheet, Point(339, 1 + i * 40), Point(25, 39)));
    
    sprites.push_back(make_shared<Sprite>(sprite_sheet, Point(365, 1), Point(25, 39)));
    sprites.push_back(make_shared<Sprite>(sprite_sheet, Point(365, 41), Point(25, 39)));
}
