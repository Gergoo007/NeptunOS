#include <devmgr/input/input.hh>
#include <util/storage.hh>
#include <mm/vmm.hh>
#include <arch/arch.hh>

// Nem használok rendes 'string'-et, mert ez nem UTF-8 kompatibilis és
// nem lehet krolátozni a méretét
char* input_buffer = nullptr;

// A méret bájtokban értendő, nem karakterekben
u64 input_buffer_size = 0;
constexpr u64 INPUT_BUFFER_MAX_SIZE = 4096;

bitmap_t kbd_state;

// Milliszekundumokban
u32 kbd_delay = 175;
u32 kbd_repeat = 45;

ScanCode kbd_lastpressed_key;
u64 kbd_lastpressed_time;

static void append_char(ScanCode sc) {
	char_t c = kbd_translate_hu(sc);
	printk("%c", c);

	if (c >> 24)
		fatal("UTF-8 characters with more than 3 code points are not supported!");

	if (input_buffer_size == INPUT_BUFFER_MAX_SIZE - 1) {
		warn("Input buffer is full (%llu)!", input_buffer_size);
	} else {
		if (c & ~0x7f)
			fatal("UTF-8 scan codes are not supported yet!");
		input_buffer[input_buffer_size++] = c;
	}
}

void input_init() {
	input_buffer = (char*)wm_alloc(INPUT_BUFFER_MAX_SIZE * sizeof(char));
	kbd_state.init(wm_alloc((u32)ScanCode::NUM_SCANCODES / 8), (u32)ScanCode::NUM_SCANCODES);
}

void kbd_onrepeat() {
	if (kbd_state.get((u32)kbd_lastpressed_key))
		append_char(kbd_lastpressed_key);
}

void kbd_press(ScanCode c) {
	if (kbd_state.get((u32)c)) return;
	kbd_lastpressed_key = c;
	kbd_lastpressed_time = tmr_counter;
	kbd_state.set((u32)c, true);
	append_char(c);
}

void kbd_release(ScanCode c) {
	kbd_state.set((u32)c, false);
}

char_t kbd_translate_hu(ScanCode c) {
	switch (c) {
		case ScanCode::RightBracket: return u'ő';
		case ScanCode::LeftBracket: return u'ú';

		case ScanCode::Semicolon: return u'é';
		case ScanCode::Quote: return u'á';
		case ScanCode::Backslash: return u'ű';

		case ScanCode::Comma: return u',';
		case ScanCode::Period: return u'.';
		case ScanCode::Slash: return u'-';

		default: return (char_t)c;
	}
}

char_t kbd_translate_en(ScanCode c) { return (char_t)c; }
