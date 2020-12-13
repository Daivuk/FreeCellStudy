#pragma once

#include <onut/Rect.h>
#include <onut/Vector2.h>

class Card
{
public:
    Card(int in_id);

    void update(float dt);
    void draw();

    void setPosition(const Vector2& position);
    void setPosition(const Vector2& position, int draw_order);
    void setPosition(const Vector2& position, bool show_back);
    void setPosition(const Vector2& position, int draw_order, bool show_back);

    void animatePosition(const Vector2& target_position, float delay = 0.f);
    void animatePosition(const Vector2& target_position, int target_draw_order, float delay = 0.f);
    void animatePosition(const Vector2& target_position, bool target_show_back, float delay = 0.f);
    void animatePosition(const Vector2& target_position, int target_draw_order, bool target_show_back, float delay = 0.f);

    bool contains(const Vector2& p) const;
    Rect getRect() const;

    int getColor() const;
    int getValue() const;
    int getSuit() const;

public:
    int id;
    float delay = 0.f;

    Vector2 position;
    bool show_back = true;
    int draw_order = 0;

    Vector2 target_position;
    bool target_show_back = true;
    int target_draw_order = 0;
};
