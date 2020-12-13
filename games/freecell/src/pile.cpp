#include "pile.h"
#include "card.h"

Pile::Pile(const Vector2& in_position)
    : position(in_position)
{
}
//    const int OFFSET_FROM = 14;
//    const int OFFSET_TO = 19;
//    const float MIN_OFFSET = 8.f;
//    const float MAX_OFFSET = 6.f;
//
//    float offset = MIN_OFFSET;
//    if ((int)pile.size() > OFFSET_FROM)
//    {
//        float offset_percent = onut::max(0.f, (float)((int)pile.size() - OFFSET_FROM) / (float)(OFFSET_TO - OFFSET_FROM));
//        offset = std::floorf(MIN_OFFSET + (MAX_OFFSET - MIN_OFFSET) * offset_percent);
//    }
//
//    for (auto card : pile)
//    {
//        Color color = card == mouse_hover_card ? HOVER_COLOR : Color::White;
//        SPRITES[card].draw(pos, color);
//        pos.y += offset;
//    }

void Pile::push(const CardRef& card)
{
    cards.push_back(card);
    card->animatePosition(position + Vector2(0, (float)(cards.size() - 1) * 8.0f), (int)cards.size());
}

void Pile::resetPositions()
{
    for (int i = 0, len = (int)cards.size(); i < len; ++i)
    {
        const auto& card = cards[i];
        card->animatePosition(position + Vector2(0, (float)i * 8.0f));
    }
}

bool Pile::empty() const
{
    return cards.empty();
}

bool Pile::contains(const CardRef& card) const
{
    for (const auto& pile_card : cards)
        if (pile_card == card)
            return true;

    return false;
}

static bool canStack(const CardRef& card, const CardRef& on)
{
    if (card->getValue() != on->getValue() - 1)
        return false;

    if (card->getColor() == on->getColor())
        return false;

    return true;
}

static bool isValidStack(const vector<CardRef>& stack)
{
    if (stack.empty())
        return false;

    auto prev_card = stack.front();
    for (int i = 1, len = (int)stack.size(); i < len; ++i)
    {
        const auto& card = stack[i];

        if (!canStack(card, prev_card))
            return false;

        prev_card = card;
    }

    return true;
}

vector<CardRef> Pile::grab(const CardRef& card)
{
    for (auto it = cards.begin(); it != cards.end(); ++it)
    {
        if (*it == card)
        {
            vector<CardRef> ret(it, cards.end());
            if (isValidStack(ret))
            {
                cards = { cards.begin(), it };
                return move(ret);
            }
        }
    }

    return {};
}

float Pile::cardOverlay(const CardRef& card) const
{
    Rect rect(position, 25, 39);
    if (!cards.empty())
        rect = cards.back()->getRect();

    auto card_rect = card->getRect();
    auto difference = rect.Difference(card_rect);
    return difference.Area();
}

bool Pile::canAcceptCard(const CardRef& card) const
{
    if (cards.empty())
        return true;

    return canStack(card, cards.back());
}
