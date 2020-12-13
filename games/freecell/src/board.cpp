#include <onut/Input.h>
#include <onut/Random.h>
#include "board.h"
#include "card.h"
#include "common.h"
#include "pile.h"
#include "reserve.h"
#include "target.h"

Board::Board()
{
    // Create playfield piles
    for (int i = 0; i < 8; ++i)
        piles.push_back(make_shared<Pile>(Vector2(10 + (float)i * 30, 44)));

    // Create reserves
    for (int i = 0; i < 4; ++i)
    {
        auto reserve = make_shared<Reserve>(Vector2(2 + (float)i * 28, 2));
        reserves.push_back(reserve);
        piles.push_back(reserve);
    }

    // Create targets
    for (int i = 0; i < 4; ++i)
    {
        auto target = make_shared<Target>(Vector2(142 + (float)i * 28, 2), i);
        targets.push_back(target);
        piles.push_back(target);
    }

    // Create deck
    deck.reserve(52);
    for (int i = 0; i < 52; ++i)
        deck.push_back(make_shared<Card>(i));

    // Shuffle
    onut::randomizeSeed();
    onut::shuffle(deck);

    // Distribute cards on board
    for (int i = 0; i < 52; ++i)
    {
        piles[i % 8]->push(deck[i]);
        deck[i]->target_show_back = false; // Reveal
        deck[i]->draw_order = deck[i]->target_draw_order; // Force draw order on first draw
        deck[i]->delay = (float)i * 0.05f;
    }
}

bool Board::grabCardStack(const CardRef& card)
{
    drag = {};

    auto mouse_pos = oInput->mousePosf / (float)PIXEL_SIZE;
    drag.mouse_offset_on_card = mouse_pos - card->position;
    //drag.mouse_offset_on_card = { 12.f, 12.f };

    for (const auto& pile : piles)
    {
        if (pile->allow_pick && pile->contains(card))
        {
            drag.stack = pile->grab(card);

            if (!drag.stack.empty())
            {
                drag.origin = pile;
                drag.stack_card_offset = 0.f;

                // Set offset between cards
                if (drag.stack.size() > 1)
                    drag.stack_card_offset = drag.stack[1]->position.y - drag.stack[0]->position.y;

                // Draw order on top of everything
                for (int i = 0, len = (int)drag.stack.size(); i < len; ++i)
                    drag.stack[i]->draw_order = 1000 + i;

                return true;
            }
        }
    }

    return false;
}

void Board::update(float dt)
{
    // Update cards animation
    for (const auto& card : deck)
        card->update(dt);

    // Pick the card the mouse is hover
    auto mouse_pos = oInput->mousePosf / (float)PIXEL_SIZE;
    auto cards = deck;
    sort(cards.begin(), cards.end(), [](const CardRef& a, const CardRef& b) { return a->draw_order < b->draw_order; });

    for (const auto& card : cards)
        if (card->contains(mouse_pos))
            mouse_hover_card = card;

    // Drag cards
    if (OInputJustPressed(OMouse1) && mouse_hover_card)
        grabCardStack(mouse_hover_card);

    // Update dragging card stack
    if (!drag.stack.empty())
    {
        // Update position to follow mouse
        for (int i = 0, len = (int)drag.stack.size(); i < len; ++i)
            drag.stack[i]->setPosition(mouse_pos - drag.mouse_offset_on_card + Vector2(0, (float)i * drag.stack_card_offset));

        // Drop
        if (OInputReleased(OMouse1))
        {
            // Check if we can move to new pile
            PileRef target_pile = drag.origin;
            float biggest_overlay_size = 0.f;
            for (const auto& pile : piles)
            {
                auto overlay_size = pile->cardOverlay(drag.stack.front());
                if (overlay_size > biggest_overlay_size)
                {
                    biggest_overlay_size = overlay_size;
                    target_pile = pile;
                }
            }

            if (!target_pile->canAcceptCard(drag.stack.front()) || (drag.stack.size() > 1 && !target_pile->accept_stack))
                target_pile = drag.origin;

            drag.origin->resetPositions();

            // Put on target pile
            for (const auto& card : drag.stack)
                target_pile->push(card);

            drag.stack.clear();
            drag.origin.reset();
        }
    }
}

void Board::draw()
{
    // Reserve slots
    for (const auto& reserve : reserves)
        reserve->draw();

    // Target slots
    for (const auto& target : targets)
        target->draw();

    // Reorder cards
    auto cards = deck;
    sort(cards.begin(), cards.end(), [](const CardRef& a, const CardRef& b) { return a->draw_order < b->draw_order; });

    // Draw cards
    for (const auto& card : cards)
        card->draw();
}
