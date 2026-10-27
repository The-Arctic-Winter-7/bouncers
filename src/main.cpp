#include <bn_core.h>
#include <bn_backdrop.h>
#include <bn_keypad.h>
#include <bn_sprite_ptr.h>
#include <bn_display.h>
#include <bn_random.h>
#include <bn_vector.h>
#include <bn_log.h>

#include "bn_sprite_items_bouncer.h"

// Set max/min x position to be the edges of the display
static constexpr int HALF_SCREEN_WIDTH = bn::display::width() / 2;
static constexpr bn::fixed MIN_X = -HALF_SCREEN_WIDTH;
static constexpr bn::fixed MAX_X = HALF_SCREEN_WIDTH;

static constexpr int HALF_SCREEN_HEIGHT = bn::display::height() / 2;
static constexpr bn::fixed MIN_Y = -HALF_SCREEN_HEIGHT;
static constexpr bn::fixed MAX_Y = HALF_SCREEN_HEIGHT;

// Starting speed of a bouncer
static constexpr bn::fixed BASE_SPEED = 2;

// Maximum number of bouncers on screen at once
static constexpr int MAX_BOUNCERS = 20;
static constexpr int MAX_ALT_BOUNCERS = 1;

bn::random rng = bn::random();

bn::fixed rng_speed()
{
    bn::fixed speed = rng.get_unbiased_fixed(-BASE_SPEED, BASE_SPEED);

    while (speed >= -1 && speed <= 1)
        ;

    return speed;
}

class Bouncer
{
private:
public:
    bn::sprite_ptr sprite;
    bn::fixed x_speed;
    bn::fixed y_speed;

    Bouncer(bn::fixed x_start, bn::fixed y_start) : sprite(bn::sprite_items::bouncer.create_sprite(x_start, y_start)),
                                                    x_speed(rng_speed()),
                                                    y_speed(rng_speed())
    {
    }

    void update()
    {

        bn::fixed x = sprite.x();
        bn::fixed y = sprite.y();

        // Update position by adding speed
        x += x_speed;
        y += y_speed;

        // If we've gone off the screen on the right
        if (x > MAX_X)
        {
            // Snap back to screen and reverse direction
            x = MAX_X;
            x_speed *= -1;
        }
        // If we've gone off the screen on the left
        if (x < MIN_X)
        {
            // Snap back to screen and reverse direction
            x = MIN_X;
            x_speed *= -1;
        }
        if (y < MIN_Y)
        {
            y = MIN_Y;
            y_speed *= -1;
        }
        if (y > MAX_Y)
        {
            y = MAX_Y;
            y_speed *= -1;
        }

        sprite.set_x(x);
        sprite.set_y(y);
    }
};

bn::fixed average_x(bn::vector<Bouncer, MAX_BOUNCERS> &bouncers)
{
    // Add all x positions together
    bn::fixed x_sum = 0;
    for (Bouncer bouncer : bouncers)
    {
        x_sum += bouncer.sprite.x();
    }

    bn::fixed x_average = x_sum;

    // Only divide if we have 1 or more
    // Prevents division by 0
    if (bouncers.size() > 0)
    {
        x_average /= bouncers.size();
    }

    return x_average;
}

bn::fixed average_y(bn::vector<Bouncer, MAX_BOUNCERS> &bouncers)
{
    // Add all x positions together
    bn::fixed y_sum = 0;
    for (Bouncer bouncer : bouncers)
    {
        y_sum += bouncer.sprite.y();
    }

    bn::fixed y_average = y_sum;

    // Only divide if we have 1 or more
    // Prevents division by 0
    if (bouncers.size() > 0)
    {
        y_average /= bouncers.size();
    }

    return y_average;
}

class BouncerAlt
{
private:
public:
    bn::sprite_ptr sprite;
    bn::fixed x_pos;
    bn::fixed y_pos;

    BouncerAlt(bn::vector<Bouncer, MAX_BOUNCERS> &bouncers, bn::fixed x_start, bn::fixed y_start) : sprite(bn::sprite_items::bouncer.create_sprite(x_start, y_start)),
                                                                                                   x_pos(average_x(bouncers)),
                                                                                                   y_pos(average_y(bouncers))
    {
    }

    void update()
    {
        sprite.set_x(x_pos);
        sprite.set_y(y_pos);
    }
};

void add_bouncer(bn::vector<Bouncer, MAX_BOUNCERS> &bouncers, bn::fixed start_x, bn::fixed start_y)
{
    // Only add if we're below the maximum
    if (bouncers.size() < bouncers.max_size())
    {
        bouncers.push_back(Bouncer(start_x, start_y));
    }
}

void add_alt_bouncer(bn::vector<BouncerAlt, MAX_ALT_BOUNCERS> &alt_bouncers, bn::vector<Bouncer, MAX_BOUNCERS> bouncers,
                     bn::fixed start_x, bn::fixed start_y)
{
    if (alt_bouncers.size() < alt_bouncers.max_size())
    {
        alt_bouncers.push_back(BouncerAlt(bouncers, start_x, start_y));
    }
}

int main()
{
    bn::core::init();

    bn::vector<Bouncer, MAX_BOUNCERS> bouncers = {};
    bn::vector<BouncerAlt, MAX_ALT_BOUNCERS> alt_bouncers = {};

    while (true)
    {
        // if b is pressed, print average x positions of bouncers
        if (bn::keypad::b_pressed() && bouncers.size() > 0)
        {
            BN_LOG(average_x(bouncers));
            add_alt_bouncer(alt_bouncers, bouncers, 0, 0);
        }

        // if A is pressed add a new bouncer
        if (bn::keypad::a_pressed())
        {
            add_bouncer(bouncers, 0, 0);
        }

        // if B is pressed print the average to the console
        /* if (bn::keypad::b_pressed())
         {
             bn::fixed x_average = average_x(sprites);
             BN_LOG("Average x: ", x_average);
       */

        for (Bouncer &b : bouncers)
        {
            b.update();
        }

        for (BouncerAlt &alt : alt_bouncers)
        {
            alt.update();
        }

        bn::core::update();
    }
}