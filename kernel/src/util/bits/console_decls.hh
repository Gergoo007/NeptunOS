#pragma once

static constexpr unsigned int con_colors[5] = {
	0xffa0a0a0,
	0xffd0d0d0,
	0xffEB6534,
	0xffC41E3D,
	0xff710627,
};

void con_push_color(unsigned int color);
void con_pop_color();
void sputc(const char c);

__attribute__((format(printf, 1, 2)))
void printk(const char* fmt, ...);

__attribute__((format(printf, 1, 2)))
void printlnk(const char* fmt, ...);

__attribute__((format(printf, 1, 2)))
void sprintk(const char* fmt, ...);

__attribute__((format(printf, 3, 4)))
void printkx(unsigned int lvl, const char* FILENAME, const char* fmt, ...);
__attribute__((format(printf, 3, 4))) [[noreturn]]
void printkxnoret(unsigned int lvl, const char* FILENAME, const char* fmt, ...);

#ifdef IS_MODULE
struct ModuleMetadata;
extern const volatile ModuleMetadata _modinfo;
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

void con_clear();
