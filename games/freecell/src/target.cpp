#include "target.h"
#include "card.h"
#include "globals.h"
#include "resources.h"
#include "sprite.h"

Target::Target(const Vector2& in_position, int in_suit)
    : Pile(in_position)
    , suit(in_suit)
{
    allow_pick = false;
    accept_stack = false;
}

void Target::draw()
{
    g_resources->sprites[SPR_TARGET_BASE + suit]->draw(position);
}

void Target::push(const CardRef& card)
{
    cards.push_back(card);
    card->animatePosition(position, (int)cards.size());
}

bool Target::canAcceptCard(const CardRef& card) const
{
    if (suit != card->getSuit())
        return false;

    if (cards.empty())
    {
        return card->getValue() == 0;
    }
    else
    {
        return card->getValue() == cards.back()->getValue() + 1;
    }
}
