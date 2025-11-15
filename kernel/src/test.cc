#include <test.hh>

#include <util/storage.hh>

struct Test {
	const char* name;

	Test(const char* name): name(name) {
		printk("%s tests commencing...\n", name);
	}

	~Test() {
		printk("%s test have finished\n", name);
	}
};

void test_libk() {
	{
		auto o = Test("libk->Vector");

		vector<u32> vec;
		vec.emplace(20);
		vec.emplace(30);
		vec.emplace(40);

		vector<u32> vec4 { 20, 30, 40 };
		assert(vec4.size == 3);
		assert(vec4[0] == 20);
		assert(vec4[1] == 30);
		assert(vec4[2] == 40);

		assert(vec.size == 3);
		
		auto vec2 = vec;
		assert(vec2.size == 3);
		assert(vec2[0] == 20);
		assert(vec2[1] == 30);
		assert(vec2[2] == 40);

		auto vec3 = move<vector<u32>>(vec);
		assert(vec3.size == 3);
		assert(vec3[0] == 20);
		assert(vec3[1] == 30);
		assert(vec3[2] == 40);

		assert(!vec.data);
	}

	{
		auto o = Test("libk->String");

		string teszt = "";
	}

	{
		auto o = Test("libk->Variant");
	}
}

void test_and_pause() {
	u32 allocs = vmm_count_allocs();

	// test_mm();
	test_libk();

	if (vmm_count_allocs() != allocs)
		error("Mismatch between vmm link count: %d", allocs);
	else
		printkx(0x00119911, false, "No errors reported\n");

	pause();
}
