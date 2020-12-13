#pragma once

#include <memory>
#include <vector>
#include <onut/Vector2.h>

using namespace std;

class Card;
using CardRef = shared_ptr<Card>;

class Pile;
using PileRef = shared_ptr<Pile>;

class Reserve;
using ReserveRef = shared_ptr<Reserve>;

class Target;
using TargetRef = shared_ptr<Target>;

class Board
{
public:
    Board();

    void update(float dt);
    void draw();

public:
    bool grabCardStack(const CardRef& card);

    vector<CardRef> deck;
    vector<PileRef> piles;
    vector<ReserveRef> reserves;
    vector<TargetRef> targets;

    CardRef mouse_hover_card;

    struct Drag
    {
        Vector2 mouse_offset_on_card;
        PileRef origin;
        vector<CardRef> stack;
        float stack_card_offset = 0.f;
    } drag;
};
