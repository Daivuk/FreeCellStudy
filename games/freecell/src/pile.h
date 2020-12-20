#pragma once

#include <memory>
#include <vector>
#include <onut/Vector2.h>

using namespace std;

class Card;
using CardRef = shared_ptr<Card>;

class Pile
{
public:
    Pile(const Vector2& in_position);
    virtual ~Pile() {};

    virtual void draw() {};
    virtual void push(const CardRef& card);
    virtual bool empty() const;
    virtual float cardOverlay(const CardRef& card) const;
    virtual bool canAcceptCard(const CardRef& card) const;
    virtual void resetPositions();

    bool contains(const CardRef& card) const;
    vector<CardRef> grab(const CardRef& card);

    bool isDecreasingOrder() const;

public:
    Vector2 position;
    vector<CardRef> cards;
    bool allow_pick = true;
    bool accept_stack = true;
};
