#include <stdlib.h>
#include <string.h>
#include "lib/screen/definitions.h"
#include "entities.hpp"
#include "lifegame.hpp"

const LifeGame::StoredState EEMEM LifeGame::STORED_STATE = LifeGame::StoredState();

LifeGame::LifeGame()
{
    eeprom_read_block((void*) &state, (const void*) &STORED_STATE, sizeof(state));
    time_to_live = state.time_to_live;
    alive_indicator = ALIVE_INDICATOR_01;
    switch (state.mode)
    {
    case Mode::RANDOM:
        script = (Script) (rand() % TOTAL_SCRIPTS);
        break;
    default:
        script = state.script;
        break;
    }

    switch (script)
    {
    case Script::RANDOM_SEA:
        random_field();
        break;
    case Script::SHIPS:
        place_entity<5, 4>(0, 0, alive_indicator, &Entities::SHIP_LIGHT);
        place_entity<6, 5>(10, 4, alive_indicator, &Entities::SHIP_MEDIUM);
        place_entity<7, 5>(20, 9, alive_indicator, &Entities::SHIP_LARGE);
        break;
    case Script::SHIPS_RANDOM:
        place_entity<5, 4>(0, 0, alive_indicator, &Entities::SHIP_LIGHT);
        place_entity<6, 5>(10, 0, alive_indicator, &Entities::SHIP_MEDIUM);
        place_entity<7, 5>(20, 0, alive_indicator, &Entities::SHIP_LARGE);
        break;
    case Script::COPERHEAD:
        place_entity<12, 8>(12, 4, alive_indicator, &Entities::COPPERHEAD);
        break;
    default:
        break;
    }

	steps = 0;
	is_step_required = 0;
}

LifeGame::~LifeGame()
{
    if (state.mode == Mode::CAROUSEL)
    {
        uint8_t script = state.script;
        if (++script == TOTAL_SCRIPTS)
        {
           script = Script::RANDOM_SEA;
        }
        state.script = (Script) script;
        eeprom_update_block((const void*) &state, (void*) &STORED_STATE, sizeof(STORED_STATE));
    }
}

void LifeGame::random_field()
{
	for (uint8_t y = 0; y < SCREEN_HEIGHT; y++)
	{
		for (uint8_t x = 0; x < SCREEN_WIDTH; x++)
		{
			field[y][x] = rand() % 2 ? alive_indicator : 0;
		}
	}
}

void LifeGame::build_scene()
{
	for (uint8_t row = 0; row < SCREEN_HEIGHT; row++)
	{
		for (uint8_t cell = 0; cell < SCREEN_WIDTH; cell++)
		{
			Color color = field[row][cell] & alive_indicator ? state.color_life : state.color_dead;
			dragberry::os::display::set_pixel(row, cell, color);
		}
	}
	dragberry::os::display::update_assured();
}

void LifeGame::step_up()
{
	uint8_t next_alive_indicator = alive_indicator ==
			ALIVE_INDICATOR_01 ? ALIVE_INDICATOR_10 : ALIVE_INDICATOR_01;
	uint8_t next_dead_indicator = ~next_alive_indicator;

	for (uint8_t y = 0; y < SCREEN_HEIGHT; y++)
	{
		uint8_t* top_row = field[y == 0 ? SCREEN_HEIGHT - 1 : y - 1];
		uint8_t* current_row = field[y];
		uint8_t* bottom_row = field[y == SCREEN_HEIGHT - 1 ? 0 : y + 1];
		for (uint8_t x = 0; x < SCREEN_WIDTH; x++)
		{
			uint8_t alive_neigbhoors = 0;
			uint8_t left = x == 0 ? SCREEN_WIDTH - 1 : x - 1;
			uint8_t right = x == SCREEN_WIDTH - 1 ? 0 : x + 1;

			if (top_row[left] & alive_indicator)
			{
				alive_neigbhoors++;
			}
			if (top_row[x] & alive_indicator)
			{
				alive_neigbhoors++;
			}
			if (top_row[right] & alive_indicator)
			{
				alive_neigbhoors++;
			}
			if (current_row[right] & alive_indicator)
			{
				alive_neigbhoors++;
			}
			if (bottom_row[right] & alive_indicator)
			{
				alive_neigbhoors++;
			}
			if (bottom_row[x] & alive_indicator)
			{
				alive_neigbhoors++;
			}
			if (bottom_row[left] & alive_indicator)
			{
				alive_neigbhoors++;
			}
			if (current_row[left] & alive_indicator)
			{
				alive_neigbhoors++;
			}

			uint8_t cell = field[y][x];
 			field[y][x] = (alive_neigbhoors == 3 || (alive_neigbhoors == 2 && (cell & alive_indicator))) ?
					cell | next_alive_indicator : cell & next_dead_indicator;
		}
	}

	if (script == Script::SHIPS_RANDOM)
	{
        if (time >= 16 && (time % (14 + rand() % 8)) == 0)
        {
            if (rand() % 2)
            {
                place_entity<3, 3>(15, 11, next_alive_indicator, &Entities::GLIDER);
            }
            else
            {
                place_entity<6, 5>(0, 8, next_alive_indicator, &Entities::SHIP_MEDIUM);
            }
        }
	}

	alive_indicator = next_alive_indicator;

}

void LifeGame::on_timer_event()
{
	increment_time();
	if (time % state.speed == 0) {
	    is_step_required = true;
	}
}

void LifeGame::run()
{
    System::register_timer(this, TICKS_PER_SECOND);
    is_step_required = true;
    dragberry::os::display::clear_screen(BLACK);
    dragberry::os::display::update_assured();
	do
	{
		if (is_step_required)
		{
            System::io::exchange(
                [&](char* frame) -> void
                {
                    if (load_requested) {
                        eeprom_read_block((void*) &state, (const void*) &STORED_STATE, sizeof(state));
                    }
                    frame[1] = System::APP_LIFE;
                    System::io::decompose(time, 2);
                    System::io::decompose(steps, 4);
                    frame[6] = load_requested ? Command::LOAD : Command::IDLE;
                    frame[7] = state.color_life;
                    frame[8] = state.color_dead;
                    frame[9] = state.mode;
                    frame[10] = script;
                    System::io::decompose(state.time_to_live / TICKS_PER_SECOND, 11);
                    frame[13] = state.speed;
                    load_requested = false;
                },
                [&](char* frame) -> void {
                    switch (frame[3])
                    {
                    case Command::SAVE:
                        {
                            state.color_life = frame[4];
                            state.color_dead = frame[5];
                            state.mode = (Mode) frame[6];
                            state.script = (Script) frame[7];
                            System::io::compose(state.time_to_live, 8);
                            state.time_to_live *= TICKS_PER_SECOND;
                            state.speed = frame[10];
                            eeprom_update_block((const void*) &state, (void*) &STORED_STATE, sizeof(STORED_STATE));
                            break;
                        }
                    case Command::LOAD:
                        {
                            load_requested = true;
                            break;
                        }
                    default:
                        break;
                    }
                }
            );

		    build_scene();
			step_up();
			is_step_required = false;
			steps++;
		}
	}
	while (is_going_on());
	System::deregister_timer();
}
