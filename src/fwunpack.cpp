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

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "nds_types.h"
#include "encryption.h"
#include "get_data.h"
#include "get_encrypted_data.h"
#include "get_normal_data.h"
#include "lz77.h"
#include "part345_comp.h"

typedef struct {
	u16	part3_romaddr;
	u16	part4_romaddr;
	u16	part34_crc16;
	u16	part12_crc16;
	u8	fw_identifier[4];
	u16	part1_romaddr;
	u16	part1_ramaddr;
	u16	part2_romaddr;
	u16	part2_ramaddr;
	u16	shift_amounts;
	u16	part5_romaddr;

	u8	fw_timestamp[5];
	u8	console_type;
	u16	unused1;
	u16	user_settings_offset;
	u16	unknown1;
	u16	unknown2;
	u16	part5_crc16;
	u16	unused2;
} FW_HEADER;

#define FW_HEADER_SIZE 0x200

#define COMPRESSION_TYPE_LZ77 1

int decrypt_decompress (u8* src, u8* *dest) {
	GET_DATA get_data = get_encrypted_data;

	get_data.set_address (src);

	int compression_type = (get_data.get_u8() & 0xF0) >> 4;
	int decompressed_size = get_data.get_u8();
	decompressed_size |= get_data.get_u8() << 8;
	decompressed_size |= get_data.get_u8() << 16;

	*dest = (u8*) malloc (decompressed_size);

	switch (compression_type) {
		case COMPRESSION_TYPE_LZ77:
			Decompress_LZ77 (get_data, *dest, decompressed_size);
			break;
		default:
			printf ("CANNOT DECOMPRESS TYPE %d\n", compression_type);
			decompressed_size = 0;
			break;
	}

	return decompressed_size;
}

int decompress (u8* src, u8* *dest) {
	GET_DATA get_data = get_normal_data;

	get_data.set_address (src);

	int compression_type = (get_data.get_u8() & 0xF0) >> 4;
	int decompressed_size = get_data.get_u8();
	decompressed_size |= get_data.get_u8() << 8;
	decompressed_size |= get_data.get_u8() << 16;

	*dest = (u8*) malloc (decompressed_size);

	switch (compression_type) {
		case COMPRESSION_TYPE_LZ77:
			Decompress_LZ77 (get_data, *dest, decompressed_size);
			break;
		default:
			printf ("CANNOT DECOMPRESS TYPE %d\n", compression_type);
			decompressed_size = 0;
			break;
	}

	return decompressed_size;
}

int main(int argc, char* argv[])
{
	printf ("Nintendo DS Firmware Unpacker by Michael Chisholm (Chishm)\n");
	if (argc != 2) {
		printf ("Specifiy a firmware file.\n");
		return -1;
	}
	// Read the firmware file
	FILE* fw_bin = fopen (argv[1], "rb");
	if (!fw_bin) {
		printf ("Failed to open file.\n");
		return -1;
	}
	fseek (fw_bin, 0, SEEK_END);
	size_t fw_size = ftell (fw_bin);
	fseek (fw_bin, 0, SEEK_SET);
	u8* fw_data = (u8*)malloc(fw_size);
	fread (fw_data, 1, fw_size, fw_bin);
	fclose (fw_bin);
	printf ("Firmware size 0x%08X\n", fw_size);

	FW_HEADER* fw_header = (FW_HEADER*)fw_data;

	u32 arm9boot_romaddr = fw_header->part1_romaddr * (4 << ((fw_header->shift_amounts>>0) & 7));
	u32 arm9boot_ramaddr = 0x02800000 - fw_header->part1_ramaddr * (4 << ((fw_header->shift_amounts>>3) & 7));

	u32 arm7boot_romaddr = fw_header->part2_romaddr * (4 << ((fw_header->shift_amounts>>6) & 7));
	u32 arm7boot_ramaddr = (fw_header->shift_amounts & 0x1000 ? 0x02800000 : 0x03810000) - fw_header->part2_ramaddr * (4 << ((fw_header->shift_amounts>>9) & 7));

	u32 arm9gui_romaddr = fw_header->part3_romaddr * 8;
	u32 arm7gui_romaddr = fw_header->part4_romaddr * 8;

	u32 data_romaddr = fw_header->part5_romaddr * 8;

	printf ("ARM9 Boot: From 0x%08X to 0x%08X\n", arm9boot_romaddr, arm9boot_ramaddr);
	printf ("ARM7 Boot: From 0x%08X to 0x%08X\n", arm7boot_romaddr, arm7boot_ramaddr);

	printf ("GUI Data: From 0x%08X\n", data_romaddr);
	printf ("ARM9 GUI: From 0x%08X\n", arm9gui_romaddr);
	printf ("ARM7 GUI: From 0x%08X\n", arm7gui_romaddr);

	// Start unpacking
	init_keycode ( ((u32*)fw_data)[2] , 2, 0x0C); // idcode (usually "MACP"), level 2

	u8* decomp_data;
	int decomp_size;
	FILE* unpacked_bin;

	// Firmware header
	unpacked_bin = fopen ("header.bin", "wb");
	fwrite (fw_data, 1, FW_HEADER_SIZE, unpacked_bin);
	fclose (unpacked_bin);

	// ARM7 boot binary
	decomp_size  = decrypt_decompress (fw_data + arm7boot_romaddr, &decomp_data);
	unpacked_bin = fopen ("arm7boot.bin", "wb");
	fwrite (decomp_data, 1, decomp_size, unpacked_bin);
	fclose (unpacked_bin);
	free (decomp_data);

	// ARM9 boot binary
	decomp_size  = decrypt_decompress (fw_data + arm9boot_romaddr, &decomp_data);
	unpacked_bin = fopen ("arm9boot.bin", "wb");
	fwrite (decomp_data, 1, decomp_size, unpacked_bin);
	fclose (unpacked_bin);
	free (decomp_data);

	// ARM7 GUI binary
	decomp_size = part345_decompress (NULL, fw_data + arm7gui_romaddr);
	printf ("ARM7 GUI size: 0x%08X\n", decomp_size);
	decomp_data = (u8*) malloc (decomp_size);
	part345_decompress (decomp_data, fw_data + arm7gui_romaddr);
	unpacked_bin = fopen ("arm7gui.bin", "wb");
	fwrite (decomp_data, 1, decomp_size, unpacked_bin);
	fclose (unpacked_bin);
	free (decomp_data);

	// ARM9 GUI binary
	decomp_size = part345_decompress (NULL, fw_data + arm9gui_romaddr);
	printf ("ARM9 GUI size: 0x%08X\n", decomp_size);
	decomp_data = (u8*) malloc (decomp_size);
	part345_decompress (decomp_data, fw_data + arm9gui_romaddr);
	unpacked_bin = fopen ("arm9gui.bin", "wb");
	fwrite (decomp_data, 1, decomp_size, unpacked_bin);
	fclose (unpacked_bin);
	free (decomp_data);

	// GUI graphics binary
	decomp_size = part345_decompress (NULL, fw_data + data_romaddr);
	printf ("GUI Data size: 0x%08X\n", decomp_size);
	decomp_data = (u8*) malloc (decomp_size);
	part345_decompress (decomp_data, fw_data + data_romaddr);
	unpacked_bin = fopen ("gui_data.bin", "wb");
	fwrite (decomp_data, 1, decomp_size, unpacked_bin);
	fclose (unpacked_bin);
	free (decomp_data);

	if (fw_data[0x17C] != 0xFF) {
		printf ("Flashme firmware\n");
		fw_header = (FW_HEADER*)(fw_data + 0x3F680);
		arm9boot_romaddr = fw_header->part1_romaddr * (4 << ((fw_header->shift_amounts>>0) & 7));
		arm9boot_ramaddr = 0x02800000 - fw_header->part1_ramaddr * (4 << ((fw_header->shift_amounts>>3) & 7));

		arm7boot_romaddr = fw_header->part2_romaddr * (4 << ((fw_header->shift_amounts>>6) & 7));
		arm7boot_ramaddr = (fw_header->shift_amounts & 0x1000 ? 0x02800000 : 0x03810000) - fw_header->part2_ramaddr * (4 << ((fw_header->shift_amounts>>9) & 7));

		printf ("ARM9 Boot2: From 0x%08X to 0x%08X\n", arm9boot_romaddr, arm9boot_ramaddr);
		printf ("ARM7 Boot2: From 0x%08X to 0x%08X\n", arm7boot_romaddr, arm7boot_ramaddr);

		// ARM7 boot2 binary
		decomp_size  = decompress (fw_data + arm7boot_romaddr, &decomp_data);
		unpacked_bin = fopen ("arm7boot2.bin", "wb");
		fwrite (decomp_data, 1, decomp_size, unpacked_bin);
		fclose (unpacked_bin);
		free (decomp_data);

		// ARM9 boot2 binary
		decomp_size  = decompress (fw_data + arm9boot_romaddr, &decomp_data);
		unpacked_bin = fopen ("arm9boot2.bin", "wb");
		fwrite (decomp_data, 1, decomp_size, unpacked_bin);
		fclose (unpacked_bin);
		free (decomp_data);
	}


	printf ("Done\n");

	return 0;
}