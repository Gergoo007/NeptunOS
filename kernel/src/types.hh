#pragma once

// Miért van redefinition hiba az ifndef nélkül?
#include <config.hh>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef __uint128_t u128;

typedef char i8;
typedef short i16;
typedef int i32;
typedef long long i64;
typedef __int128_t i128;

typedef u8 uint8_t;
typedef u16 uint16_t;
typedef u32 uint32_t;
typedef u64 uint64_t;

typedef i8 int8_t;
typedef i16 int16_t;
typedef i32 int32_t;
typedef i64 int64_t;

typedef u16 wchar;

typedef u64 uintptr_t;

typedef __SIZE_TYPE__ size_t;

// #define atomic _Atomic

#define UINT64_C(v) v##ULL

#define va_start(v, l)	__builtin_va_start(v,l)
#define va_end(v)		__builtin_va_end(v)
#define va_arg(v,l)		__builtin_va_arg(v,l)
#define va_list __builtin_va_list

#define foreach(var, l) for (i64 var = 0; var < (l); var++)
#define offsetof(s, m) __builtin_offsetof(s, m)

#define bytes2kibs(bytes) ((bytes) >> 10)
#define bytes2mibs(bytes) ((bytes) >> 20)
#define bytes2gibs(bytes) ((bytes) >> 30)
#define bytes2tibs(bytes) ((bytes) >> 40)

#define kib2bytes(kibs) ((kibs) << 10)
#define mib2bytes(mibs) ((mibs) << 20)
#define gib2bytes(gibs) ((gibs) << 30)
#define tib2bytes(tibs) ((tibs) << 40)

#define assert(c) if (!(c)) fatal("Assert failed: %s (" __FILE__ ":%d)", #c, __LINE__)

// #define align(x, n) ((typeof(x))(((x) & ((n)-1)) ? (((x) | ((n)-1))+1) : (x)))
#define align_down(x, n) ((typeof(x))(((u64)x) & ~(((u64)n)-1)))

#define align(x, n) ((typeof(x))((((u64)(x)) % (n)) ? (((u64)(x) + (n)) - (((u64)(x))) % (n)) : (x)))

#define noret __attribute__((noreturn))
#define packed __attribute__((packed))
#define interrupt __attribute__((interrupt))
#define aligned(x) __attribute__((aligned(x)))

#define pstruct struct packed
#define punion union packed

extern u8 _binary_src_font_psf_start;
extern u8 _binary_src_font_psf_end;

// higher half
extern void* higherhalf;

#define FONTFILE_START &_binary_src_font_psf_start
#define FONTFILE_END &_binary_src_font_psf_end

// #define min(a, b) ((a) < (b) ? (a) : (b))
template <typename T>
constexpr T min(const T& asd1) { return asd1; }
template <typename T, typename U>
constexpr T min(const T& asd1, const U& asd2) { return asd1 < asd2 ? asd1 : asd2; }
template <typename T, typename U, typename... Args>
constexpr T min(const T& asd1, const U& asd2, const Args&... args) { return min(min(asd1, asd2), args...); }

// #define max(a, b) ((a) > (b) ? (a) : (b))
template <typename T>
constexpr T max(const T& asd1) { return asd1; }
template <typename T, typename U>
constexpr T max(const T& asd1, const U& asd2) { return asd1 < asd2 ? asd1 : asd2; }
template <typename T, typename U, typename... Args>
constexpr T max(const T& asd1, const U& asd2, const Args&... args) { return max(max(asd1, asd2), args...); }

// #define abs(a) ((a) < 0 ? (-(a)) : (a))
template <typename T>
constexpr T abs(const T& asd1) { if (asd1 < 0) return -asd1; else return asd1; }

#define bitset(x, n, b) ((typeof(x))((b) ? (((u64)x) | (1ULL << (n))) : (((u64)x) & ~(1ULL << (n)))))

template <typename T>
constexpr T VIRTUAL(T addr) { return (T)((u64)addr | (u64)higherhalf); }

template <typename T>
constexpr T PHYSICAL(T addr) { return (T)((u64)addr & ~(u64)higherhalf); }

__attribute__((format(printf, 1, 2)))
void printk(const char* fmt, ...);

__attribute__((format(printf, 1, 2)))
void printk(const char* fmt, ...);

__attribute__((format(printf, 1, 2)))
void sprintk(const char* fmt, ...);

__attribute__((format(printf, 3, 4)))
void printkx(u32 lvl, const char* FILENAME, const char* fmt, ...);
__attribute__((format(printf, 3, 4))) [[noreturn]]
void printkxnoret(u32 lvl, const char* FILENAME, const char* fmt, ...);
#ifdef IS_MODULE
struct module_metadata_t;
extern const volatile module_metadata_t _modinfo;
#define debug(fmt, ...) printkx(0, __FILE_NAME__, "[%s %s:%d]: " fmt "\n", (const char*)&_modinfo, __FILE_NAME__, __LINE__, ##__VA_ARGS__)
#define report(fmt, ...) printkx(1, __FILE_NAME__, "[%s %s:%d]: " fmt "\n", (const char*)&_modinfo, __FILE_NAME__, __LINE__, ##__VA_ARGS__)
#define warn(fmt, ...) printkx(2, __FILE_NAME__, "[%s %s:%d]: " fmt "\n", (const char*)&_modinfo, __FILE_NAME__, __LINE__, ##__VA_ARGS__)
#define error(fmt, ...) printkx(3, __FILE_NAME__, "[%s %s:%d]: " fmt "\n", (const char*)&_modinfo, __FILE_NAME__, __LINE__, ##__VA_ARGS__)
#define fatal(fmt, ...) printkxnoret(4, __FILE_NAME__, "[%s %s:%d]: " fmt "\n", (const char*)&_modinfo, __FILE_NAME__, __LINE__, ##__VA_ARGS__)
#else
#define debug(fmt, ...) printkx(0, __FILE_NAME__, "[%s:%d]: " fmt "\n", __FILE_NAME__, __LINE__, ##__VA_ARGS__)
#define report(fmt, ...) printkx(1, __FILE_NAME__, "[%s:%d]: " fmt "\n", __FILE_NAME__, __LINE__, ##__VA_ARGS__)
#define warn(fmt, ...) printkx(2, __FILE_NAME__, "[%s:%d]: " fmt "\n", __FILE_NAME__, __LINE__, ##__VA_ARGS__)
#define error(fmt, ...) printkx(3, __FILE_NAME__, "[%s:%d]: " fmt "\n", __FILE_NAME__, __LINE__, ##__VA_ARGS__)
#define fatal(fmt, ...) printkxnoret(4, __FILE_NAME__, "[%s:%d]: " fmt "\n", __FILE_NAME__, __LINE__, ##__VA_ARGS__)
#endif

void arch_halt();
[[noreturn]] static inline void pause() { while (1) { arch_halt(); } }

// thank you osdev.org
static inline int oct2bin(u8* str, int size) {
    int n = 0;
    while (size-- > 0) {
        n *= 8;
        n += *(str++) - '0';
    }
    return n;
}

extern volatile u64 tmr_counter;
void sched_setrunning(bool otoole);
