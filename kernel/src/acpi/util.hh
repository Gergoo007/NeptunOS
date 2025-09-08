#pragma once

#include <types.hh>
#include <util/storage.hh>

namespace acpi {
	enum struct OPCODES : u8;
	struct DataObject;

	pstruct MethodFlags {
		u8 argcount : 3;
		u8 serialized : 1;
		u8 synclevel : 4;
	};

	struct Package {
		u32 size = 0;
		DataObject* elems = nullptr;

		Package(u64 elems);
		Package() {}
		DataObject& operator[](u64 idx);
		~Package();
	};

	struct DataObject {
		enum Type : u8 {
			BYTE,
			WORD,
			DWORD,
			QWORD,
			STRING,
			METHOD,
			PACKAGE,
		} type;

		union ValueUnion {
			u64 integer;
			// String string;
			const char* string;
			struct {
				OPCODES* start;
				u64 len;
			} method;
			Package pkg;

			ValueUnion(u64 intval): integer(intval) {  }
			ValueUnion(const char* s): string(s) {  }
			ValueUnion(OPCODES* method, u64 len) {  }
			// TODO: ez kétszer copyzik
			ValueUnion(Package p): pkg(p) {  }
			ValueUnion() {  };
			~ValueUnion() {  }
		} values;

		DataObject(Type t, u64 intval): type(t), values(intval) {  }
		DataObject(Type t, const char* s): type(t), values(s) {  }
		DataObject(Type t, OPCODES* method, u64 len): type(t), values(method, len) {  }
		DataObject(Type t, Package pkg): type(t), values(pkg) {  }
		DataObject() {  }
		~DataObject() {  }
	};

	// TODO: legyen sorted list vagy hashmap, a storage.hh-ból
	struct NameStore {
		struct Entry {
			u32 name;
			DataObject data;
		};
		Entry* entries;
		u32 capacity = 128;
		u32 size = 0;

		NameStore();
		// void insert(u32 name, DataObject::Type type, u64 intval);
		// void insert(u32 name, const char* strval);
		void insert(u32 name, DataObject& data);
		DataObject& operator[](u32 idx);
		DataObject& operator[](const char* name);
		DataObject& byName(u32 name);
		void reserve(u32 cap);
		~NameStore();
	};
}
