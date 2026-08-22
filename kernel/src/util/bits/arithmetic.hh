#pragma once

#include <util/bits/int.hh>

#define bytes2kibs(bytes) ((bytes) >> 10)
#define bytes2mibs(bytes) ((bytes) >> 20)
#define bytes2gibs(bytes) ((bytes) >> 30)
#define bytes2tibs(bytes) ((bytes) >> 40)

#define kibs2bytes(kibs) ((kibs) << 10)
#define mibs2bytes(mibs) ((mibs) << 20)
#define gibs2bytes(gibs) ((gibs) << 30)
#define tibs2bytes(tibs) ((tibs) << 40)

// #define min(a, b) ((a) < (b) ? (a) : (b))
template <typename T>
constexpr T min(const T& asd1) { return asd1; }
template <typename T, typename U>
constexpr T min(const T& asd1, const U& asd2) { return asd1 < asd2 ? asd1 : asd2; }
template <typename T, typename U, typename... Args>
constexpr T min(const T& asd1, const U& asd2, const Args&... args) { return min(min(asd1, asd2), args...); }

static_assert(min(10, 20, 30) == 10);
static_assert(min(30, 20, 10) == 10);
static_assert(min(30, 10, 20) == 10);

// #define max(a, b) ((a) > (b) ? (a) : (b))
template <typename T>
constexpr T max(const T& asd1) { return asd1; }
template <typename T, typename U>
constexpr T max(const T& asd1, const U& asd2) { return asd1 > asd2 ? asd1 : asd2; }
template <typename T, typename U, typename... Args>
constexpr T max(const T& asd1, const U& asd2, const Args&... args) { return max(max(asd1, asd2), args...); }

// #define abs(a) ((a) < 0 ? (-(a)) : (a))
template <typename T>
constexpr T abs(const T& asd1) { if (asd1 < 0) return -asd1; else return asd1; }

#define bitset(x, n, b) ((typeof(x))((b) ? (((u64)x) | (1ULL << (n))) : (((u64)x) & ~(1ULL << (n)))))

// Count leading zeroes
#define clz(x) __builtin_clz(x)
// Count trailing zeroes
#define ctz(x) __builtin_ctz(x)
// Population count
#define popcount(x) __builtin_popcount(x)
#define parity(x) __builtin_parity(x)

#define ISDIGIT(x) ((x) >= '0' && (x) <= '9')
#define ISLOWERCASE(x) ((x) >= 'a' && (x) <= 'z')
#define ISUPPERCASE(x) ((x) >= 'A' && (x) <= 'Z')
#define ISLETTER(x) (ISLOWERCASE(x) || ISUPPERCASE(x))

// #define align(x, n) ((typeof(x))(((x) & ((n)-1)) ? (((x) | ((n)-1))+1) : (x)))
// #define align_down(x, n) ((typeof(x))(((u64)x) & ~(((u64)n)-1)))
// #define align(x, n) ((typeof(x))((((u64)(x)) % (n)) ? (((u64)(x) + (n)) - (((u64)(x))) % (n)) : (x)))

#define align(x, a) ((((x) + ((a)-1)) / (a)) * (a))
#define align_down(x, a) ((x) / (a) * (a))

#define isaligned(x, a) ((((u64)(x)) % ((u64)(a))) == 0)

template <typename T>
constexpr T VIRTUAL(T addr) { return (T)((u64)addr | (u64)0xffff800000000000); }

template <typename T>
constexpr T PHYSICAL(T addr) { return (T)((u64)addr & ~(u64)0xffff800000000000); }
