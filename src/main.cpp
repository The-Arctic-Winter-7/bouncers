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
        // If we've gone off the screen on the top
        if (y < MIN_Y)
        {
            // Snap back to screen and reverse direction
            y = MIN_Y;
            y_speed *= -1;
        }
        // If we've gone off the screen on the bottom
        if (y > MAX_Y)
        {
            // Snap back to screen and reverse direction
            y = MAX_Y;
            y_speed *= -1;
        }

        sprite.set_x(x);
        sprite.set_y(y);
    }
};

// Both functions that calculate averages must be set before the BouncerAlt class
bn::fixed average_x(bn::vector<Bouncer, MAX_BOUNCERS> &bouncers)
{
    // Add all x positions together
    bn::fixed x_sum = 0;
    // Iterates through all current bouncers
    for (Bouncer bouncer : bouncers)
    {
        // and adds each x position into a single variable
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
    // Add all y positions together
    bn::fixed y_sum = 0;
    // Iterates through all current bouncers
    for (Bouncer bouncer : bouncers)
    {
        // and adds each y position into a single variable
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

// Alternative bouncer that remains at the average position of all other bouncers
class BouncerAlt
{
private:
public:
    bn::sprite_ptr sprite;

    BouncerAlt(bn::fixed x_start, bn::fixed y_start) : sprite(bn::sprite_items::bouncer.create_sprite(x_start, y_start))
    {
    }

    void update(bn::vector<Bouncer, MAX_BOUNCERS> &bouncers)
    {
        // No speed needed
        // Only updates to the average position of all bouncers
        sprite.set_x(average_x(bouncers));
        sprite.set_y(average_y(bouncers));
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

void add_alt_bouncer(bn::vector<BouncerAlt, MAX_ALT_BOUNCERS> &alt_bouncers,
                     bn::fixed start_x, bn::fixed start_y)
{
    // Only one alternative bouncer is created, so there is no need to check if there are more than zero
    alt_bouncers.push_back(BouncerAlt(start_x, start_y));
}

void check_collision(bn::vector<Bouncer, MAX_BOUNCERS> &bouncers)
{
    // This is the radius of a bouncer sprite
    bn::fixed r = 8;

    // i represents x and j represents y; radius is the same for colliding pairs
    // Use distance formula: (x2 - x1)^2 + (y2 - y1)^2 <= (2r)^2
    for (int i = 0; i < bouncers.size(); i++)
    {
        // Nested for loop checks every possible pair every frame
        for (int j = i + 1; j < bouncers.size(); j++)
        {
            // These variables represent the x and y coordiates of two bouncers
            bn::fixed x1 = bouncers[i].sprite.x();
            bn::fixed x2 = bouncers[j].sprite.x();
            bn::fixed y1 = bouncers[i].sprite.y();
            bn::fixed y2 = bouncers[j].sprite.y();

            // Butano has no math function for powers so a workaroud is needed
            // These are expanded polynomials for each x and y binomial pairing
            bn::fixed x_dist = ((x1 * x1) - (2 * x1 * x2) + (x2 * x2));
            bn::fixed y_dist = ((y1 * y1) - (2 * y1 * y2) + (y2 * y2));
            // This is meant to be the two radii of each object, but all bouncers are the same size
            bn::fixed r_dist = (2 * r) * (2 * r);

            // Uses distance formula to check if a pair of bouncers intersect with each other
            if (x_dist + y_dist <= r_dist)
            {
                // Bouncers exchange x and y speeds
                bn::fixed tempx = bouncers[i].x_speed; // temp storage for swap
                bn::fixed tempy = bouncers[i].y_speed; // temp storage for swap
                bouncers[i].x_speed = bouncers[j].x_speed;
                bouncers[i].y_speed = bouncers[j].y_speed;
                bouncers[j].x_speed = tempx;
                bouncers[j].y_speed = tempy;
            }
        }
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
            add_alt_bouncer(alt_bouncers, 0, 0);
        }

        // if A is pressed add a new bouncer
        if (bn::keypad::a_pressed())
        {
            add_bouncer(bouncers, 0, 0);
        }

        check_collision(bouncers);

        for (Bouncer &b : bouncers)
        {
            b.update();
        }

        for (BouncerAlt &alt : alt_bouncers)
        {
            alt.update(bouncers);
        }

        bn::core::update();
    }
}