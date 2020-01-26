#ifndef SNAKEGAME_H_
#define SNAKEGAME_H_

#include "lib/avr/hardware/timers.hpp"
#include "../../../dragberry/os/display.hpp"
#include "../../../util/arraylist.hpp"

class SnakeGame : public Timers::T1::Handler
{
private:
    static const uint8_t FIELD_COLOR        = 0b00000100;
    static const uint8_t SNAKE_COLOR        = WHITE;
    static const uint8_t SNAKE_HEAD_COLOR   = YELLOW;
    static const uint8_t TRASH_COLOR        = 0b00010101;
    static const uint8_t WALL_COLOR         = BLACK;

    static const uint8_t FOOD_INCREMENT_COLOR   = RED;
    static const uint8_t FOOD_DECREMENT_COLOR   = BLUE;
    static const uint8_t FOOD_SPEED_UP_COLOR    = MAGENTA;
    static const uint8_t FOOD_SPEED_DOWN_COLOR  = 0b00010011;

    static const uint8_t MASK_TYPE = 0b00000011;

    enum Type : uint8_t
    {
        FIELD   = 0b00000000,
        SNAKE   = 0b00000001,
        FOOD    = 0b00000010,
        WALL    = 0b00000011,
    };

    static const uint8_t MASK_SNAKE_PART = 0b00001100;

    enum SnakePart : uint8_t
    {
        HEAD = 0b00000000,
        BODY = 0b00000100,
        TAIL = 0b00001000,
    };

    static const uint8_t MASK_SNAKE_DIRECTION = 0b00110000;

    enum SnakeDirection : uint8_t
    {
        UP      = 0b00000000,
        RIGHT   = 0b00010000,
        DOWN    = 0b00100000,
        LEFT    = 0b00110000,
    };

    static const uint8_t MASK_FIELD_TYPE = 0b00001100;

    enum FieldType : uint8_t
    {
        GRASS  = 0b00000000,
        LAKE   = 0b00000100,
        TRASH  = 0b00001000,
    };

    static const uint8_t MASK_FIELD_LIFETIME = 0b11110000;

    static const uint8_t MASK_FOOD_TYPE = 0b00001100;

    enum FoodType : uint8_t
    {
        INCREMENT   = 0b00000000,
        DECREMENT   = 0b00000100,
        SPEED_UP    = 0b00001000,
        SPEED_DOWN  = 0b00001100,
    };

    const static uint8_t MAX_SPEED = 20;
    const static uint8_t MIN_SPEED = 1;

    volatile uint16_t time;

    uint8_t steps = 0;

    uint8_t current_speed = MIN_SPEED;

    volatile uint8_t remaining_time = 0;

    uint8_t field[SCREEN_HEIGHT][SCREEN_WIDTH];

    struct Point
    {
        int8_t x;
        int8_t y;

        inline
        bool operator == (Point& p)
        {
            return p.x == x && p.y == y;
        };
    };

    Point head;
    Point tail;

    struct Distance
    {
        int8_t x;
        int8_t y;

        Distance(const Point& start, const Point& end);

        bool is_zero();

        int8_t full();
    };

    struct PossibleStep
    {
        enum Priority
        {
            PREFERRED,
            POSSIBLE,
            NOT_RECOMMENDED,
            IMPOSSIBLE
        } priority;

        SnakeDirection direction;

        int8_t distance;
    };

    ArrayList<Point, 5> food;

public:
	SnakeGame();

    static void runner();

    void run();

    void on_timer1_event();

private:
    inline
    void refresh_remaining_time()
    {
        remaining_time = MAX_SPEED + 1 - current_speed;
    }

    bool do_step();

    void set(const uint8_t x, const uint8_t y, const uint8_t data);

    void set(const Point& point, const uint8_t data);

    bool move();

    void move_head(Point& next, SnakeDirection direction);

    void move_tail();

    void eat(Point& what_to_eat);

    bool place_food();

    uint8_t get_food();

    void eat_yourself(const Point& next);

    uint8_t get_next(const Point& point);

    SnakeDirection get_direction(const Point& point);

    Point get_next(const Point& current, SnakeDirection direction);

    SnakeDirection turn_left(SnakeDirection direction);

    SnakeDirection turn_right(SnakeDirection direction);

    PossibleStep make_decision();

    void possible_step(SnakeDirection direction, PossibleStep& step);

    void draw();

    void place_snake(uint8_t start_x, uint8_t start_y, uint8_t length);

};

#endif
