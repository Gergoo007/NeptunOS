#include <acpi/aml.hh>
#include <mm/vmm.hh>
#include <acpi/util.hh>
#include <util/storage.hh>

namespace acpi {
	NameStore names;
	Stack<u64> state;

	u64 process_pkglength(OPCODES*& code) {
		u8 leadbyte = (u8)*(code++);

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

	DataObject process_dataobject(OPCODES*& code) {
		DataObject ret {};
		// auto rtype = DataObject::BYTE;
		// u64 rintval = 0;
		// char* rstrval = nullptr;
		auto& rtype = ret.type;
		u64& rintval = ret.values.integer;
		const char*& rstrval = ret.values.string;

		// Az integerekből annyi van hoy inkább
		// hagyom elérni a function végét, a többi adattípust viszont azonnal
		// returnölöm

		switch (*(code++)) {
			case OPCODES::BytePrefix: {
				rtype = DataObject::BYTE;
				rintval = (u8)*code;
				code++;
				break;
			}
			case OPCODES::WordPrefix: {
				rtype = DataObject::WORD;
				rintval = *(u16*)code;
				code += 2;
				break;
			}
			case OPCODES::DWordPrefix: {
				rtype = DataObject::DWORD;
				rintval = *(u32*)code;
				code += 4;
				break;
			}
			case OPCODES::QWordPrefix: {
				rtype = DataObject::QWORD;
				rintval = *(u64*)code;
				code += 8;
				break;
			}
			case OPCODES::OneOp: {
				rtype = DataObject::BYTE;
				rintval = 0x01;
				break;
			}
			case OPCODES::OnesOp: {
				rtype = DataObject::BYTE;
				rintval = 0xff;
				break;
			}
			case OPCODES::ZeroOp: {
				rtype = DataObject::BYTE;
				rintval = 0x00;
				break;
			}
			case OPCODES::StringPrefix: {
				rtype = DataObject::STRING;
				u32 rstrchars = strlen((char*)code);
				rstrval = (char*)code;
				code += rstrchars + 1;

				return DataObject(DataObject::STRING, rstrval);
				break;
			}
			case OPCODES::PackageOp: {
				rtype = DataObject::PACKAGE;
				u64 numbytes = process_pkglength(code);
				u8 numelems = *(u8*)(code++);
				Package pkg(numelems);
				warn("Package: numelems: %d, bytes: %d\n", numelems, (u32)numbytes);

				for (u32 i = 0; i < numelems; i++)
					pkg[i] = process_dataobject(code);

				for (u32 i = 0; i < numelems; i++)
					error("data %d: %llu\n", i, pkg[i].values.integer);

				return DataObject(DataObject::PACKAGE, pkg);

				break;
			}
			default: {
				fatal("Kezeletlen data object: %02x\n", (u8)*(code-1));
				break;
			}
		}

		return DataObject(rtype, rintval);
	}

	void process_aml(OPCODES* code, u64 len) {
		OPCODES* end = code + len;

		// Mindegyik case kezdeténél a 'code' a switchelt opcode-ra mutat,
		// a 'code' növelése a case blokk felelőssége.
		while (code < end) {
			switch (*code) {
				case OPCODES::NameOp: {
					code++;
					u32 name = *(u32*)code;
					code += 4;
					DataObject data = process_dataobject(code);
					printk("nameop %.4s: %llx\n", (char*)&name, data.values.integer);
					names.insert(name, data);
					break;
				}
				case OPCODES::MethodOp: {
					OPCODES* start = code;

					code++;
					u64 length = process_pkglength(code);
					u32 name = *(u32*)code;
					code += 4;
					MethodFlags flags = *(MethodFlags*)code;
					code++;

					DataObject method(DataObject::METHOD, code, length);
					printk("Talaltam methodot: %.4s, %llu byte; %d argumentum\n", (char*)&name, length, flags.argcount);
					names.insert(name, method);

					code = start + length + 1;

					if (!memcmp((void*)(char*)&name, (void*)"PXXX", 4)) {
						printk("halo: %llx\n", names["PICM"].values.integer);
					}

					break;
				}
				default: {
					fatal("Kezeletlen AML opcode: [%02x] [%02x] %02x [%02x]\n", *(u8*)code-2, *(u8*)code-1, *(u8*)code, *(u8*)code+1);
					break;
				}
			}
		}
	}
}
