#pragma once

#include <types.hh>
#include <util/storage.hh>

#define ISSTRING(a) ((*a >= OPCODES::NameCharStart && *a <= OPCODES::NameCharEnd) || \
						*a == OPCODES::RootChar || *a == OPCODES::ParentPrefixChar)

// namespace acpi {
// 	enum struct OPCODES : u8;

// 	pstruct MethodFlags {
// 		u8 argcount : 3;
// 		u8 serialized : 1;
// 		u8 synclevel : 4;
// 	};

// 	enum struct RegionSpace : u8 {
// 		SystemMemory,
// 		SystemIO,
// 		PCI_Config,
// 		EmbeddedControl,
// 		SMBus,
// 		SystemCMOS,
// 		PciBarTarget,
// 		IPMI,
// 		GeneralPurposeIO,
// 		GenericSerialBus,
// 		PCC,
// 	};

// 	pstruct FieldFlags {
// 		enum {
// 			AnyAcc,
// 			ByteAcc,
// 			WordAcc,
// 			DWordAcc,
// 			QWordAcc,
// 			BufferAcc,
// 			Reserved,
// 		} accesstype : 4;
// 		u8 lock : 1;
// 		enum {
// 			Preserve,
// 			WriteAsOnes,
// 			WriteAsZeroes,
// 		} updaterule : 2;
// 		u8 : 1;
// 	};

// 	pstruct AccessType {
// 		enum {
// 			AnyAcc,
// 			ByteAcc,
// 			WordAcc,
// 			DWordAcc,
// 			QWordAcc,
// 			BufferAcc,
// 			Reserved,
// 		} accesstype : 4;
// 		u8 : 2;
// 		enum {
// 			NormalAccessAttributes,
// 			AttribBytes,
// 			AttribRawBytes,
// 			AttribRawProcessBytes,
// 		} attribas : 2;
// 	};

// 	enum AccessAttrib : u8 {
// 		AttribQuick				= 0x02,
// 		AttribSendReceive		= 0x04,
// 		AttribByte				= 0x06,
// 		AttribWord				= 0x08,
// 		AttribBlock				= 0x0A,
// 		AttribProcessCall		= 0x0C,
// 		AttribBlockProcessCall	= 0x0D,
// 	};

// 	enum ExtendedAccessAttrib : u8 {
// 		AttribBytes				= 0x0B,
// 		AttribRawBytes			= 0x0E,
// 		AttribRawProcess		= 0x0F,
// 	};

// 	pstruct SyncFlags {
// 		u8 synclevel : 4;
// 		u8 : 4;
// 	};

// 	// A Name a zegyszerű, 4 karakteres neve az objektumnak,
// 	// az ObjectKey azonban tárolja a path-et is

// 	pstruct Name {
// 		u32 name = 0;
// 		u8 nullterm = 0;
// 		bool root = false;

// 		Name(u32 _name): name(_name) {  }
// 		Name(const char* _name): name(*(u32*)_name) {  }
// 		Name(OPCODES*& code) {
// 			if (*(u8*)code == '\\') {
// 				root = true;
// 				code++;
// 			}

// 			for (u32 i = 0; i < 4; i++, code++) {
// 				if (*(u8*)code == 0) { code++; break; }
// 				name |= (*(u8*)code) << (i * 8);
// 			}
// 		}
// 		const char* c_str() {
// 			return (char*)&name;
// 		}
// 	};

// 	struct MultiName {
// 		Vector<Name> names;
// 		MultiName(OPCODES*& code);
// 		MultiName() = default;
// 		void init(OPCODES*& code);
// 	};

// 	struct OpRegion {
// 		u64 base;
// 		u64 length;
// 		RegionSpace type;
// 	};

// 	struct Field {
// 		OpRegion opregion;
// 		FieldFlags fags;
// 		union {
// 			struct {
// 				u32 base;
// 			} regular;
// 			struct {
// 				u32 index;
// 				u32 value;
// 			} indexed;
// 			struct {

// 			} bank;
// 		};

// 		enum Type {
// 			REGULAR,
// 			INDEXED,
// 			BANK,
// 		} type;

// 		Field(OpRegion& opr): opregion(opr) {  }
// 		Field() {}
// 	};

// 	struct FieldElem {
// 		Field& parent;
// 		u32 offset;
// 		u32 width;

// 		FieldElem(Field& p, u32 w, u32 o): parent(p), offset(o), width(w) {  }
// 	};

// 	struct Buffer {
// 		u8* start;
// 		u64 size;
// 	};

// 	struct Integer {
// 		u64 value;
// 		enum {
// 			QWORD,
// 			DWORD,
// 			WORD,
// 			BYTE,
// 		} width;
// 	};

// 	struct Method {
		
// 	};

// 	struct Scope {
// 		Name name;
// 		OPCODES* end;
// 	};

// 	struct ObjectValue;
// 	using ScopeStack = Stack<Scope>;
// 	using Package = Vector<ObjectValue>;

// 	struct ObjectPath {
// 		ScopeStack scope;
// 		bool root = false;
// 		u32 parentPrefixes = 0;
// 		MultiName path;
// 		ObjectPath(ScopeStack& _scope, OPCODES*& code);
// 		ObjectValue* resolve();
// 		String toString();
// 	};

// 	#define OBJECT_VARIANT_TYPES Integer, Package, String, Method, OpRegion, FieldElem, ObjectPath, Buffer
// 	struct ObjectValue : Variant<OBJECT_VARIANT_TYPES> {
// 		ObjectValue(ScopeStack& scope, OPCODES*& code);
// 		u64 evalToInt();
// 		ObjectValue() = default;
// 	};
	
// 	struct SuperName {
// 		enum Type {
// 			Local0, Local1,
// 			Local2, Local3,
// 			Local4, Local5,
// 			Local6, Local7,

// 			Arg0, Arg1,
// 			Arg2, Arg3,
// 			Arg4, Arg5,
// 			Arg6, Arg7,

// 			Ref
// 		} type;
// 		Opt<ObjectPath> ref;

// 		SuperName(OPCODES*& code);
// 	};

// 	u64 process_pkglength(OPCODES*& code);

// 	// Az objektum abszolút elérési útja
// 	struct ObjectKey : String {
// 		ObjectKey();
// 		ObjectKey(ScopeStack& scope, OPCODES*& code);
// 		ObjectKey(ScopeStack& scope, Name n);
// 		ObjectKey(MultiName& mn);
// 	};

// 	// TODO: hashmap
// 	struct NamespaceEntry {
// 		ObjectKey key;
// 		ObjectValue object;
// 	};

// 	struct Namespace : Vector<NamespaceEntry> {
// 		void insert(ScopeStack& scope, OPCODES*& code) {
// 			ObjectKey key(scope, code);
// 			ObjectValue val(scope, code);
// 			emplace(key, val);
// 		}

// 		ObjectValue* operator[](const ObjectKey& key) {
// 			for (auto& e : *this) {
// 				if (e.key == key)
// 					return &e.object;
// 			}
// 			return nullptr;
// 			// fatal("Objektum nincs a namespace-ben: %s", key.c_str());
// 		}
// 	};
// }
