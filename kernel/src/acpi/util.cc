#include <acpi/util.hh>
#include <cppcompat.hh>

// namespace acpi {
// 	extern Namespace ns;

// 	u64 process_pkglength(OPCODES*& code) {
// 		u8 leadbyte = (u8)*(code++);

// 		u32 bytes = (leadbyte >> 6);

// 		if (!bytes) {
// 			// Csak LeadByte van
// 			return leadbyte & 0b00111111;
// 		} else {
// 			u64 pkglength = leadbyte & 0b00001111;
// 			for (u32 i = 0; i < bytes; i++) {
// 				u32 byte = (u8)*(code++);
// 				// 4 bit alapból meg van a leadbyte-ból,
// 				pkglength |= byte << (4 + i * 8);
// 			}

// 			return pkglength;
// 		}
// 	}

// 	ObjectValue::ObjectValue(ScopeStack& scope, OPCODES*& code) {
// 		switch (*(code++)) {
// 			case OPCODES::BytePrefix: {
// 				emplace<Integer>(*(u8*)code, Integer::BYTE);
// 				code++;
// 				break;
// 			}
// 			case OPCODES::WordPrefix: {
// 				emplace<Integer>(*(u16*)code, Integer::WORD);
// 				code += 2;
// 				break;
// 			}
// 			case OPCODES::DWordPrefix: {
// 				emplace<Integer>(*(u32*)code, Integer::DWORD);
// 				code += 4;
// 				break;
// 			}
// 			case OPCODES::QWordPrefix: {
// 				emplace<Integer>(*(u64*)code, Integer::QWORD);
// 				code += 8;
// 				break;
// 			}
// 			case OPCODES::OneOp: {
// 				emplace<Integer>(0x01, Integer::BYTE);
// 				break;
// 			}
// 			case OPCODES::OnesOp: {
// 				emplace<Integer>(0xff, Integer::BYTE);
// 				break;
// 			}
// 			case OPCODES::ZeroOp: {
// 				emplace<Integer>(0x00, Integer::BYTE);
// 				break;
// 			}
// 			case OPCODES::StringPrefix: {
// 				u32 rstrchars = strlen((char*)code);
// 				emplace<String>((const char*)code, rstrchars);
// 				code += rstrchars + 1;
// 				break;
// 			}
// 			case OPCODES::PackageOp: {
// 				u64 numbytes = process_pkglength(code);
// 				u8 numelems = *(u8*)(code++);
// 				Package& pkg = emplace<Package>(numelems);
// 				// warn("Package: numelems: %d, bytes: %d", numelems, (u32)numbytes);

// 				for (u32 i = 0; i < numelems; i++)
// 					pkg.emplace(scope, code);
// 				break;
// 			}
// 			case OPCODES::BufferOp: {
// 				u32 length = process_pkglength(code);
// 				u64 buffersize = ObjectValue(scope, code).evalToInt();

// 				emplace<Buffer>((u8*)code, buffersize);
// 				code += buffersize;
// 				break;
// 			}
// 			default: {
// 				code--;
// 				if (ISSTRING(code)) {
// 					// Reference name(currentScope, code);
// 					// // printk("referenbce to %.4s", (char*)&name);
// 					// return DataObject(DataObject::REFERENCE, name);

// 					emplace<ObjectPath>(scope, code);
// 				} else {
// 					fatal(
// 						"Kezeletlen ObjectValue prefix: [%02x] %02x [%02x %02x] @ %lx",
// 						*(u8*)(code-1), *(u8*)code, *(u8*)(code+1), *(u8*)(code+2), code - tablestart
// 					);
// 				}
// 				break;
// 			}
// 		}
// 	}

// 	u64 ObjectValue::evalToInt() {
// 		switch (idx) {
// 			case IndexOf<Integer, OBJECT_VARIANT_TYPES>::value: {
// 				return get<Integer>().value;
// 			}
// 			case IndexOf<ObjectPath, OBJECT_VARIANT_TYPES>::value: {
// 				ObjectValue* sym = get<ObjectPath>().resolve();
// 				if (!sym)
// 					fatal("Unable to find name '%s'!", get<ObjectPath>().toString().c_str());
// 				return sym->evalToInt();
// 			}
// 			case IndexOf<FieldElem, OBJECT_VARIANT_TYPES>::value: {
// 				FieldElem& fe = get<FieldElem>();
// 				Field& f = fe.parent;

// 				if (f.type == Field::REGULAR) {
// 					u64 addr = f.opregion.base;
// 					assert(!(fe.offset & 7));
// 					addr += fe.offset;
// 					return addr;
// 				} else if (f.type == Field::INDEXED) {
// 					fatal("Indexed field to int?");
// 				} else if (f.type == Field::BANK) {
// 					fatal("Unsupported field type 'bank'!");
// 				} else {
// 					fatal("wgat");
// 				}
// 			}
// 			default: {
// 				fatal("evalToInt: not an integer (is instead %lld)", idx);
// 			}
// 		}
// 	}

// 	ObjectKey::ObjectKey() {}

// 	ObjectKey::ObjectKey(ScopeStack& scope, OPCODES*& code) {
// 		for (u32 i = 0; i < scope.size; i++)
// 			String::operator+=(scope[i].name.c_str());

// 		Name name(code);
// 		String::operator+=(name.c_str());
// 	}

// 	ObjectKey::ObjectKey(ScopeStack& scope, Name n) {
// 		for (u32 i = 0; i < scope.size; i++)
// 			String::operator+=(scope[i].name.c_str());

// 		String::operator+=(n.c_str());
// 	}

// 	ObjectKey::ObjectKey(MultiName& mn) {
// 		for (u32 i = 0; i < mn.names.size; i++) {
// 			String::operator+=(mn.names[i].c_str());
// 			String::operator+=('.');
// 		}
// 		data[--size] = 0;
// 		printk("key init from mn %s", data);
// 	}

// 	SuperName::SuperName(OPCODES*& code) {
// 		if (*code >= OPCODES::Local0Op && *code <= OPCODES::Arg6Op) {
// 			type = (Type)((u8)*code - (u8)OPCODES::Local0Op);
// 		} else {
// 			fatal("Unhandled AML SuperName prefix '%02x'", *(u8*)code);
// 		}
// 	}

// 	void MultiName::init(OPCODES*& code) {
// 		if (*code == OPCODES::DualNamePrefix) {
// 			code++;
// 			names.emplace(code);
// 			names.emplace(code);
// 		} else if (*code == OPCODES::MultiNamePrefix) {
// 			code++;
// 			u8 num = *(u8*)code;
// 			for (u32 j = 0; j < num; j++)
// 				names.emplace(code);
// 		} else {
// 			names.emplace(code);
// 		}
// 	}

// 	MultiName::MultiName(OPCODES*& code) { init(code); }

// 	ObjectPath::ObjectPath(ScopeStack& _scope, OPCODES*& code): scope(_scope) {
// 		if (*(u8*)code == '\\') {
// 			root = true;
// 		} else {
// 			while (*(u8*)code == '^') {
// 				parentPrefixes++;
// 				code++;
// 			}
// 		}

// 		path.init(code);
// 	}

// 	String ObjectPath::toString() {
// 		String ret;
// 		for (u32 i = 0; i < path.names.size; i++) {
// 			ret += path.names[i].c_str();
// 		}
// 		return ret;
// 	}

// 	ObjectValue* ObjectPath::resolve() {
// 		ScopeStack bak(scope);

// 		// fatal("resolve: %s [%s %d]", path.names[-1].c_str(), root ? "true" : "false", parentPrefixes);
// 		if (root || path.names.size > 1) {
// 			// printk("option 1");

// 			return ns[ObjectKey(path)];
// 		} else if (parentPrefixes) {
// 			// printk("option 2");

// 			assert(parentPrefixes < bak.size);

// 			for (u32 i = 0; i < parentPrefixes; i++)
// 				bak.pop();

// 			MultiName mn;
// 			for (auto& t : bak)
// 				mn.names.emplace(t.name);
// 			return ns[ObjectKey(mn)];
// 		} else {
// 			// printk("option 3");

// 			// Csak itt kell keresni
// 			// Ha nincs se rootchar se prefixchar, akkor a névnek egyedülállónak kell lennie
// 			assert(path.names.size == 1);

// 			// A közelebbi scope-okat kell keresni először
// 			u32 s = bak.size;
// 			for (u32 i = 0; i < s; i++, bak.pop()) {
// 				ObjectKey teszt(bak, path.names[0]);
// 				printk("searfch %lld for %s", bak.size, teszt.c_str());
// 				auto o = ns[teszt];
// 				if (o) return o;
// 			}
			
// 			// Ha még mindig nincs, akkor scope nélkül
// 			ObjectKey teszt(bak, path.names[0]);
// 			printk("searfch for %s", teszt.c_str());
// 			auto o = ns[teszt];
// 			if (o) return o;
// 		}

// 		return nullptr;
// 	}
// }
