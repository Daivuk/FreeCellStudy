#pragma once

#include <memory>
#include <vector>

#include <onut/ForwardDeclaration.h>

#define SPR_CARD_BASE 0
#define SPR_TARGET_BASE 52
#define SPR_BACK 56
#define SPR_RESERVE 57

using namespace std;

OForwardDeclare(Texture);

class Sprite;
using SpriteRef = shared_ptr<Sprite>;

class Resources
{
public:
    Resources();

    void changeSkin(const OTextureRef& skin);

public:
    vector<OTextureRef> skins;
    OTextureRef skin;
    vector<SpriteRef> sprites;
};
