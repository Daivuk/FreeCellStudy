#include <onut/Maths.h>
#include <onut/Rect.h>
#include "card.h"
#include "globals.h"
#include "resources.h"
#include "sprite.h"

Card::Card(int in_id)
    : id(in_id)
{
}

void Card::update(float dt)
{
    delay = onut::max(0.f, delay - dt);

    if (!delay)
    {
        position = Vector2::MoveToward(position, target_position, 5.f * dt, 3.5f);
        show_back = target_show_back;
    }

    if (position == target_position)
        draw_order = target_draw_order;
}

void Card::draw()
{
    if (show_back)
        g_resources->sprites[SPR_BACK]->draw(position);
    else
        g_resources->sprites[id]->draw(position);
}

void Card::setPosition(const Vector2& in_position)
{
    setPosition(in_position, draw_order, show_back);
}

void Card::setPosition(const Vector2& in_position, int in_draw_order)
{
    setPosition(in_position, in_draw_order, show_back);
}

void Card::setPosition(const Vector2& in_position, bool in_show_back)
{
    setPosition(in_position, draw_order, in_show_back);
}

void Card::setPosition(const Vector2& in_position, int in_draw_order, bool in_show_back)
{
    position = target_position = in_position;
    draw_order = target_draw_order = in_draw_order;
    show_back = target_show_back = in_show_back;
    delay = 0.f;
}

void Card::animatePosition(const Vector2& in_target_position, float in_delay)
{
    animatePosition(in_target_position, target_draw_order, target_show_back, in_delay);
}

void Card::animatePosition(const Vector2& in_target_position, int in_target_draw_order, float in_delay)
{
    animatePosition(in_target_position, in_target_draw_order, target_show_back, in_delay);
}

void Card::animatePosition(const Vector2& in_target_position, bool in_target_show_back, float in_delay)
{
    animatePosition(in_target_position, target_draw_order, in_target_show_back, in_delay);
}

void Card::animatePosition(const Vector2& in_target_position, int in_target_draw_order, bool in_target_show_back, float in_delay)
{
    target_position = in_target_position;
    target_draw_order = in_target_draw_order;
    target_show_back = in_target_show_back;
    delay = in_delay;
}

bool Card::contains(const Vector2& p) const
{
    return Rect(position, 25, 39).Contains(p);
}

Rect Card::getRect() const
{
    return Rect(position, 25, 39);
}

int Card::getColor() const
{
    return id / 26;
}

int Card::getValue() const
{
    return id % 13;
}

int Card::getSuit() const
{
    return id / 13;
}
