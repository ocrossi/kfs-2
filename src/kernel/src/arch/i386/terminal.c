#include <kernel/terminal.h>
#include <vga.h>
#include <string.h>

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

struct terminal terminal;

void terminal_clear_at(size_t x, size_t y)
{
	for (; y < VGA_HEIGHT; y++) {
		for (; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;

			terminal.buffer[index] = vga_entry(' ', terminal.color);
		}
		x = 0;
	}
}

void terminal_clear(void)
{
	terminal_clear_at(0, 0);
}

void terminal_initialize(void)
{
	terminal.row = 0;
	terminal.column = 0;

	terminal.color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal.buffer = (uint16_t*)0xB8000;

	terminal_clear();
}

size_t	terminal_get_row(void)
{
	return terminal.row;
}

void terminal_setcolor(uint8_t color)
{
	terminal.color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
	const size_t index = y * VGA_WIDTH + x;

	terminal.buffer[index] = vga_entry(c, color);
}

void terminal_scroll_down(unsigned y)
{
	const size_t delta = y * VGA_WIDTH;
	const size_t new_length = VGA_WIDTH * (VGA_HEIGHT - y);

	memmove(terminal.buffer, terminal.buffer + delta, sizeof(*terminal.buffer) * new_length);
	terminal.row -= y;
	terminal_clear_at(0, terminal.row + 1);
}

void terminal_move_rel(int x, int y)
{
	if (terminal.column + x >= VGA_WIDTH)
	{
		y += (terminal.column + x) / VGA_WIDTH;
	}

	x = terminal.column + x % VGA_WIDTH;

	if (terminal.row + y >= VGA_HEIGHT)
	{
		terminal_scroll_down(terminal.row + y - VGA_HEIGHT + 1);
		y = VGA_HEIGHT - 1;
	}
	else
	{
		y = terminal.row + y;
	}

	terminal.column = x;
	terminal.row = y;
}

void terminal_newline()
{
	terminal_move_rel(0, 1);
	terminal.column = 0;
}

void terminal_putchar(char c)
{
	switch (c) {
		case '\n':
			terminal_newline();
			break;
		default:
			terminal_putentryat(c, terminal.color, terminal.column, terminal.row);
			terminal_move_rel(1, 0);
	}
}

int terminal_write(const char* data, size_t size)
{
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);

	return size;
}
