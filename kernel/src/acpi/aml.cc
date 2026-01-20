#include <acpi/aml.hh>
#include <acpi/acpi.hh>

constexpr bool isascii(AmlOpCode* code) { return *(u8*)code > 'A' && *(u8*)code < 'z'; }

u64 pkglength(AmlOpCode*& code) {
	u8 leadbyte = *(u8*)(code++);

	u32 bytes = (leadbyte >> 6);

	if (!bytes) {
		// Csak LeadByte van
		return leadbyte & 0b00111111;
	} else {
		u64 pkglength = leadbyte & 0b00001111;
		for (u32 i = 0; i < bytes; i++) {
			u32 byte = (u8)*(code++);
			// 4 bit alapból meg van a leadbyte-ból,
			pkglength |= byte << (4 + i * 8);
		}

		return pkglength;
	}
}

struct simplename {
	u32 name;
	u8 nullterm = 0;
	const char* const sname = (const char* const)&name;

	void operator=(const simplename& o) { name = o.name; }

	simplename(u32 n): name(n) {  }
	simplename(AmlOpCode*& yes) {
		assert(isascii(yes));
		assert(isascii(yes+1));
		assert(isascii(yes+2));
		assert(isascii(yes+3));

		name = *(u32*)yes;
		yes += 4;
	}
	simplename(): name(0) {  }
};

struct scopeelem {
	// 0: sima simplename
	// -1: root prefix
	// n: ennyi parent prefix volt a simplename előtt
	u16 prefix;
	simplename name;
	u32 length;

	scopeelem(AmlOpCode*& code) {
		length = pkglength(code);

		switch (*code) {
			case AmlOpCode::ParentPrefixChar:
				prefix++;
				code++;
				break;
			case AmlOpCode::RootChar:
				prefix = -1;
				code++;
				break;
			default:
				if (isascii(code)) {
					name = simplename(code);
					report("name here %s", name.sname);
				} else {
					fatal("Invalid AML scope element prefix: %02x", *(u8*)code);
				}
				break;
		}
	}
};

string scopetext(AmlOpCode*& code) {
	string ret;
	switch (*code) {
		case AmlOpCode::ParentPrefixChar:
		case AmlOpCode::RootChar:
			ret += *(char*)(code++);
			code++;
			break;
		default:
			if (isascii(code)) {
				simplename name(code);
				report("name here %s", name.sname);
			} else {
				fatal("Invalid AML scope element prefix: %02x", *(u8*)code);
			}
			break;
	}
	return ret;
}

void acpi_parse_aml(sdt_t* table) {
	return;

	AmlOpCode* op = (AmlOpCode*)(table + 1);
	AmlOpCode* end = (AmlOpCode*)table + table->length;
	vector<string> scope;

	while (op < end) {
		switch (*op) {
			case AmlOpCode::ScopeOp: {
				op++;
				u64 len = pkglength(op);
				auto s = scopetext(op);
				report("s is %s", s.c_str());
				break;
			}
			case AmlOpCode::ExtOpPrefix:
				op++;
				switch (*op) {
					case AmlOpCode::OpRegionOp: {
						
						break;
					}
					default:
						fatal("Unknown AML EXT opcode: %02x %02x <%02x> %02x %02x", *(op - 2), *(op - 1), *op, *(op + 1), *(op + 2));
				}
			default:
				fatal("Unknown AML opcode: %02x %02x <%02x> %02x %02x", *(op - 2), *(op - 1), *op, *(op + 1), *(op + 2));
		}
	}
}
