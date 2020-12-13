#include "reserve.h"
#include "card.h"
#include "globals.h"
#include "resources.h"
#include "sprite.h"

Reserve::Reserve(const Vector2& in_position)
    : Pile(in_position)
{
    accept_stack = false;
}

void Reserve::draw()
{
    g_resources->sprites[SPR_RESERVE]->draw(position);
}

bool Reserve::canAcceptCard(const CardRef& card) const
{
    return cards.empty();
}
