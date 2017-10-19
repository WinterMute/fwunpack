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

#ifndef GET_DATA_H
#define GET_DATA_H

#include "nds_types.h"

typedef u8  (* FN_GET_U8 ) (void) ;
typedef u16 (* FN_GET_U16) (void) ;
typedef u32 (* FN_GET_U32) (void) ;
typedef void (* FN_SET_ADDRESS) (u8* addr) ;

typedef struct {
	FN_GET_U8	get_u8;
	FN_GET_U16	get_u16;
	FN_GET_U32	get_u32;
	FN_SET_ADDRESS	set_address;
} GET_DATA;

#endif
