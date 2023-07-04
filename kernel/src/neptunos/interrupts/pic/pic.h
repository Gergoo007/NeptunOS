#pragma once

#include <neptunos/serial/com.h>
#include <neptunos/interrupts/interrupts.h>
#include <neptunos/time/pit.h>

#define MPIC_CMD 0x20
#define MPIC_DATA 0x21
#define SPIC_CMD 0xa0
#define SPIC_DATA 0xa1
#define PIC_EOI 0x20

enum pic_cmds {
	PIC_CMD1_INIT = 0x10,
	PIC_CMD1_LVL = 0x08,
	PIC_CMD1_INTERV4 = 0x04,
	PIC_CMD1_NO_CASCADE = 0x02,
	PIC_CMD1_CMD4_PRESENT = 0x01,

	PIC_CMD4_8086 = 0x01,
};

void pic_init(void);
