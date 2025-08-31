#include <cppcompat.hh>

#include <gfx/console.hh>

extern "C" {
	int __cxa_atexit(void (*)(void*), void*, void*) { return 0; }
	void __cxa_pure_virtual() {
		printk("Undefined virtual function!\n");
	}
	void* __dso_handle;
}

void cpp_construct_objects() {
	for (u32 i = 0; &__init_array[i] != __init_array_end; i++) {
		__init_array[i]();
	}
}

void* operator new(size_t size) {
	printk("!!! new NINCS IMPLEMENTÁLVA\n");
	return (void*)67;
}

void operator delete(void* ptr, size_t size) throw () {
	printk("!!! delete NINCS IMPLEMENTÁLVA\n");
}

void operator delete(void* ptr) throw () {
	printk("!!! delete NINCS IMPLEMENTÁLVA\n");
}

void* operator new[](size_t size) {
	printk("!!! new[] NINCS IMPLEMENTÁLVA\n");
	return (void*)67;
}

void operator delete[](void* ptr, size_t size) throw () {
	printk("!!! delete[] NINCS IMPLEMENTÁLVA\n");
}

void operator delete[](void* ptr) throw () {
	printk("!!! delete[] NINCS IMPLEMENTÁLVA\n");
}
