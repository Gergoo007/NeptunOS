#include <acpi/aml.hh>
#include <mm/vmm.hh>
#include <acpi/util.hh>
#include <util/storage.hh>
#include <util/stacktrace.hh>

namespace acpi {
	Namespace ns;
	OPCODES* tablestart;
	// Lehet hogy vannak nested scope opcode-ok
	ScopeStack scope;

	// Ezeknek nincs nevük ezért vannak külön helyen a namespace-től
	Vector<Field> fields;

	// TODO: Watermark allocator-ral lehet jobban járunk mivel
	// az ACPI namespace élettartama megegyezik a kernel élettartamával

	// TODO: A scope-ok használjanak olyan stack-et ami a watermark allocatort használja

	// TODO: !!!! A NAME KEZDŐDHET ^-EL VAGY \-EL IS, EZEKET
	// NEM TUDJA KEZELNI A process_name() JELENLEG

	void process_fieldelement(OPCODES*& code, Field& field, u32& currentoffs) {
		switch (*(u8*)code) {
			case 0x00: {
				// ReservedField
				code++; // 0x00
				u32 asd = process_pkglength(code);
				currentoffs += asd;
				break;
			}
			case 0x01: {
				// AccessField
				code++;
				AccessType at = *(AccessType*)(code++);
				AccessAttrib aa = *(AccessAttrib*)(code++);
				fatal("accessfield\n");
				// printk("AccessField.. type: %01x attrib as: %01x; Attrib: %02x\n", at.accesstype, at.attribas, aa);
				break;
			}
			case 0x02: {
				// ConnectField
				fatal("connectfield");
				break;
			}
			default: {
				if (ISSTRING(code)) {
					// NamedField
					ObjectKey key(scope, code);
					u32 pkglen = process_pkglength(code);
					ns.emplace(key).object.emplace<FieldElem>(field, pkglen, currentoffs);
					currentoffs += pkglen;
				} else {
					error("Ismeretlen field [+%lx]: %02x %02x\n", code - tablestart, *((u8*)code), *((u8*)code+1));
				}
				break;
			}
		}
	}

	// // A null-t is kezeli
	// void process_supername(OPCODES*& code) {
	// 	switch (*code) {
	// 		case OPCODES::ZeroOp: {
	// 			printk("zero\n");
	// 			code++;
	// 			break;
	// 		}
	// 		case OPCODES::Arg0Op: {
	// 			printk("arg0\n");
	// 			code++;
	// 			break;
	// 		}
	// 		default: {
	// 			if (ISSTRING(code)) {
	// 				Reference name(currentScope, code);
	// 				printk("name: %.4s\n", name.c_str());
	// 			} else {
	// 				fatal("Ismeretlen supername prefix: %02x\n", *(u8*)code);
	// 			}
	// 			break;
	// 		}
	// 	}
	// }

	// bool process_condition(OPCODES*& code) {
	// 	switch (*code) {
	// 		case OPCODES::ExtOpPrefix: {
	// 			code++;
	// 			switch (*code) {
	// 				case OPCODES::CondRefOfOp: {
	// 					code++;
	// 					printk("condrefoif\n");
	// 					process_supername(code);
	// 					process_supername(code);
	// 					printk("code %02x\n", *code);
	// 					break;
	// 				}
	// 				default: {
	// 					fatal("Ismeretlen kondicio EXT opkod: 5b %02x [%02x]", *code, *(code+1));
	// 					break;
	// 				}
	// 			}
	// 			break;
	// 		}
	// 		default: {
	// 			fatal("Ismeretlen kondicio opkod: %02x [%02x]", *code, *(code+1));
	// 			break;
	// 		}
	// 	}

	// 	printk("namestore dump\n");
	// 	while (1);
	// 	for (u32 i = 0; i < names.size; i++) {
	// 		printk("%s: [%d] %16llx\n", names[i].key.c_str(), names[i].data.type, names[i].data.values.integer);
	// 	}

	// 	fatal("jaj\n");
	// }

	void nsdump() {
		printk("ns dump:\n");
		for (auto& n : ns) {
			printk("%s: type %lld\n", n.key.c_str(), n.object.idx);
		}
		hlt();
	}

	void process_aml(OPCODES* code, u64 len) {
		tablestart = code - 0x24;
		OPCODES* amlstart = code;
		OPCODES* end = code + len;

		// Mindegyik case kezdeténél a 'code' a switchelt opcode-ra mutat,
		// a 'code' növelése a case blokk felelőssége
		while (code < end) {
			// Érvényes még a jelenlegi scope?
			while (scope.size && code >= scope.last().end) {
				Name name = scope.pop().name;
				printk("exit scope %.4s\n", name.c_str());
			}

			switch (*code) {
				default: {
					fatal("ismeretlen opkod @ %lx %02x %02x\n", code - tablestart, *(u8*)code, *(u8*)(code+1));
					break;
				}
				case OPCODES::NameOp: {
					code++;
					ObjectKey key(scope, code);
					ns.emplace(key, ObjectValue(scope, code));
					break;
				}
				case OPCODES::MethodOp: {
					OPCODES* start = code;
					code++;
					u64 length = process_pkglength(code);
					Name n(code);
					printk("n of %s [%d %x bytes] @ %x\n", n.c_str(), (u32)length, (u32)length, (u32)(code - tablestart));
					MethodFlags flags = *(MethodFlags*)code;
					code++;

					ns.emplace(ObjectKey(scope, n)).object.emplace<Method>();
					// ns.insert(scope, code);

					code = start + length + 1;

					break;
				}
				case OPCODES::ScopeOp: {
					code++;
					auto start = code;
					u64 length = process_pkglength(code);

					auto turiend = start + length;
					MultiName mn(code);

					for (auto turi : mn.names) {
						error("enter scope %s\n", turi.c_str());
						scope.push(Scope { turi, turiend });
					}

					// warn("enter scope %.4s of %lld bytes (exit @ %p)\n", (char*)&name, length, (start + length));
					break;
				}
				case OPCODES::ExtOpPrefix: {
					code++;
					switch (*code) {
						case OPCODES::OpRegionOp: {
							code++;
							ObjectKey name(scope, code);
							// printk("opregion name: %.4s\n", (char*)&name);
							RegionSpace region = *(RegionSpace*)(code++);
							// printk("offset\n");
							ObjectValue regionoffset(scope, code);
							// printk("len\n");
							ObjectValue regionlength(scope, code);

							// printk("region: %d\n", region);
							// printk("region offset & len %llx; %llx\n", regionoffset.values.integer, regionlength.values.integer);
							OpRegion opregion;
							opregion.type = region;
							opregion.base = regionoffset.evalToInt();
							opregion.length = regionlength.evalToInt();
							ns.emplace(name).object.emplace<OpRegion>(opregion);
							break;
						}
						case OPCODES::FieldOp: {
							OPCODES* start = code;
							code++;
							u32 pkglen = process_pkglength(code);
							ObjectPath oprName(scope, code);
							FieldFlags flags = *(FieldFlags*)(code++);
							printk("oprname %s\n", oprName.toString().c_str());

							printk("FieldOp: %s; access: %d, lock: %d, update: %d\n", oprName.toString().c_str(), flags.accesstype, flags.lock, flags.updaterule);

							auto* ov = oprName.resolve();
							if (!ov)
								fatal("Nem talalhato a kovetkezo namespace elem: %s\n", oprName.toString().c_str());

							Field& field = fields.emplace(ov->get<OpRegion>());
							field.fags = flags;
							field.type = Field::REGULAR;

							u32 currentoffs = 0;
							while (code < (start + pkglen))
								process_fieldelement(code, field, currentoffs);

							break;
						}
						case OPCODES::IndexFieldOp: {
							OPCODES* start = code;
							code++;
							u32 pkglen = process_pkglength(code);
							ObjectPath indxReg(scope, code);
							ObjectPath dataReg(scope, code);
							FieldFlags flags = *(FieldFlags*)(code++);
							printk("indxreg %s\n", indxReg.toString().c_str());
							printk("datareg %s\n", dataReg.toString().c_str());

							printk("IndexFieldOp: %s %s; access: %d, lock: %d, update: %d\n", indxReg.toString().c_str(), dataReg.toString().c_str(), flags.accesstype, flags.lock, flags.updaterule);

							auto* ov1 = indxReg.resolve();
							if (!ov1)
								fatal("Nem talalhato a kovetkezo namespace elem: %s\n", indxReg.toString().c_str());
							FieldElem& e1 = ov1->get<FieldElem>();

							auto* ov2 = dataReg.resolve();
							if (!ov2)
								fatal("Nem talalhato a kovetkezo namespace elem: %s\n", dataReg.toString().c_str());
							FieldElem& e2 = ov2->get<FieldElem>();

							Field& field = fields.emplace();
							field.fags = flags;
							field.type = Field::INDEXED;

							u32 currentoffs = 0;
							while (code < (start + pkglen))
								process_fieldelement(code, field, currentoffs);

							break;
						}
						case OPCODES::DeviceOp: {
							code++;

							auto start = code;
							u32 length = process_pkglength(code);
							// TODO: NameString
							// u32 name = *(u32*)code;
							// warn("UJ ESZKOZ SCOPE: %.4s\n", (char*)code);
							Name name(code);
							scope.push(Scope { name, start + length });
							// warn("enter dev scope %.4s\n", (char*)&name);

							break;
						}
						case OPCODES::MutexOp: {
							code++;
							ObjectPath name(scope, code);
							SyncFlags fl = *(SyncFlags*)(code++);
							printk("mutex of name %.4s\n", name.toString().c_str());
							break;
						}
						default: {
							nsdump();
							fatal("Kezeletlen AML EXT opcode [+0x%lx]: [%02x] [%02x] %02x [%02x]\n", code - tablestart, *(u8*)(code-2), *(u8*)(code-1), *(u8*)code, *(u8*)(code+1));
							break;
						}
					}
					break;
				}
				case OPCODES::AliasOp: {
					code++;
					ObjectPath name1(scope, code);
					ObjectPath name2(scope, code);
					printk("alias %.4s to %.4s\n", name1.toString().c_str(), name2.toString().c_str());
					break;
				}
				// case OPCODES::CreateByteFieldOp:
				// case OPCODES::CreateWordFieldOp:
				// case OPCODES::CreateDWordFieldOp:
				// case OPCODES::CreateQWordFieldOp: {
				// 	u8 type;
				// 	switch (*code) {
				// 		case OPCODES::CreateByteFieldOp:
				// 			type = 0;
				// 			break;
				// 		case OPCODES::CreateWordFieldOp:
				// 			type = 1;
				// 			break;
				// 		case OPCODES::CreateDWordFieldOp:
				// 			type = 2;
				// 			break;
				// 		case OPCODES::CreateQWordFieldOp:
				// 			type = 3;
				// 			break;
				// 		default: break;
				// 	}

				// 	code++;
				// 	DataObject buffer = process_dataobject(code);
				// 	assert(buffer.type == DataObject::REFERENCE);

				// 	DataObject byteIdx = process_dataobject(code);
				// 	Reference name(currentScope, code);

				// 	DataObject field = DataObject(
				// 		DataObject::BUFFERFIELD,
				// 		BufferField(
				// 			buffer.values.ref, byteIdx.evalToInt(), (typeof(BufferField::access))type
				// 		)
				// 	);
				// 	names.insert(name, field);
				// 	break;
				// }
				// case OPCODES::IfOp: {
				// 	code++;

				// 	u32 pkglen = process_pkglength(code);
				// 	printk("ifop of %d bytes\n", pkglen);
				// 	bool cond = process_condition(code);
				// 	printk("next byte %02x\n", *code);

				// 	break;
				// }
				// default: {
				// 	fatal("Kezeletlen AML opcode [+0x%lx]: [%02x] [%02x] %02x [%02x]\n", code - tablestart, *(u8*)(code-2), *(u8*)(code-1), *(u8*)code, *(u8*)(code+1));
				// 	break;
				// }
			}
		}
	}
}
