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

#define atomic _Atomic

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

#define assert(c) if (!(c)) fatal("Assert failed: "#c" (" __FILE__ ":%d)", __LINE__)

#define align(x, n) ((typeof(x))(((x) & ((n)-1)) ? (((x) | ((n)-1))+1) : (x)))
#define align_down(x, n) ((typeof(x))(((u64)x) & ~(((u64)n)-1)))

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

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define abs(a) ((a) < 0 ? (-(a)) : (a))

#define bitset(x, n, b) ((typeof(x))((b) ? (((u64)x) | (1ULL << (n))) : (((u64)x) & ~(1ULL << (n)))))

#define VIRTUAL(a) ((typeof(a))(u64(a) | u64(higherhalf)))
#define PHYSICAL(a) ((typeof(a))(u64(a) & ~u64(higherhalf)))

__attribute__((format(printf, 1, 2)))
void printk(const char* fmt, ...);

__attribute__((format(printf, 1, 2)))
void sprintk(const char* fmt, ...);

namespace console { extern void push_color(u32 color); extern void pop_color(); }
#include <devmgr/moduleinfo.hh>
#ifdef IS_MODULE
#define report(fmt, ...) { console::push_color(0xffd0d0d0); extern volatile modules::ModuleInfo _modinfo; printk("[%s %s:%d]: " fmt, _modinfo.name, __FILE_NAME__, __LINE__, ##__VA_ARGS__); console::pop_color(); }
#define warn(fmt, ...) { console::push_color(0xffEB6534); extern volatile modules::ModuleInfo _modinfo; printk("[%s %s:%d]: " fmt, _modinfo.name, __FILE_NAME__, __LINE__, ##__VA_ARGS__); console::pop_color(); }
#define error(fmt, ...) { console::push_color(0xffC41E3D); extern volatile modules::ModuleInfo _modinfo; printk("[%s %s:%d]: " fmt, _modinfo.name, __FILE_NAME__, __LINE__, ##__VA_ARGS__); console::pop_color(); }
#define fatal(fmt, ...) { console::push_color(0xff710627); extern volatile modules::ModuleInfo _modinfo; printk("[%s %s:%d]: " fmt, _modinfo.name, __FILE_NAME__, __LINE__, ##__VA_ARGS__); while (1) asm volatile ("cli\nhlt"); }
#else
#define report(fmt, ...) { console::push_color(0xffd0d0d0); printk("[%s:%d]: " fmt, __FILE_NAME__, __LINE__, ##__VA_ARGS__); console::pop_color(); }
#define warn(fmt, ...) { console::push_color(0xffEB6534); printk("[%s:%d]: " fmt, __FILE_NAME__, __LINE__, ##__VA_ARGS__); console::pop_color(); }
#define error(fmt, ...) { console::push_color(0xffC41E3D); printk("[%s:%d]: " fmt, __FILE_NAME__, __LINE__, ##__VA_ARGS__); console::pop_color(); }
#define fatal(fmt, ...) { console::push_color(0xff710627); printk("[%s:%d]: " fmt, __FILE_NAME__, __LINE__, ##__VA_ARGS__); while (1) asm volatile ("cli\nhlt"); }
#endif

void hlt();

#define pause() while(1) { hlt(); }

// thank you osdev.org
static inline int oct2bin(u8* str, int size) {
    int n = 0;
    u8* c = str;
    while (size-- > 0) {
        n *= 8;
        n += *c - '0';
        c++;
    }
    return n;
}
