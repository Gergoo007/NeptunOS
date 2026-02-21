#include <util/storage.hh>
#include <util/smartptrs.hh>
#include <util/async.hh>
#include <util/variant.hh>
#include <mm/pmm4g.hh>
#include <arch/amd64/paging.hh>
#include <mm/vmm.hh>

void test_libk() {
	{
		void* p = kmalloc(128);
		memset(p, 0xff, 128);
		g_vmm.check(p);
		kfree(p);
	}

	{
		vector<u32> vec;
		vec.emplace_back(20);
		vec.emplace_back(30);
		vec.emplace_back(40);

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
		const char constexpr* TESZTSTR = "hello world";
		const char constexpr* TESZTSTR2 = "hello worldturiip ip";
		string teszt = TESZTSTR;
		assert(!strcmp(teszt.data, TESZTSTR));
		assert(teszt.size == strlen(TESZTSTR));

		teszt += 't';
		teszt += 'u';
		teszt += 'r';
		teszt += 'i';

		teszt += "ip ip";

		assert(!strcmp(teszt.data, TESZTSTR2));
		assert(string(TESZTSTR2, 11) == string(TESZTSTR));
	}

	{
		variant<u32, char*, string> asd;
		asd.emplace_back<string>("turi");
		assert(asd.get<string>() == string("turi"));
		asd.destroy();
		asd.emplace_back<u32>(10);
		assert(asd.get<u32>() == 10);
	}

	{
		unique_ptr<u32> p(10);
		assert(p.ptr);
		assert(*p == 10);
	}

	{
		void* p = kmalloc4g(10);
		assert((paging_lookup((u64)p) >> 32) == 0);
		kfree4g(p);

		p = kmalloc4g(10);
		assert((paging_lookup((u64)p) >> 32) == 0);
		kfree4g(p);

		p = kmalloc4g(10);
		assert((paging_lookup((u64)p) >> 32) == 0);
		kfree4g(p);

		p = kmalloc_aligned4g(16, 128);
		assert((paging_lookup((u64)p) >> 32) == 0);
		assert(((u64)p & 127) == 0);
		memset(p, 0xff, 10);
		kfree4g(p);
	}

	{
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
		hashmap<string, int> hm;
		assert(hm["helo"] == 0);
		hm["helo"] = 10;
		assert(hm["helo"] == 10);
		hm.remove("helo");
		assert(hm["helo"] == 0);

		assert(hm.has("helo"));
		assert(!hm.has("helo2"));
	}

	{
		static_assert(index_of<u32, u32, vector<u8>>::value == 0);
		static_assert(index_of<u8, u32, vector<u8>>::value == -1ull);
	}

	{
		// constexpr int arr1[] = { 1, 2, 3 };
		// array<3, int> arr2 { 4, 5, 6, 7 };
		// const vector<int> vec { 7, 8, 9, 10, 11 };

		// constexpr auto s = span(arr1);
		// assert(s.size == 3);
		// assert(s[2] == 3);

		// auto s2 = span(arr2);
		// assert(s2.size == 4);
		// assert(s2[2] == 6);
	}
}

void test() {
	u32 allocs = g_vmm.count_allocs();

	// test_mm();
	test_libk();

	if (g_vmm.count_allocs() != allocs)
		fatal("Mismatch between vmm link count: %d", allocs);
}
