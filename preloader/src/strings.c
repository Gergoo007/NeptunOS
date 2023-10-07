#include <strings.h>

// Return string length WITHOUT counting the null terminator
u8 strlen(const char* str) {
	u8 len = 1;
	while (*(str++))
		len++;

	return len;
}
