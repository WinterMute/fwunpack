/*
fwunpack

	Nintendo DS Firmware Unpacker
    Copyright (C) 2007  Michael Chisholm (Chishm)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "get_normal_data.h"
#include "encryption.h"
#include <string.h>
#include <stdio.h>


#define DATA_BUFFER_SIZE 8

u8* normal_src_pointer;

u8 get_normal_u8 (void) {
	u8 output;
	output = normal_src_pointer[0];
	normal_src_pointer += 1;
	return output;
}

u16 get_normal_u16 (void) {
	u16 output;
	output = get_normal_u8();
	output |= (get_normal_u8() << 8);
	return output;
}

u32 get_normal_u32 (void) {
	u32 output;
	output = get_normal_u8();
	output |= (get_normal_u8() << 8);
	output |= (get_normal_u8() << 16);
	output |= (get_normal_u8() << 24);
	return output;
}

void set_normal_address (u8* addr) {
	normal_src_pointer = addr;
}

const GET_DATA get_normal_data = {
	(FN_GET_U8)			get_normal_u8,
	(FN_GET_U16)		get_normal_u16,
	(FN_GET_U32)		get_normal_u32,
	(FN_SET_ADDRESS)	set_normal_address
};

