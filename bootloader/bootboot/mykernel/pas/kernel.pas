{*
 * mykernel/pas/kernel.pas
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
 * @brief A sample BOOTBOOT compatible kernel
 *
 *}

Unit kernel;

Interface

{ imported virtual addresses, see linker script }
Var
    bootboot : Tbootboot; external;                 { see bootboot.inc }
    environment : Array[0..4095] of Char; external; { configuration, UTF-8 text key=value pairs }
    fb : Byte; external;                            { linear framebuffer mapped }
    _binary_font_psf_start : Tpsf; external;

Procedure _start(); stdcall;
Procedure Puts(s : PChar);

Implementation

{******************************************
 * Entry point, called by BOOTBOOT Loader *
 ******************************************}
Procedure _start(); stdcall; [public, alias: '_start'];
Var
    X : Integer;
    Y : Integer;
    S : Integer;
    W : Integer;
    H : Integer;
    P : PDword;
Begin
    {*** NOTE: this code runs on all cores in parallel ***}
    S := Integer(bootboot.fb_scanline);
    W := Integer(bootboot.fb_width);
    H := Integer(bootboot.fb_height);

    If (S > 0) Then
    Begin
        { cross-hair to see screen dimension detected correctly }
        For Y := 0 to (H-1) Do
        Begin
            P := PDword(@fb + S*Y + W*2);
            P^ := $00FFFFFF;
        End;
        For X := 0 to (W-1) Do
        Begin
            P := PDword(@fb + S*(H shr 1) + X*4);
            P^ := $00FFFFFF;
        End;

        { red, green, blue boxes in order }
        For Y := 0 to 20 Do
        Begin
            For X := 0 to 20 Do
            Begin
                P := PDword(@fb + S*(Y+20) + (X+20)*4);
                P^ := $00FF0000;
            End;
        End;
        For Y := 0 to 20 Do
        Begin
            For X := 0 to 20 Do
            Begin
                P := PDword(@fb + S*(Y+20) + (X+50)*4);
                P^ := $0000FF00;
            End;
        End;
        For Y := 0 to 20 Do
        Begin
            For X := 0 to 20 Do
            Begin
                P := PDword(@fb + S*(Y+20) + (X+80)*4);
                P^ := $000000FF;
            End;
        End;

        { say hello }
        Puts('Hello from a simple BOOTBOOT kernel');
    End;
    { hang for now }
    While (True) Do;
End;

{**************************
 * Display text on screen *
 **************************}
Procedure Puts(s : PChar);
Var
    X : Integer;
    Y : Integer;
    Kx : Integer;
    Line : Integer;
    Mask : Byte;
    Offs : Integer;
    Bpl : Integer;
    Glyph : PByte;
    P : PDword;
Begin
    Kx := 0;
    Bpl := (_binary_font_psf_start.width + 7) shr 3;
    While (s^ <> #0) Do
    Begin
        Glyph := PByte(@_binary_font_psf_start + _binary_font_psf_start.headersize +
            Integer(s^) * _binary_font_psf_start.bytesperglyph );
        Offs := (Kx * (_binary_font_psf_start.width+1) * 4);
        For Y := 0 to (_binary_font_psf_start.height-1) Do
        Begin
            Line := Offs;
            mask := 1 shl (_binary_font_psf_start.width-1);
            For X := 0 to (_binary_font_psf_start.width-1) Do
            Begin
                P := PDword(@fb + Line);
                If ((Glyph^ and Mask) <> 0) Then
                    P^ := $00FFFFFF
                Else
                    P^ := $00000000;
                Mask := Mask shr 1;
                Line := Line + 4;
            End;
            P := PDword(@fb + Line);
            P^ := $00000000;
            Glyph := Glyph + Bpl;
            Offs := Offs + Integer(bootboot.fb_scanline);
        End;
        Inc(Kx);
        Inc(s);
    End;
End;

End.
