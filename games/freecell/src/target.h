#pragma once

#include "pile.h"

class Target : public Pile
{
public:
    Target(const Vector2& in_position, int suit);

    void draw() override;

    void push(const CardRef& card) override;
    bool canAcceptCard(const CardRef& card) const override;

public:
    int suit;
    vector<CardRef> cards;
};
