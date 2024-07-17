#pragma once

void printf(void (putc)(char c),
	void (puts)(char* str),
	const char* fmt, ...
);
