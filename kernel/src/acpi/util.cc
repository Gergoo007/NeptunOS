#include <acpi/util.hh>
#include <cppcompat.hh>

namespace acpi {
	NameStore::NameStore() {
		entries = (Entry*)vmm::alloc(capacity * sizeof(Entry));
	}

	// void NameStore::insert(u32 name, DataObject::Type type, u64 intval) {
	// 	new (&entries[size++]) Entry { name, DataObject(type, intval) };
	// }

	// void NameStore::insert(u32 name, const char* value) {
	// 	new (&entries[size++]) Entry { name, DataObject(DataObject::STRING, value) };
	// 	// entries[size++] = entry(name, value);
	// }

	void NameStore::insert(u32 name, DataObject& data) {
		new (&entries[size++]) Entry { name, DataObject(data) };
		// entries[size++] = entry(name, value);
	}

	DataObject& NameStore::byName(u32 name) {
		for (u32 i = 0; i < size; i++)
			if (entries[i].name == name)
				return entries[i].data;
		fatal("No such name: %.4s\n", (char*)&name);
	}

	DataObject& NameStore::operator[](u32 idx) {
		return entries[idx].data;
	}

	DataObject& NameStore::operator[](const char* name) {
		return byName(*(u32*)name);
	}

	void NameStore::reserve(u32 cap) {
		capacity *= 4;
		entries = (Entry*)vmm::realloc(entries, capacity * sizeof(Entry));
	}

	NameStore::~NameStore() {
		vmm::free(entries);
	}

	Package::Package(u64 s): size(s) {
		elems = (DataObject*)vmm::alloc(size);
	}

	DataObject& Package::operator[](u64 idx) {
		#ifdef DEBUG
			if (idx >= size)
				fatal("Package index out of bounds! (%llu vs %llu)\n", idx, size);
		#endif
		return elems[idx];
	}

	Package::~Package() {
		vmm::free(elems);
		elems = (DataObject*)0x6767676767676768;
	}
}
