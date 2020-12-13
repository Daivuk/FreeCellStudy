#pragma once

#include <chrono>
#include <memory>
#include <vector>
#include <onut/Vector2.h>
#include <onut/Timing.h>

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
    Board(unsigned int seed);

    void update(float dt);
    void draw();
    void undo();

public:
    struct Move
    {
        PileRef from;
        PileRef to;
        vector<CardRef> cards;
    };

    bool grabCardStack(const CardRef& card);
    bool isDragStackSizeAllowed(int size, const PileRef& from, const PileRef& to) const;
    bool winCondition() const;

    unsigned int seed = 0;

    vector<CardRef> deck;
    vector<PileRef> piles;
    vector<ReserveRef> reserves;
    vector<TargetRef> targets;
    vector<Move> history;

    CardRef mouse_hover_card;

    OTime start_time;
    OSeconds final_time = 0s;

    struct Drag
    {
        Vector2 mouse_pos_on_down;
        Vector2 mouse_offset_on_card;
        PileRef origin;
        vector<CardRef> stack;
        float stack_card_offset = 0.f;
        bool started_drag = false;
    } drag;
};
