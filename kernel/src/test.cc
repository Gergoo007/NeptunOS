#include <test.hh>

#include <util/storage.hh>
#include <util/smartptrs.hh>
#include <util/async.hh>
#include <mm/pmm4g.hh>
#include <arch/amd64/paging.hh>

struct Test {
	const char* name;

	Test(const char* name): name(name) {
		// printk("%s tests commencing...\n", name);
	}

	~Test() {
		// printk("%s test have finished\n", name);
	}
};

void test_libk() {
	{
		auto _ = Test("vmm");
		void* p = kmalloc(128);
		memset(p, 0xff, 128);
		vmm_check(p);
		kfree(p);
	}

	{
		auto _ = Test("libk->Vector");

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

		vector<u32> vec5 { 67, 61, 41 };
		vector<u32> vec6 = vec5;
		assert(vec5 == vec6);

		// Nullméretű vektor: nem használ memóriát, csak ha pusholnak bele
		vector<u32> vec7(0);
		assert(vec7.size == 0);
		assert(vec7.capacity == 0);
		assert(vec7.data == nullptr);
		vec7.push_back(10);
		assert(vec7.size == 1);
		assert(vec7.data != nullptr);

		array<4, u32> asd { 1, 2, 3, 4, };
		array<4, u32> asd2 = asd;
		assert(asd[0] == asd2[0]);
		assert(asd[1] == asd2[1]);
		assert(asd[2] == asd2[2]);
		assert(asd[3] == asd2[3]);
		assert(asd == asd2);
		assert(!(asd != asd2));
		assert(asd.size == 4 && asd2.size == 4);
	}

	{
		auto _ = Test("libk->String");

		const char constexpr* TESZTSTR = "hello world";
		string teszt = TESZTSTR;
		assert(!strcmp(teszt.data, TESZTSTR));
		assert(teszt.size == strlen(TESZTSTR));
	}

	{
		auto _ = Test("libk->Variant");
	}

	{
		auto _ = Test("unique_ptr");

		unique_ptr<u32> p(10);
		assert(p.ptr);
		assert(*p == 10);
	}

	{
		auto _ = Test("pmm4g");
		
		void* p = kmalloc4g(10);
		assert((paging_lookup((u64)p) >> 32) == 0);
		kfree4g(p);

		p = kmalloc_aligned4g(10, 128);
		assert((paging_lookup((u64)p) >> 32) == 0);
		assert(((u64)p & 127) == 0);
		kfree4g(p);
	}

	{
		auto _ = Test("llist");

		llist<int> dll;
		dll.push_back(10);
		dll.push_back(50);
		dll.push_back(30);
		dll.push_back(40);
		dll.push_front(88);

		assert(dll[0] == 88);
		assert(dll[1] == 10);
		assert(dll[2] == 50);
		assert(dll[3] == 30);
		assert(dll[4] == 40);

		dll.remove(0);
		dll.remove(3);
		dll.remove(1);
		
		assert(dll[0] == 10);
		assert(dll[1] == 30);
		assert(dll.size == 2);
	}

	{
		auto _ = Test("async");
		atomic<int> atom = 10;
		assert(atom == 10);
		assert(atom++ == 10);
		assert(++atom == 12);
		assert(atom-- == 12);
		assert(--atom == 10);
		assert(atom + 10 == 20);
		assert(atom - 10 == 0);
		atom += 10;
		assert(atom == 20);
		atom -= 10;
		assert(atom == 10);
	}

	{
		auto _ = Test("hashmap");

		// hashmap<string, int> hm;
		// hm["helo"] = 10;
		// assert(hm["helo"] == 10);
	}
}

void test() {
	u32 allocs = vmm_count_allocs();

	// test_mm();
	test_libk();

	if (vmm_count_allocs() != allocs)
		fatal("Mismatch between vmm link count: %d", allocs);
}
