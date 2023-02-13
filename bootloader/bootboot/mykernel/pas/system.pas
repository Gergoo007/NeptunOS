{*
 * mykernel/pas/system.pas
 *
 * Copyright (C) 2017 - 2021 bzt (bztsrc@gitlab)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This file is part of the BOOTBOOT Protocol package.
 * @brief Indirectly and unintuitively compiled by FPC along with kernel.pas
 *
 *}

Unit System;

{$define FPS_IS_SYSTEM}

Interface

{ Make FPC 3.2.0 happy by defining dummy types }

Const
    FPC_EXCEPTION = 0;

Type
    Cardinal = 0..$FFFFFFFF;
    Int8 = ShortInt;
    Int16 = SmallInt;
    Int32 = LongInt;
    UInt8 = Byte;
    UInt16 = Word;
    UInt32 = Cardinal;
    Uint64 = QWord;
    DWord = UInt32;
    PDWord = ^DWord;
    HRESULT = QWord;
    TTypeKind = QWord;
    Integer = Int32;
    PChar = ^Char;
    PByte = ^Byte;

    TExceptAddr = Record
    End;

    jmp_buf = Record
    End;

    PGuid = ^TGuid;
    TGuid = Record
        Data1 : DWord;
        Data2 : Word;
        Data3 : Word;
        Data4 : Array[0..7] of Byte;
    End;

    Ppsf = ^Tpsf;
    Tpsf = Packed Record
        magic : UInt32;
        version : UInt32;
        headersize : UInt32;
        flags : UInt32;
        numglyph : UInt32;
        bytesperglyph : UInt32;
        height : UInt32;
        width : UInt32;
    End;

{$I bootboot.inc}

Implementation
End.
