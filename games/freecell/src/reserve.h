#pragma once

#include <onut/Vector2.h>
#include "Pile.h"

class Reserve final : public Pile
{
public:
    Reserve(const Vector2& in_position);

    void draw() override;
    bool canAcceptCard(const CardRef& card) const override;
};
