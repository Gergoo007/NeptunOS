#pragma once

#include <types.hh>
#include <util/bitmap.hh>
#include <devmgr/usb/usb.hh>
#include <devmgr/devmgr.hh>

pstruct HCSPARAMS_t {
	u32 num_ports : 4;
	u32 port_pwr_ctl_supported : 1;
	u32 : 2;
	u32 port_routing_rules : 1;
	u32 ports_per_companion : 4;
	u32 num_companions : 4;
	u32 port_indicator_supported : 1;
	u32 : 3;
	u32 debug_port : 4;
	u32 : 8;
};

pstruct HCCPARAMS_t {
	u32 longaddress : 1; // 64 bit support
	u32 programmable_frame_list_size : 1; // Ha 0, 1024 elem van a frame list-ben, különben átírható, és be is kell állítani
	u32 async_schedule_park_supported : 1;
	u32 : 1;
	u32 iso_sched_threshold : 4;
	u32 extended_caps_ptr : 8; // 0: nincs ext. caps.; >0: pci config space-ben offset, NEM a BAR0-ban!
	u32 : 16;
};

// Operational Registers; a BAR0 + CAPLENGTH után találhatóak
pstruct USBCMD_t {
	u32 running : 1;
	u32 hcreset : 1;
	// 0: 1024 elem, alapérték
	// 1: 512 elem
	// 2: 256 elem
	// 3: Érvénytelen
	u32 frlist_size : 2;
	u32 periodic_schedule_enable : 1;
	u32 async_schedule_enable : 1;
	u32 int_on_async_advance : 1;
	u32 lighthcreset : 1; // nem biztos hogy van
	u32 async_sched_park_mode_count : 2; // Ha létezik ez a funkció, ennyi async tranzakciót hajt végre a vezérlő mielőtt a következő async queue-head-re haladna
	u32 : 1;
	u32 async_sched_park_mode_enable : 1;
	u32 : 4;
	u32 max_int_interval : 8; // Hány micro-frame-enként (125 us) küldhet a vezérlő megszakításokat
	u32 : 8;
};

pstruct USBSTS_t {
	u32 usbint : 1;
	u32 usberrint : 1;
	u32 portchange : 1;
	u32 : 9;
	u32 hchalted : 1;
	u32 reclamation : 1;
	u32 periodic_schedule : 1;
	u32 async_schedule : 1;
	u32 : 16;
};

pstruct USBINTR_t {
	u32 intr : 1;
	u32 errointr : 1;
	u32 portchange : 1;

	u32 : 29;
};

punion PORTSC_t {
	pstruct {
		u32 connect_sts : 1;
		u32 connect_sts_change : 1;
		u32 port_enabled : 1;
		u32 port_enabled_change : 1;
		u32 overcurrent : 1;
		u32 overcurrent_change : 1;
		u32 force_port_resume : 1;
		u32 suspend : 1;
		u32 reset : 1;
		u32 : 1;
		u32 line_sts : 2;
		u32 port_pwr : 1;
		u32 port_owner : 1; // Hogyha 1, akkor companion vezérlő kezeli a portot
		u32 port_indicator : 2;
		u32 port_test_ctl : 4;
		u32 wake_on_connect_enable : 1;
		u32 wake_on_disconnect_enable : 1;
		u32 wake_on_overcurrent_enable : 1;
		u32 : 9;
	};
	u32 _raw;
};

punion EhciRegUnion {
	HCSPARAMS_t HCSPARAMS;
	HCCPARAMS_t HCCPARAMS;

	USBCMD_t USBCMD;
	USBSTS_t USBSTS;
	USBINTR_t USBINTR;

	PORTSC_t PORTSC;

	u32 _raw;
};

pstruct extended_capability_hdr {
	u16 capability_id : 8;
	u16 next_capability_ptr : 8;
};

punion USBLEGSUP {
	pstruct {
		extended_capability_hdr hdr;
		u32 hc_bios_owned : 1;
		u32 : 7;
		u32 hc_os_owned : 1;
		u32 : 7;
	};
	u32 raw;

	constexpr USBLEGSUP(u32 raw): raw(raw) {  }
};

pstruct USBLEGCTLSTS {
	u32 : 32;
};

template<typename T> struct is_register		{ static constexpr bool value = false; };
template<> struct is_register<HCCPARAMS_t>	{ static constexpr bool value = true; };
template<> struct is_register<HCSPARAMS_t>	{ static constexpr bool value = true; };
template<> struct is_register<USBCMD_t>		{ static constexpr bool value = true; };
template<> struct is_register<USBSTS_t>		{ static constexpr bool value = true; };
template<> struct is_register<USBINTR_t>	{ static constexpr bool value = true; };
template<> struct is_register<PORTSC_t>	{ static constexpr bool value = true; };

template <typename T>
concept ehci_reg_concept = is_register<T>::value;

enum struct EhciQHType : u32 {
	iTD = 0b00,
	QH = 0b01,
	siTD = 0b10,
	FSTN = 0b11,
};

enum struct EhciEndpointSpeed : u32 {
	FS = 0b00,
	LS = 0b01,
	HS = 0b10,
};

enum struct EhciTAPerUframe : u32 {
	ONE_PER_UFRAME = 0b01,
	TWO_PER_UFRAME = 0b10,
	THREE_PER_UFRAME = 0b11,
};

enum struct EhciPid : u32 {
	OUT = 0b00,
	IN = 0b01,
	SETUP = 0b10,
};

pstruct ehci_qtd {
	punion {
		u32 t : 1;
		u32 ptr;
	} next_qtd;

	// Ha a jelenlegi IN qTD Short Packet-tel végződött, ez a qTD fog lefutni.
	punion {
		pstruct {
			u32 t : 1;
			u32 nak : 4;
		};
		u32 ptr;
	} alt_next_qtd;

	volatile pstruct {
		punion {
			pstruct {
				u32 ping : 1;
				u32 split : 1;
				u32 missed_uframe : 1;
				u32 transaction_err : 1;
				u32 babble : 1;
				u32 databuf_err : 1;
				u32 halted : 1;
				u32 active : 1;
			};
			u8 raw;
		} sts;
		EhciPid pid : 2;
		u32 err_counter : 2;
		u32 current_page : 3;
		u32 ioc : 1;
		u32 bytes : 15;
		u32 data : 1;
	} token;

	volatile u32 buffers[5];
};

pstruct ehci_qh {
	// DWORD 0
	volatile punion {
		pstruct {
			u32 t : 1;
			EhciQHType type : 2;
			u32 reserved : 2;
		};
		u32 ptr;
	} horiz_link;

	// DWORD 1
	pstruct {
		u32 addr : 7;
		// Csak akkor lehet érvényes, ha az eszköz nem HS és a QH nem async listben van
		u32 inactive_on_success : 1;
		u32 endpoint : 4;
		EhciEndpointSpeed endpoint_speed : 2;
		// Ha 1, akkor a bejövő qTD data toggle-je lesz az érvényes,
		// különben meghagyja a meglévőt
		u32 data_toggle_ctl : 1;
		u32 head_of_reclamation_list : 1;
		u32 mps : 11;
		// Ha nem HS és control endpoint az cél, be kell állítani,
		// különben 0 kell hogy legyen
		u32 ctl_endpoint : 1;
		u32 nak_reload_counter : 4;
	} endpoint_characteristics;

	// DWORD 2
	pstruct {
		// Async sched.-be nulla kell hogy legyen
		// Különben akkor megy végbe a tranzakció, ha int_sched_mask[(FRINDEX & 0b111)] == true
		u32 int_sched_mask : 8;
		// Csak akkor lényeges ha LS/FS és Periodic List-ben van
		u32 split_completion_mask : 8;
		// Ha a cél egy LS/FS eszköz egy HS hub-on, ez a hub címe, különben nem lényeges
		u32 hub_addr : 7;
		// Ha a cél egy LS/FS eszköz egy HS hub-on, ez a port a hub-on ahol az eszköz található, különben nem lényeges
		u32 port_num : 7;
		EhciTAPerUframe high_bw_pipe_multiplier : 2;
	} endpoint_caps;

	// DWORD 3
	// Ez a jelenlegi qTD, ahova visszaírja a vezérlő az overlayt a tranzakció konklúzióját követően.
	volatile u32 current_qtd;
	volatile ehci_qtd overlay;
};

void ehci_send(device_t& usbdev, u8 endp, usb_request* request, void* databuf);
void ehci_send_reset(device_t& usbdev);
u8 ehci_make_address(device_t& hc);

struct ehci_internal {
	usb_hci_interface_t ehci_module_interface {
		.usb_send = ehci_send,
		.usb_reset_port = ehci_send_reset,
		.usb_make_address = ehci_make_address,
	};
	u64 mmio;
	device_t* hc;
	bitmap_t addresses;
	u8 caplength;

	ehci_qh* head;
};
extern ehci_internal* context;

static_assert(offsetof(ehci_internal, ehci_module_interface) == 0);

pstruct ehci_reg {
	u32 offset;
	u32 length;
	bool opreg : 1;

	constexpr ehci_reg(u32 o, u32 l, bool op): offset(o), length(l), opreg(op) {  }
	constexpr ehci_reg(u32 portnum): offset(0x44 + portnum * 4), length(4), opreg(true) {  }

	[[nodiscard]]
	EhciRegUnion read() const {
		EhciRegUnion ret;

		u64 addr = context->mmio + offset;
		if (opreg)
			addr += context->caplength;

		if (length == 4)
			ret._raw = *(volatile u32*)addr;
		else if (length == 2)
			ret._raw = *(volatile u16*)addr;
		else if (length == 1)
			ret._raw = *(volatile u8*)addr;
		else
			fatal("Invalid EHCI register length %02x!", length);

		return ret;
	}

	void write(const EhciRegUnion& val) const {
		u64 addr = context->mmio + offset;
		if (opreg)
			addr += context->caplength;

		if (length == 4)
			*(volatile u32*)addr = val._raw;
		else if (length == 2)
			*(volatile u16*)addr = val._raw;
		else if (length == 1)
			*(volatile u8*)addr = val._raw;
		else
			fatal("Invalid EHCI register length %02x!", length);
	}

	void write(const u32 val) const {
		write(EhciRegUnion { ._raw = val });
	}

	void write(ehci_reg_concept auto reg) const {
		write(*(u32*)&reg);
	}
};

struct EhciRegs {
	static constexpr ehci_reg CAPLENGTH			= { 0x00, 1, false };
	static constexpr ehci_reg HCIVERSION		= { 0x02, 2, false };
	static constexpr ehci_reg HCSPARAMS			= { 0x04, 4, false };
	static constexpr ehci_reg HCCPARAMS			= { 0x08, 4, false };
	static constexpr ehci_reg HCSPPORTROUTE 	= { 0x0c, 8, false };

	static constexpr ehci_reg USBCMD			= { 0x00, 4, true };
	static constexpr ehci_reg USBSTS			= { 0x04, 4, true };
	static constexpr ehci_reg USBINTR			= { 0x08, 4, true };
	static constexpr ehci_reg FRINDEX			= { 0x0c, 4, true };
	static constexpr ehci_reg CTRL4GSEGMENT		= { 0x10, 4, true };
	static constexpr ehci_reg PERIDICLISTBASE	= { 0x14, 4, true };
	static constexpr ehci_reg ASYNCLISTBASE		= { 0x18, 4, true };

	// . = 0x40
	static constexpr ehci_reg CONFIGFLAG		= { 0x40, 4, true };
};
