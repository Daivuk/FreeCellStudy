#include <onut/Font.h>
#include <onut/Input.h>
#include <onut/Random.h>
#include <onut/Settings.h>
#include <onut/SpriteBatch.h>
#include "board.h"
#include "card.h"
#include "common.h"
#include "pile.h"
#include "reserve.h"
#include "target.h"

Board::Board()
    : Board(onut::randomizeSeed())
{
}

Board::Board(unsigned int in_seed)
    : seed(in_seed)
{
    // Create playfield piles
    for (int i = 0; i < 8; ++i)
        piles.push_back(make_shared<Pile>(Vector2(10 + (float)i * 30, 44 + TOP_OFFSET)));

    // Create reserves
    for (int i = 0; i < 4; ++i)
    {
        auto reserve = make_shared<Reserve>(Vector2(2 + (float)i * 28, 2 + TOP_OFFSET));
        reserves.push_back(reserve);
        piles.push_back(reserve);
    }

    // Create targets
    for (int i = 0; i < 4; ++i)
    {
        auto target = make_shared<Target>(Vector2(142 + (float)i * 28, 2 + TOP_OFFSET), i);
        targets.push_back(target);
        piles.push_back(target);
    }

    // Create deck
    deck.reserve(52);
    for (int i = 0; i < 52; ++i)
        deck.push_back(make_shared<Card>(i));

    // Shuffle
    onut::setSeed(seed);
    onut::shuffle(deck);

    // Distribute cards on board
    for (int i = 0; i < 52; ++i)
    {
        piles[i % 8]->push(deck[i]);
        deck[i]->target_show_back = false; // Reveal
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

bool Board::canSolve() const
{
    if (!drag.stack.empty())
        return false;

    for (int i = 0; i < 8; ++i)
        if (!piles[i]->isDecreasingOrder())
            return false;

    return true;
}

TargetRef Board::getTargetLowestValue(int& lowest_value) const
{
    lowest_value = 14;
    TargetRef lowest;

    for (const auto& target : targets)
    {
        int target_value = 0;
        if (!target->empty())
        {
            target_value = target->cards.back()->getValue();
        }

        if (target_value < lowest_value)
        {
            lowest = target;
            lowest_value = target_value;
        }
    }

    return lowest_value == 12 ? nullptr : lowest;
}

void Board::autoSolve()
{
    for (int i = 0; i < 12; ++i)
        for (const auto& card : piles[i]->cards)
            card->draw_order += 1000;

    float delay = 0.f;
    int lowest_value;
    while (auto target = getTargetLowestValue(lowest_value))
    {
        auto suit = target->suit;
        auto value = lowest_value + 1;

        // Find the card that matches
        for (int i = 0; i < 12; ++i)
        {
            const auto& pile = piles[i];
            if (!pile->empty())
            {
                const auto& card = pile->cards.back();
                auto card_value = card->getValue();
                auto card_suit = card->getSuit();
                if (card_value == value && card_suit == suit)
                {
                    pile->cards.pop_back();
                    card->position = card->target_position;
                    target->push(card);
                    card->delay = delay;
                    delay += 0.1f;

                    Move move;
                    move.cards = { card };
                    move.from = pile;
                    move.to = target;
                    history.push_back(move);
                    break;
                }
            }
        }
    }
}

void Board::update(float dt)
{
    // Update cards animation
    for (const auto& card : deck)
        card->update(dt);

    if (!won)
    {
        final_time = OToSeconds(ONow - start_time);

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
                if (Vector2::Distance(mouse_pos, drag.mouse_pos_on_down) > 8.f / (float)PIXEL_SIZE)
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

                                if (pile == drag.origin)
                                    continue;

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

        if (canSolve())
            autoSolve();

        if (winCondition())
        {
            won = true;

            best_turn_count = stoi(oSettings->getUserSetting("best_turn_count"));
            best_time = stoi(oSettings->getUserSetting("best_time"));

            now_turn_count = (int)history.size();
            now_time = (int)final_time.count();

            if (now_turn_count < best_turn_count)
            {
                oSettings->setUserSetting("best_turn_count", to_string(now_turn_count));
            }

            if (now_time < best_time)
            {
                oSettings->setUserSetting("best_time", to_string(now_time));
            }
        }
    }
}

void Board::undo()
{
    if (!history.empty()) // Undo
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
    oSpriteBatch->drawText(font, "Time", {126, 8 + TOP_OFFSET}, OTop, TEXT_COLOR);
    oSpriteBatch->drawText(font, onut::secondsToString(final_time, true), {126, 8 + 6 + TOP_OFFSET}, OTop, ACTIVE_TEXT_COLOR);
    oSpriteBatch->drawText(font, "Moves", {126, 24 + TOP_OFFSET}, OTop, TEXT_COLOR);
    oSpriteBatch->drawText(font, to_string(history.size()), {126, 24 + 6 + TOP_OFFSET}, OTop, ACTIVE_TEXT_COLOR);

    // Win scores
    if (won)
    {
        oSpriteBatch->drawText(font, "Best Time: " + onut::secondsToString(OSeconds(best_time), true), {126, 110}, OTop, 
                               best_time > now_time ? ACTIVE_TEXT_COLOR : TEXT_COLOR);
        oSpriteBatch->drawText(font, "Now: " + onut::secondsToString(OSeconds(now_time), true), {126, 116}, OTop, 
                               best_time < now_time ? ACTIVE_TEXT_COLOR : TEXT_COLOR);

        oSpriteBatch->drawText(font, "Best Moves: " + to_string(best_turn_count), {126, 130}, OTop, 
                               best_turn_count < now_turn_count ? ACTIVE_TEXT_COLOR : TEXT_COLOR);
        oSpriteBatch->drawText(font, "Now: " + to_string(now_turn_count), {126, 136}, OTop, 
                               best_turn_count > now_turn_count ? ACTIVE_TEXT_COLOR : TEXT_COLOR);
    }
}
