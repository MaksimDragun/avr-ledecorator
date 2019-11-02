#include "screen.h"

Screen::Screen(DisplayInterface* display_interface, DataInterface* data_interface, TimerInterface* timer_interface)
{
	this->display_interface = display_interface;
	this->data_interface = data_interface;
	this->timer_interface = timer_interface;

	buffer_1 = new uint8_t*[SCREEN_HEIGHT];
	buffer_2 = new uint8_t*[SCREEN_HEIGHT];
	for (uint8_t row = 0; row < SCREEN_HEIGHT; row++)
	{
		buffer_1[row] = new uint8_t[SCREEN_WIDTH];
		buffer_2[row] = new uint8_t[SCREEN_WIDTH];
	}
	active_buffer = buffer_1;
	buffer = buffer_2;

	is_being_read = 0;

	current_row = Row();
	rows_state = 0xFF;
}

inline void Screen::launch()
{
	this->timer_interface->launch();
}

inline void Screen::stop() {
	this->timer_interface->stop();
}

void Screen::switch_buffer()
{
	while (is_being_read);
	uint8_t** temp = active_buffer;
	active_buffer = buffer;
	buffer = temp;
}

inline void Screen::start_reading()
{
	is_being_read = 1;
}

inline void Screen::stop_reading()
{
	is_being_read = 0;
}

void Screen::apply_colors(Colors& colors, const uint8_t color, const uint8_t currentBit)
{
	if (color > 0b00)
	{
		colors.level0 |= currentBit;
		colors.level1 |= currentBit;
		if (color > 0b01)
		{
			colors.level2 |= currentBit;
			if (color > 0b10)
			{
				colors.level3 |= currentBit;
			}
		}
	}
}

void Screen::apply_colors(Section& section, const uint8_t color, const uint8_t offset)
{
	uint8_t currentBit = (1<<offset);
	apply_colors(section.red, (color & RED), currentBit);
	apply_colors(section.green, (color & GREEN) >> 2, currentBit);
	apply_colors(section.blue, (color & BLUE) >> 4, currentBit);
}

void Screen::draw_row()
{
	if (current_row.brightness_step == 0)
	{
		if (current_row.index == 0)
		{
			start_reading();
		}
		current_row.reset();
		for (uint8_t sectionIdx = 0; sectionIdx < SECTIONS; sectionIdx++)
		{
			// 0,1,2,3,4,5,6,7...
			uint8_t sectionRowIdx = sectionIdx / SECTIONS_PER_ROW;
			// 0,0,0,0,1,1,1,1...
			uint8_t pictureY = (H_SECTION * sectionRowIdx) + current_row.index;
			if (sectionRowIdx % 2 == 0) {
				// not-inverted
				// sectionRowIdx: 0
				uint8_t sectionColumnIdx = sectionIdx % SECTIONS_PER_ROW;
				// sectionColumnIdx: 0,1,2,3
				// pictureY: 0,1,2,3,4,5,6,7
				uint8_t pictureX = (W_SECTION * sectionColumnIdx);
				for (uint8_t xOffset = 0; xOffset < W_SECTION; xOffset++)
				{
					uint8_t value = active_buffer[pictureY][pictureX + xOffset];
					apply_colors(current_row.sections[sectionIdx], value, xOffset);
				}
			}
			else
			{
				// inverted
				// sectionRowIdx: 1
				uint8_t sectionColumnIdx = SECTIONS_PER_ROW - 1 - (sectionIdx % SECTIONS_PER_ROW);
				// sectionColumnIdx: 3,2,1,0
				// pictureY: 8,9,10,11,12,13,14,15
				uint8_t pictureX = (W_SECTION * sectionColumnIdx);
				for (uint8_t xOffset = 0; xOffset < W_SECTION; xOffset++)
				{
					uint8_t value = active_buffer[pictureY][pictureX + W_SECTION - 1 - xOffset];
					apply_colors(current_row.sections[sectionIdx], value, xOffset);
				}
			}

		}
		rows_state = ~(1<<current_row.index);
		if (current_row.index == ROWS - 1)
		{
			stop_reading();
		}
		display_interface->start_row();
		display_interface->send_byte(rows_state);
		for (uint8_t sectionIdx = 0; sectionIdx < SECTIONS; sectionIdx++)
		{
			Section section = current_row.sections[sectionIdx];
			display_interface->send_byte(section.red.level0);
			display_interface->send_byte(section.green.level0);
			display_interface->send_byte(section.blue.level0);
		}
		display_interface->complete_row();
	}
	else
	{
		display_interface->start_row();
		display_interface->send_byte(rows_state);
		for (uint8_t sectionIdx = 0; sectionIdx < SECTIONS; sectionIdx++)
		{
			Section section = current_row.sections[sectionIdx];

			if (current_row.brightness_step > 2)
			{
				display_interface->send_byte(section.red.level3);
				display_interface->send_byte(section.green.level3);
				display_interface->send_byte(section.blue.level3);
			}
			else if (current_row.brightness_step > 0)
			{
				display_interface->send_byte(section.red.level2);
				display_interface->send_byte(section.green.level2);
				display_interface->send_byte(section.blue.level2);
			}
			else if (current_row.brightness_step > 0)
			{
				display_interface->send_byte(section.red.level1);
				display_interface->send_byte(section.green.level1);
				display_interface->send_byte(section.blue.level1);
			}
			else
			{
				display_interface->send_byte(section.red.level0);
				display_interface->send_byte(section.green.level0);
				display_interface->send_byte(section.blue.level0);
			}

		}
		display_interface->complete_row();
	}

	if (++current_row.brightness_step == 4)
	{
		current_row.reset();
		if (++current_row.index == ROWS)
		{
			current_row.index = 0;
		}
	}
}
