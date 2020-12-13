#pragma once

#include <memory>
#include <vector>

#define SPR_CARD_BASE 0
#define SPR_TARGET_BASE 52
#define SPR_BACK 56
#define SPR_RESERVE 57

using namespace std;

class Sprite;
using SpriteRef = shared_ptr<Sprite>;

class Resources
{
public:
    Resources();

public:
    vector<SpriteRef> sprites;
};
