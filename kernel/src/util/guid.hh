//-------------------------------------------------------------------------------------------------------
// constexpr GUID parsing
// Written by Alexander Bessonov
//
// Licensed under the MIT license.
//-------------------------------------------------------------------------------------------------------

#pragma once

#include <types.hh>
#include <util/storage.hh>
#include <util/printf.hh>

union GUID {
	struct {
		u32 Data1;
		u16 Data2;
		u16 Data3;
		u8 Data4[8];
	};
	u128 raw;

	constexpr u128 getraw() const {
		return (u128)Data1 | ((u128)Data2 << 32) | ((u128)Data3 << 48) |
			((u128)Data4[0] << 64) | ((u128)Data4[1] << 72) | ((u128)Data4[2] << 80) | ((u128)Data4[3] << 88) |
			((u128)Data4[4] << 96) | ((u128)Data4[5] << 104) | ((u128)Data4[6] << 112) | ((u128)Data4[7] << 120);
	}
	constexpr bool operator==(const GUID& g) { return g.raw == raw; }
	constexpr bool operator!=(const GUID& g) { return g.raw != raw; }
};

namespace guid_parse {
	namespace details {
		constexpr const size_t short_guid_form_length = 36;	// XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
		constexpr const size_t long_guid_form_length = 38;	// {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}

		constexpr int parse_hex_digit(const char c) {
			if ('0' <= c && c <= '9')
				return c - '0';
			else if ('a' <= c && c <= 'f')
				return 10 + c - 'a';
			else if ('A' <= c && c <= 'F')
				return 10 + c - 'A';
			else
				fatal("he");
		};
	}

	template<class T>
	constexpr T parse_hex(const char *ptr) {
		constexpr size_t digits = sizeof(T) * 2;
		T result{};
		for (size_t i = 0; i < digits; ++i)
			result |= details::parse_hex_digit(ptr[i]) << (4 * (digits - i - 1));
		return result;
	}

	constexpr GUID make_guid_helper(const char *begin) {
		GUID result{};
		result.Data1 = parse_hex<u32>(begin);
		begin += 8 + 1;
		result.Data2 = parse_hex<u16>(begin);
		begin += 4 + 1;
		result.Data3 = parse_hex<u16>(begin);
		begin += 4 + 1;
		result.Data4[0] = parse_hex<u8>(begin);
		begin += 2;
		result.Data4[1] = parse_hex<u8>(begin);
		begin += 2 + 1;
		for (size_t i = 0; i < 6; ++i)
			result.Data4[i + 2] = parse_hex<u8>(begin + i * 2);
		return result;
	}

	template<size_t N>
	constexpr GUID make_guid(const char (&str)[N]) {
		static_assert(N == (details::long_guid_form_length + 1) || N == (details::short_guid_form_length + 1), "String GUID of the form {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX} or XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX is expected");

		if constexpr(N == (details::long_guid_form_length + 1)) {
			if (str[0] != '{' || str[details::long_guid_form_length - 1] != '}')
				fatal("he2");
		}

		return make_guid_helper(str + (N == (details::long_guid_form_length + 1) ? 1 : 0));
	}
}

static inline String guid2string(u8* guid) {
	String ret;
	ret.resize(46);
	sprintf(ret.data, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		*(u32*)guid, *(u16*)(guid + 4), *(u16*)(guid + 6), *(u8*)(guid + 8), *(u8*)(guid + 9),
		(u32)*(u8*)(guid + 10), (u32)*(u8*)(guid + 11), (u32)*(u8*)(guid + 12), (u32)*(u8*)(guid + 13), (u32)*(u8*)(guid + 14), (u32)*(u8*)(guid + 15));
	return ret;
}

