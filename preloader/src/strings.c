#include <strings.h>

// Return string length WITHOUT counting the null terminator
u8 strlen(const char* str) {
	u8 len = 1;
	while (*(str++))
		len++;

	return len;
}

void strcpy(const char* src, char* dest) {
	while (*src != '\0') {
		*dest = *src;
		src++;
		dest++;
	}
	*dest = '\0';
}

u8 strncmp(const char* s1, const char* s2, u32 chars) {
	while (chars--) {
		if (*s1 != *s2)
			return 1;
	}
	return 0;
}

void memcpy(const u8* src, u8* dest, u64 i) {
	while (i--) {
		*dest = *src;
		src++;
		dest++;
	}
}
