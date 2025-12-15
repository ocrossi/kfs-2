#pragma once

#include <stddef.h>
#include <stdint.h>

struct terminal {
	size_t		row;
	size_t		column;
	uint8_t		color;
	uint16_t	*buffer;
};

void	terminal_clear(void);
void	terminal_initialize(void);

size_t	terminal_get_row(void);

void	terminal_setcolor(uint8_t color);
void	terminal_move_rel(int x, int y);
void	terminal_newline(void);

void	terminal_putentryat(char c, uint8_t color, size_t x, size_t y);
void	terminal_putchar(char c);
int		terminal_write(const char* data, size_t size);
