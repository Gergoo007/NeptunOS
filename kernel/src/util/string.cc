#include <util/string.hh>

static_assert(sizeof(wchar) == 2);

u32 ucs2_to_ascii(wchar* in, char* out) {
	u32 count = 0;
	while (in[count]) {
		out[count] = in[count];
		count++;
	}
	out[count] = 0;
	return count;
}

void ucs2_to_asciin(wchar* in, char* out, u32 n) {
	out[n] = '\0';
	while (n--) {
		*out = *in;
		out++;
		in++;
	}
}

u32 strcpy(const char* src, char* dest) {
	u32 len = 0;
	while (src[len]) {
		// *dest = *src;
		// src++;
		// dest++;
		dest[len] = src[len];
		len++;
	}
	// *dest = '\0';
	dest[len]= 0;
	return len;
}

u8 strncmp(const char* s1, const char* s2, u32 chars) {
	if (!chars || *s1 != *s2)
		return 1;
	while (chars-- && *s1 && *s2) {
		if (*s1 != *s2)
			return 1;
		s1++;
		s2++;
	}
	return 0;
}

u8 strcmp(const char* s1, const char* s2) {
	while (*s1 && *s2) {
		if (*s1 != *s2)
			return 1;
		s1++;
		s2++;
	}
	if (*s1 != *s2) return 1;
	return 0;
}

void strcat(char* dest, char* src) {
	while (*dest) dest++;

	while (*src) {
		*dest = *src;

		src++;
		dest++;
	}
	*dest = 0;
}
