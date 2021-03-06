/* --COPYRIGHT--,BSD
 * Copyright (c) 2014, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
/* 
 * ======== LUN0_data.c ========
 */

#include "USB_config/descriptors.h"

#ifdef _MSC_

// The Master Boot Record (MBR), which defines the partitions and
// their attributes.
// Notice it is stored in flash ("const"), and does not change.
#ifdef __IAR_SYSTEMS_ICC__  
#pragma location="MYDRIVE"
#endif
#ifdef __TI_COMPILER_VERSION__
#pragma DATA_SECTION ( MBR , "MYDRIVE"); 
#endif
const uint8_t MBR[512]=
{
  // The MBR contains three sections, with pre-determined fixed boundaries: 
  // a) Executable code (bytes 0-445)
  // b) The partition table (bytes 446-509).
  //	Entries begin at bytes 446, 462, 478, and 494.  (four max)
  // c) The boot signature.    
        
        // Executable code (blank)
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        
	// Partition table
	// Each entry is 16 bytes long; there's a table defining these 16 bytes
	// on p172 of Axelson.
    0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
    0x27, 0x00, 0x00, 0x00, 0x54, 0x00, 0x00, 0x00, // 446-461:First partition
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 462-477:Second partition (blank)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 478-493:Third partition (blank)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 494-509:Fourth partition (blank)
        
	// First partition data:
	// 0x00:  		Do not boot from this partition.
	// 0x000000:  	The partition's 1st sector address
	// 0x04:  		The partition type:  FAT16
	// 0x000000:  	The partition's last sector address
	// 0x27000000:  The partition's first LBA, expressed as an offset from the
    				//MBR sector (sector 0):  0x00000027
	// 0x54000000:  Total number of sectors in the partition:  84 sectors
        
    // Boot signature.  Fixed values.
	0x55, 0xAA
};

// First sector (block) of our one and only partition. This is the 'boot sector.
// Again, notice it is stored in flash ("const"), and does not change.
#ifdef __IAR_SYSTEMS_ICC__  
#pragma location="MYDRIVE"
#endif
#ifdef __TI_COMPILER_VERSION__
#pragma DATA_SECTION ( Partition , "MYDRIVE"); 
#endif
const uint8_t Partition[512] = {
  
	// First 62 bytes
	0xEB, 0x3C, 0x90,  // 
	0x4D, 0x53, 0x57, 0x49, 0x4E, 0x34, 0x2E, 0x31, // String that identifies
				  //the OS that formatted the media. Use "MSWIN4.1"
	0x00, 0x02,   // Number of bytes per sector (block).  0x200 = 512 bytes
	0x01,         // Number of sectors (blocks) per cluster.
	0x04, 0x00,   // Number of reserved sectors (four)
	0x02,         // File allocation tables (identical copies).Usually set to 2.
	0x40, 0x00,   // Max number of entries in the root directory.  64 entries
	0x08, 0x00,   // Total number of sectors, if <32K. Here its 8
	0xF8,         // Media descriptor.  0xF8 = non-removable, 0xF0 = removable.
				  // However, the host usually determines this using an
			      // INQUIRY command, not this field.
	0x01, 0x00,   // Number of sectors per FAT (file allocation table).
	0x00, 0x00,   // Number of sectors per track (not used in LBA system)
	0x00, 0x00,   // Number of heads (not used in LBA addressing system)
	0x27, 0x00, 0x00, 0x00,  // Number of hidden sectors.  0x00000027.
				  // (Compare this value to the address of this partition block,
			 	  // in Read_LBA().)
	0x08, 0x00, 0x00, 0x00,  // Total number of sectors, Here its 8
	0x80,         // Logical drive number of the partition.  OS-specific
	0x00,         // Reserved
	// Extended boot signature. Set to 0x29 if the next three fields are present
	0x29,
	  // Volume serial number.  Typically time/date the volume was formatted.
	0xE7, 0xC0, 0xD4, 0xA0,
	// Volume label, "NO NAME    ".  But most hosts ignore this, and use the
	// volume label in the root directory instead.
	0x4E, 0x4F, 0x20, 0x4E, 0x41, 0x4D, 0x45, 0x20, 0x20, 0x20, 0x20,
	// File system type.  Text.  "FAT16".  Most hosts generally ignore this.
	0x46, 0x41, 0x54, 0x31, 0x36, 0x20, 0x20, 0x20,

    // Boot code -- executable (ignore)
    0x33, 0xC9,
	0x8E, 0xD1, 0xBC, 0xF0, 0x7B, 0x8E, 0xD9, 0xB8,
	0x00, 0x20, 0x8E, 0xC0, 0xFC, 0xBD, 0x00, 0x7C,
	0x38, 0x4E, 0x24, 0x7D, 0x24, 0x8B, 0xC1, 0x99,
	0xE8, 0x3C, 0x01, 0x72, 0x1C, 0x83, 0xEB, 0x3A,
	0x66, 0xA1, 0x1C, 0x7C, 0x26, 0x66, 0x3B, 0x07,
	0x26, 0x8A, 0x57, 0xFC, 0x75, 0x06, 0x80, 0xCA,
	0x02, 0x88, 0x56, 0x02, 0x80, 0xC3, 0x10, 0x73,
	0xEB, 0x33, 0xC9, 0x8A, 0x46, 0x10, 0x98, 0xF7,
	0x66, 0x16, 0x03, 0x46, 0x1C, 0x13, 0x56, 0x1E,
	0x03, 0x46, 0x0E, 0x13, 0xD1, 0x8B, 0x76, 0x11,
	0x60, 0x89, 0x46, 0xFC, 0x89, 0x56, 0xFE, 0xB8,
	0x20, 0x00, 0xF7, 0xE6, 0x8B, 0x5E, 0x0B, 0x03,
	0xC3, 0x48, 0xF7, 0xF3, 0x01, 0x46, 0xFC, 0x11,
	0x4E, 0xFE, 0x61, 0xBF, 0x00, 0x00, 0xE8, 0xE6,
	0x00, 0x72, 0x39, 0x26, 0x38, 0x2D, 0x74, 0x17,
	0x60, 0xB1, 0x0B, 0xBE, 0xA1, 0x7D, 0xF3, 0xA6,
	0x61, 0x74, 0x32, 0x4E, 0x74, 0x09, 0x83, 0xC7,
	0x20, 0x3B, 0xFB, 0x72, 0xE6, 0xEB, 0xDC, 0xA0,
	0xFB, 0x7D, 0xB4, 0x7D, 0x8B, 0xF0, 0xAC, 0x98,
	0x40, 0x74, 0x0C, 0x48, 0x74, 0x13, 0xB4, 0x0E,
	0xBB, 0x07, 0x00, 0xCD, 0x10, 0xEB, 0xEF, 0xA0,
	0xFD, 0x7D, 0xEB, 0xE6, 0xA0, 0xFC, 0x7D, 0xEB,
	0xE1, 0xCD, 0x16, 0xCD, 0x19, 0x26, 0x8B, 0x55,
	0x1A, 0x52, 0xB0, 0x01, 0xBB, 0x00, 0x00, 0xE8,
	0x3B, 0x00, 0x72, 0xE8, 0x5B, 0x8A, 0x56, 0x24,
	0xBE, 0x0B, 0x7C, 0x8B, 0xFC, 0xC7, 0x46, 0xF0,
	0x3D, 0x7D, 0xC7, 0x46, 0xF4, 0x29, 0x7D, 0x8C,
	0xD9, 0x89, 0x4E, 0xF2, 0x89, 0x4E, 0xF6, 0xC6,
	0x06, 0x96, 0x7D, 0xCB, 0xEA, 0x03, 0x00, 0x00,
	0x20, 0x0F, 0xB6, 0xC8, 0x66, 0x8B, 0x46, 0xF8,
	0x66, 0x03, 0x46, 0x1C, 0x66, 0x8B, 0xD0, 0x66,
	0xC1, 0xEA, 0x10, 0xEB, 0x5E, 0x0F, 0xB6, 0xC8,
	0x4A, 0x4A, 0x8A, 0x46, 0x0D, 0x32, 0xE4, 0xF7,
	0xE2, 0x03, 0x46, 0xFC, 0x13, 0x56, 0xFE, 0xEB,
	0x4A, 0x52, 0x50, 0x06, 0x53, 0x6A, 0x01, 0x6A,
	0x10, 0x91, 0x8B, 0x46, 0x18, 0x96, 0x92, 0x33,
	0xD2, 0xF7, 0xF6, 0x91, 0xF7, 0xF6, 0x42, 0x87,
	0xCA, 0xF7, 0x76, 0x1A, 0x8A, 0xF2, 0x8A, 0xE8,
	0xC0, 0xCC, 0x02, 0x0A, 0xCC, 0xB8, 0x01, 0x02,
	0x80, 0x7E, 0x02, 0x0E, 0x75, 0x04, 0xB4, 0x42,
	0x8B, 0xF4, 0x8A, 0x56, 0x24, 0xCD, 0x13, 0x61,
	0x61, 0x72, 0x0B, 0x40, 0x75, 0x01, 0x42, 0x03,
	0x5E, 0x0B, 0x49, 0x75, 0x06, 0xF8, 0xC3, 0x41,
	0xBB, 0x00, 0x00, 0x60, 0x66, 0x6A, 0x00, 0xEB,
	0xB0, 0x42, 0x4F, 0x4F, 0x54, 0x4D, 0x47, 0x52,
	0x20, 0x20, 0x20, 0x20, 0x0D, 0x0A, 0x44, 0x61,
	0x74, 0x65, 0x6E, 0x74, 0x72, 0x84, 0x67, 0x65,
	0x72, 0x20, 0x65, 0x6E, 0x74, 0x66, 0x65, 0x72,
	0x6E, 0x65, 0x6E, 0xFF, 0x0D, 0x0A, 0x4D, 0x65,
	0x64, 0x69, 0x65, 0x6E, 0x66, 0x65, 0x68, 0x6C,
	0x65, 0x72, 0xFF, 0x0D, 0x0A, 0x4E, 0x65, 0x75,
	0x73, 0x74, 0x61, 0x72, 0x74, 0x3A, 0x20, 0x54,
	0x61, 0x73, 0x74, 0x65, 0x20, 0x64, 0x72, 0x81,
	0x63, 0x6B, 0x65, 0x6E, 0x0D, 0x0A, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xAC, 0xC4, 0xD3,
        
	// Boot signature.  Fixed values.
	0x55, 0xAA
};

/*
 * The File Allocation Table.  It has a single entry ("entry two"), indicating
 * that only one "cluster" in the volume is used.  There are actually supposed
 * to be two identical copies of this, but this example only stores one and
 * reports it in response to the host asking for either (see Read_LBA().
 * Again, notice it is stored in flash ("const"), and does not change.
 */
#ifdef __IAR_SYSTEMS_ICC__  
#pragma location="MYDRIVE"
#endif
#ifdef __TI_COMPILER_VERSION__
#pragma DATA_SECTION ( FAT , "MYDRIVE"); 
#endif
const uint8_t FAT[512]  = {
	0xF8, 0xFF,  // Entry zero:  Value 0xF8 should match "media descriptor"
				 // in the boot sector.  Other byte set to 0xFF.
	0xFF, 0xFF,  // Entry one:  Holds error codes.
	0xFF, 0xFF,  // Entry two:  Data cluster for the first file is this cluster.
				 // 0xFFFF indicates 'end of cluster chain', meaning the file
				 // stored here only requires this one cluster.
	0x00, 0x00,  // Entry three: Data cluster for second file
	0x00, 0x00,	 // Entry four: Data cluster for third file
	// Remaining free clusters are marked with 0x00
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// The remaining clusters are 0x01, which means they are reserved
	0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01
};

/*
 * The blocks below will be writable from the host.
 * They're defined at specific addresses to ensure they align with the MSP430's
 * lash segment boundaries.  Flash segments on the F550x, F551x/2x, and
 * F563x/663x are 512 bytes in length -- the same as a FAT block.
 * Flash must be erased a whole segment at a time, so it's important the
 * boundaries align.
 */

#ifdef __IAR_SYSTEMS_ICC__ 
// Data blocks
uint8_t* Data50 = (uint8_t*)0xE000;
uint8_t* Data51 = (uint8_t*)0xE200;
uint8_t* Data52 = (uint8_t*)0xE400;
uint8_t* Data53 = (uint8_t*)0xE600;
uint8_t* Data54 = (uint8_t*)0xE800;
uint8_t* Data55 = (uint8_t*)0xEA00;
uint8_t* Data56 = (uint8_t*)0xEC00;
#endif

#if defined(__TI_COMPILER_VERSION__)  || defined(__GNUC__)
//Data blocks
#pragma DATA_SECTION ( Data50Arr , "MYDRIVE_Data50");
uint8_t Data50Arr[512];
#pragma DATA_SECTION ( Data51Arr , "MYDRIVE_Data51");
uint8_t Data51Arr[512];
#pragma DATA_SECTION ( Data52Arr , "MYDRIVE_Data52");
uint8_t Data52Arr[512];
#pragma DATA_SECTION ( Data53Arr , "MYDRIVE_Data53");
uint8_t Data53Arr[512];
#pragma DATA_SECTION ( Data54Arr , "MYDRIVE_Data54");
uint8_t Data54Arr[512];
#pragma DATA_SECTION ( Data55Arr , "MYDRIVE_Data55");
uint8_t Data55Arr[512];
#pragma DATA_SECTION ( Data56Arr , "MYDRIVE_Data56");
uint8_t Data56Arr[512];

uint8_t* Data50 = &Data50Arr[0];
uint8_t* Data51 = &Data51Arr[0];
uint8_t* Data52 = &Data52Arr[0];
uint8_t* Data53 = &Data53Arr[0];
uint8_t* Data54 = &Data54Arr[0];
uint8_t* Data55 = &Data55Arr[0];
uint8_t* Data56 = &Data56Arr[0];
#endif

/* At compile-time for this demo, there will be three text files on the volume.
 * This is reflected in the root directory, and we also have initial contents
 * for the text file that needs to be stored in the data block for this file.
 * At the beginning of execution, the contents of these "_init" arrays will get
 * written to the corresponding volume blocks.  They're not stored very
 * efficiently, but in this demo we have plenty of flash available.
 */
#ifdef __IAR_SYSTEMS_ICC__  
#pragma location="MYDRIVE"
#endif
#ifdef __TI_COMPILER_VERSION__
#pragma DATA_SECTION ( Root_Dir , "MYDRIVE"); 
#endif
const uint8_t Root_Dir[2048] = {
  
  // The root directory typically holds 512 entries of 32 bytes each.  At 512 bytes per sector (aka "block"), this
  // requires 32 sectors.  In this example we actually create only one sector, and the other sectors are 0x00.  
  // We define one entry below (32 bytes).  
  
  // Note the 'file attributes' field below:
  //  Bit0 = "read only". (The host may still attempt to write to it however; so it's still important to return an error via USBMSC_bufferProcessed if it attempts to do so.)  
  //  Bit1 = "hidden". In a commercial product, this might be useful to hide the file from the user.  A host application to access the file instead.  
  //  Bit2 = "system".  May cause host to treat the file as 'critical'.  
  //  Bit3 = "volume".  Indicates volume label.  
  //  Bit4 = "directory".  Indicates the entry is for a directory, rather than a file.  
  //  Bit5 = "archive".  Used with backup utilities.  

    //First (and only) entry:
	// Short filename: The "." is implied. Only UPPER CASE is allowed.
	0x44, 0x41, 0x54, 0x41, 0x5F, 0x4C, 0x4F, 0x47, 0x54, 0x58, 0x54,
	0x00, // File attributes.  No special flags are set here.
	0x19, // Reserved
	0x44, // File creation time (in tenths of seconds)
	0x11, 0x03, // Creation time (hours/min/sec)
	0xFE, 0x37, // Creation date (year/month/day)
	0x01, 0x40, // Last accessed date (year/month/day)
	0x00, 0x00, // Higher two bytes of the first cluster's number
		        //(0x0000 for FAT16)
	0x45, 0xAA, // Last modified time (hours/min/sec)
	0x01, 0x40, // Last modified date (year/month/day)
	0x02, 0x00, // Lower two bytes of the cluster's number
	0x38, 0x00, 0x00, 0x00,  // File size

    //Blank
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};


#ifdef __IAR_SYSTEMS_ICC__
#pragma location="MYDRIVE"
#endif
#ifdef __TI_COMPILER_VERSION__
#pragma DATA_SECTION ( Data49 , "MYDRIVE");
#endif
const uint8_t Data49[] = {
    0x54, 0x49, 0x27, 0x73, 0x20, 0x4D, 0x61, 0x73, 0x73, 0x20, 0x53, 0x74,
    0x6F, 0x72, 0x61, 0x67, //TI's Mass Storag
    0x65, 0x20, 0x73, 0x61, 0x6D, 0x70, 0x6C, 0x65, 0x20, 0x61, 0x70, 0x70,
    0x6C, 0x69, 0x63, 0x61, //e sample applica
    0x74, 0x69, 0x6F, 0x6E, 0x2C, 0x20, 0x75, 0x73, 0x69, 0x6E, 0x67, 0x20,
    0x4D, 0x53, 0x50, 0x34, //tion, using MSP4
    0x33, 0x30, 0x2E, 0x20, 0x20, 0x0D, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, //30.
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20,
};




#endif  //_MSC_
/*------------------------ Nothing Below This Line --------------------------*/
//Released_Version_4_10_02
