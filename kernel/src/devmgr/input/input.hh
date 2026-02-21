#pragma once

#include <types.hh>

using char_t = u32;

enum struct ScanCode : u8 {
	Backspace = 0x08,
    Tab       = 0x09,
    Enter     = 0x0D,
    Escape    = 0x1B,
    Space     = 0x20,

    // Digits
    N0 = '0', N1 = '1', N2 = '2', N3 = '3', N4 = '4',
    N5 = '5', N6 = '6', N7 = '7', N8 = '8', N9 = '9',

    // Uppercase letters
    A = 'A', B = 'B', C = 'C', D = 'D', E = 'E', F = 'F',
    G = 'G', H = 'H', I = 'I', J = 'J', K = 'K', L = 'L',
    M = 'M', N = 'N', O = 'O', P = 'P', Q = 'Q', R = 'R',
    S = 'S', T = 'T', U = 'U', V = 'V', W = 'W', X = 'X',
    Y = 'Y', Z = 'Z',

    // Lowercase letters (optional if you want case distinction)
    a = 'a', b = 'b', c = 'c', d = 'd', e = 'e', f = 'f',
    g = 'g', h = 'h', i = 'i', j = 'j', k = 'k', l = 'l',
    m = 'm', n = 'n', o = 'o', p = 'p', q = 'q', r = 'r',
    s = 's', t = 't', u = 'u', v = 'v', w = 'w', x = 'x',
    y = 'y', z = 'z',

    // ASCII punctuation
    Exclamation = '!',
    Quote       = '"',
    Hash        = '#',
    Dollar      = '$',
    Percent     = '%',
    Ampersand   = '&',
    Apostrophe  = '\'',
    LeftParen   = '(',
    RightParen  = ')',
    Asterisk    = '*',
    Plus        = '+',
    Comma       = ',',
    Minus       = '-',
    Period      = '.',
    Slash       = '/',
    Colon       = ':',
    Semicolon   = ';',
    Less        = '<',
    Equal       = '=',
    Greater     = '>',
    Question    = '?',
    At          = '@',
    LeftBracket = '[',
    Backslash   = '\\',
    RightBracket= ']',
    Caret       = '^',
    Underscore  = '_',
    Grave       = '`',
    LeftBrace   = '{',
    Pipe        = '|',
    RightBrace  = '}',
    Tilde       = '~',

    // Special / non-ASCII keys (start above ASCII)
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

    Insert, DeleteKey, Home, End, PageUp, PageDown,
    ArrowUp, ArrowDown, ArrowLeft, ArrowRight,

    CapsLock, NumLock, ScrollLock,
    ShiftLeft, ShiftRight,
    CtrlLeft,  CtrlRight,
    AltLeft,   AltRight,
    MetaLeft,  MetaRight,  // Windows / Command key
    Menu,

    PrintScreen, Pause,
    Numpad0, Numpad1, Numpad2, Numpad3, Numpad4,
    Numpad5, Numpad6, Numpad7, Numpad8, Numpad9,
    NumpadAdd, NumpadSub, NumpadMul, NumpadDiv, NumpadEnter, NumpadDot,

	NUM_SCANCODES
};

extern u32 kbd_delay;
extern u32 kbd_repeat;

extern u64 kbd_lastpressed_time;

// 🇭🇺
char_t kbd_translate_hu(ScanCode c);

void input_init();
void kbd_onrepeat();
void kbd_press(ScanCode c);
void kbd_release(ScanCode c);
