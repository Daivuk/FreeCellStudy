#include <onut/Font.h>
#include <onut/Input.h>
#include <onut/Random.h>
#include <onut/SpriteBatch.h>
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
        deck[i]->delay = (float)i * 0.0125f;
    }

    start_time = ONow;
}

bool Board::grabCardStack(const CardRef& card)
{
    drag = {};

    auto mouse_pos = oInput->mousePosf / (float)PIXEL_SIZE;
    drag.mouse_offset_on_card = mouse_pos - card->position;
    drag.mouse_pos_on_down = mouse_pos;

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

bool Board::isDragStackSizeAllowed(int size, const PileRef& from, const PileRef& to) const
{
    if (from == to)
        return true;

    int allowed_count = 1;

    for (const auto& reserve : reserves)
        if (reserve->empty())
            allowed_count++;

    for (int i = 0; i < 8; ++i)
        if (piles[i]->empty() && piles[i] != from && piles[i] != to)
            allowed_count *= 2;

    return size <= allowed_count;
}

bool Board::winCondition() const
{
    for (const auto& target : targets)
        if (target->cards.size() != 13)
            return false;

    return true;
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

    mouse_hover_card = nullptr;
    for (const auto& card : cards)
        if (card->contains(mouse_pos))
            mouse_hover_card = card;

    // Drag cards
    if (OInputJustPressed(OMouse1) && mouse_hover_card)
        grabCardStack(mouse_hover_card);

    // Update dragging card stack
    if (!drag.stack.empty())
    {
        // Right click cancel the move
        if (OInputJustPressed(OMouse2))
        {
            // Put back on origin pile
            for (const auto& card : drag.stack)
                drag.origin->push(card);

            drag.stack.clear();
            drag.origin.reset();
        }
        else
        {
            if (Vector2::Distance(mouse_pos, drag.mouse_pos_on_down) > 3.f / (float)PIXEL_SIZE)
                drag.started_drag = true;

            // Update position to follow mouse
            for (int i = 0, len = (int)drag.stack.size(); i < len; ++i)
                drag.stack[i]->setPosition(mouse_pos - drag.mouse_offset_on_card + Vector2(0, (float)i * drag.stack_card_offset));

            // Drop
            if (OInputReleased(OMouse1))
            {
                // Check if we can move to new pile
                PileRef target_pile;

                // Was this a single click? auto-pick target pile
                if (!drag.started_drag)
                {
                    const auto& bottom_card = drag.stack.front();

                    // Targets priority
                    for (const auto& target : targets)
                    {
                        if (target->canAcceptCard(bottom_card) && drag.stack.size() == 1)
                        {
                            target_pile = target;
                            break;
                        }
                    }

                    // If card is king, priority move to empty cell
                    if (!target_pile && bottom_card->getValue() == 12)
                    {
                        for (int i = 0; i < 8; ++i)
                        {
                            const auto& pile = piles[i];
                            if (pile->empty())
                            {
                                target_pile = pile;
                                break;
                            }
                        }
                    }

                    // Then, find another pile where we can move the stack to
                    if (!target_pile)
                    {
                        for (int i = 0; i < 8; ++i)
                        {
                            const auto& pile = piles[i];
                            if (pile->canAcceptCard(bottom_card) && !pile->empty())
                            {
                                target_pile = pile;
                                break;
                            }
                        }
                    }

                    // Finally, if single card, move to reserve
                    if (!target_pile && drag.stack.size() == 1)
                    {
                        for (const auto& reserve : reserves)
                        {
                            if (reserve->canAcceptCard(bottom_card))
                            {
                                target_pile = reserve;
                                break;
                            }
                        }
                    }
                }
                else
                {
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

                    if (target_pile && 
                        (!target_pile->canAcceptCard(drag.stack.front()) || (drag.stack.size() > 1 && !target_pile->accept_stack)))
                        target_pile = drag.origin;
                }

                if (!target_pile || !isDragStackSizeAllowed((int)drag.stack.size(), drag.origin, target_pile))
                    target_pile = drag.origin;

                drag.origin->resetPositions();

                // Put on target pile
                for (const auto& card : drag.stack)
                    target_pile->push(card);

                if (target_pile != drag.origin)
                {
                    Move move;
                    move.cards = drag.stack;
                    move.from = drag.origin;
                    move.to = target_pile;
                    history.push_back(move);
                }

                drag.stack.clear();
                drag.origin.reset();
            }
        }
    }
    else if (OInputPressed(OKeyLeftControl) && OInputJustPressed(OKeyZ) && !history.empty()) // Undo
    {
        auto move = history.back();
        history.pop_back();

        move.to->grab(move.cards.front());

        // Draw order on top of everything
        for (int i = 0, len = (int)move.cards.size(); i < len; ++i)
            move.cards[i]->draw_order = 1000 + i;
        
        for (const auto& card : move.cards)
            move.from->push(card);
    }

    if (!winCondition())
        final_time = OToSeconds(ONow - start_time);
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

    // Moves/time/score
    auto font = OGetFont("font.fnt");
    oSpriteBatch->drawText(font, "Time", {126, 8}, OTop, TEXT_COLOR);
    oSpriteBatch->drawText(font, onut::secondsToString(final_time, true), {126, 8 + 6}, OTop, ACTIVE_TEXT_COLOR);
    oSpriteBatch->drawText(font, "Moves", {126, 24}, OTop, TEXT_COLOR);
    oSpriteBatch->drawText(font, to_string(history.size()), {126, 24 + 6}, OTop, ACTIVE_TEXT_COLOR);
}
