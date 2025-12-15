#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef size_t	(*formatter)(const char *restrict *fmt, va_list params,
	size_t max_len);

static size_t	format_c(const char *restrict *fmt, va_list params,
	size_t max_len);
static size_t	format_s(const char *restrict *fmt, va_list params,
	size_t max_len);
static size_t	format_d(const char *restrict *fmt, va_list params,
	size_t max_len);
static size_t	format_default(const char *restrict *fmt, va_list params,
	size_t max_len);

enum {
	_FMT_CHAR,
	_FMT_STRING,
	_FMT_INTEGER,
	_FMT_DIGITAL,
	_FMT_COUNT,
};

static const char		formatter_ids[_FMT_COUNT] =
{
	[_FMT_CHAR] = 'c',
	[_FMT_STRING] = 's',
	[_FMT_INTEGER] = 'i',
	[_FMT_DIGITAL] = 'd',
};

static const formatter	formatters[_FMT_COUNT + 1] =
{
	[_FMT_CHAR] = format_c,
	[_FMT_STRING] = format_s,
	[_FMT_INTEGER] = format_d,
	[_FMT_DIGITAL] = format_d,
	[_FMT_COUNT] = format_default,
};

static size_t		print(const char* data, size_t length)
{
	const unsigned char*	bytes = (const unsigned char*) data;

	for (size_t i = 0; i < length; i++)
		if (putchar(bytes[i]) == EOF)
			return -1;

	return length;
}

static size_t		format_c(const char *restrict *fmt, va_list params,
	size_t max_len)
{
	const char	c = (char) va_arg(params, int);

	(*fmt) += 2;

	if (max_len == 0) {
		// TODO: Set errno to EOVERFLOW.
		return -1;
	}

	if (!print(&c, sizeof(c)))
		return -1;

	return 1;
}

static size_t		format_s(const char *restrict *fmt, va_list params,
	size_t max_len)
{
	const char *const	str = va_arg(params, const char*);
	const size_t		len = strlen(str);

	(*fmt) += 2;

	if (max_len < len) {
		// TODO: Set errno to EOVERFLOW.
		return -1;
	}

	return print(str, len);
}


static size_t		format_d(const char *restrict *fmt, va_list params,
	size_t max_len)
{
	char		representation[11];
	size_t		len;
	int			value;
	char		sign;

	len = 0;
	value = va_arg(params, int);

	(*fmt) += 2;

	if (value < 0) {
		sign = '-';
		value = -value;
	}
	else {
		sign = '\0';
	}

	do {
		representation[sizeof(representation) - len++ - 1] = '0' + value % 10;
		value /= 10;
	} while (value != 0);

	if (sign != '\0')
		representation[sizeof(representation) - len++ - 1] = sign;

	if (max_len < len) {
		// TODO: Set errno to EOVERFLOW.
		return -1;
	}

	return print(representation + sizeof(representation) - len, len);
}

static size_t		format_default(const char *restrict *fmt, va_list params,
	size_t max_len)
{
	(void)params;
	size_t	len = strlen(*fmt);

	if (max_len < len) {
		// TODO: Set errno to EOVERFLOW.
		return -1;
	}

	len = print(*fmt, len);

	if (len != -1U)
		(*fmt) += len;

	return len;
}

static formatter	formatter_id(char c)
{
	size_t	i;

	for (
		i = 0;
		i < sizeof(formatter_ids) / sizeof(*formatter_ids)
		&& formatter_ids[i] != c;
		++i
	);

	return formatters[i];
}

static int			format(const char *restrict *fmt, va_list params, size_t max_len)
{
	const formatter		formatter = formatter_id((*fmt)[1]);

	return formatter(fmt, params, max_len);
}

int					printf(const char* restrict fmt, ...)
{
	va_list	params;
	size_t	len;

	va_start(params, fmt);

	int written_len = 0;

	while (*fmt != '\0') {
		size_t max_len = INT_MAX - written_len;

		if (fmt[0] != '%' || fmt[1] == '%') {
			if (fmt[0] == '%')
				fmt++;

			len = 1;

			while (fmt[len] && fmt[len] != '%')
				++len;

			if (max_len < len) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}

			len = print(fmt, len);

			if (len != -1U)
				fmt += len;
		}
		else {
			len = format(&fmt, params, max_len);
		}

		if (len == -1U) {
			written_len = -1;
			goto error;
		}

		written_len += len;
	}

error:
	va_end(params);

	return written_len;
}
