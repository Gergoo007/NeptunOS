#include <usb/hci/ohci.h>

ohci_t* ohcis = NULL;
u64 num_ohcis;

void ohci_init_controller(pci_hdr_t* device) {
	if (!ohcis) ohcis = request_page();

	ohci_regs* regs = (ohci_regs*)((u64)device->type0.bar0 & ~((u32) 0b11));
	
	printk("OHCI rev: %02x\n", io32r(regs->revision));

	// Reset
	io32w(regs->cmd_sts, 1 << 0);
	u32 to = 30;
	while (io32r(regs->cmd_sts) & 1 && to) {
		sleep(1);
		to--;
	}

	if (to == 0) printk("OHCI reset timeout!\n");

	// HCCA beállítása
	ohci_hcca* hcca = pmm_alloc_page();
	ptr_32bit(hcca);
	io32w(regs->hcca, (u64)hcca);
}

void ohci_send_in(ohci_t* hc, usb_dev_t* dev, u8* packet, void* output, u8 len) {

}

void ohci_send_address(ohci_t* hc, usb_dev_t* dev, u8 addr) {

}
