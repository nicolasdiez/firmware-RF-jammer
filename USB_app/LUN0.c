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
 * ======== LUN0.c ========
 */
#include <string.h>
#include <stdint.h>
#include "USB_API/USB_Common/device.h"
#include "USB_config/descriptors.h"

//MSC #includes
#include "USB_API/USB_MSC_API/UsbMscScsi.h"
#include "USB_API/USB_MSC_API/UsbMsc.h"
#include "USB_API/USB_MSC_API/UsbMscStateMachine.h"

#include "LUN0.h"
#include "LUN0_data.h"


//The number of bytes per block.  In FAT, this is 512
const uint16_t BYTES_PER_BLOCK = 512;

//Data-exchange buffer between the API and the application.
//The application allocates it, and then registers it with the API.
//Later, the API will pass it back to the application when it
//needs the application to "process" it (exchange data with the media).
// In this case each cluster has one block so this buffer is 512 bytes.
uint8_t RWbuf[512];


//The API allocates an instance of structure type USBMSC_RWbuf_Info to hold all
//information describing buffers to be processed.  The structure instance is a
//shared resource between the API and application.
//During initialization, we'll call USBMSC_fetchInfoStruct() to obtain the
//pointer from the API.
USBMSC_RWbuf_Info *RWbuf_info;


//The application must tell the API about the media.  This information is
//conveyed in a call to USBMSC_updateMediaInfo(), passing an instance of
//USBMSC_RWbuf_Info.

struct USBMSC_mediaInfoStr mediaInfo;

/*
 * This function initializes the MSC data variables
 */
void LUN0_init (void)
{
    //The API maintains an instance of the USBMSC_RWbuf_Info structure.This is
    // a shared resource between the API and application; the application must
    //request the pointer.
    RWbuf_info = USBMSC_fetchInfoStruct();


    //The application must tell the API about the media.  Since the media isn't
    //removable, this is only called once, at the beginning of execution.
    //If the media were removable, the application must call this any time
    //the status of the media changes.
    mediaInfo.mediaPresent = 0x01; // internal flash is non-removable.
    mediaInfo.mediaChanged = 0x00; //It can't change, it's in internal memory.
    mediaInfo.writeProtected = 0x00; //It's not write-protected
    // 84 blocks in the volume. (This number is also found twice in the volume
    // itself; see mscFseData.c. They should match.)
    mediaInfo.lastBlockLba = 84;
    //512 bytes per block. (This number is also found in the
    //volume itself; see mscFseData.c. They should match.)
    mediaInfo.bytesPerBlock = BYTES_PER_BLOCK;
    USBMSC_updateMediaInfo(0, &mediaInfo);

    //The data interchange buffer (used when handling SCSI READ/WRITE) is
    //declared by the application, and registered with the API using this
    //function.  This allows it to be assigned dynamically, giving
    //the application more control over memory management.
    USBMSC_registerBufInfo(0, &RWbuf[0], NULL, 512);
}

void LUN0_processBuffer(void)
{
	//If the API needs the application to process a buffer, it
	//will keep the CPU awake by returning kUSBMSC_processBuffer
	//from USBMSC_poll().  The application should then check the
	//'operation' field of all defined USBMSC_RWbuf_Info
	//structure instances.  If any of them is non-null, then an
	//operation needs to be processed.  A value of
	//kUSBMSC_READ indicates the API is waiting for the
	//application to fetch data from the storage volume, in
	//response
	//to a SCSI READ command from the USB host.  After the
	//application does this, it must indicate whether the
	//operation succeeded, and then close the buffer operation
	//by calling USBMSC_bufferProcessed().
    while (RWbuf_info->operation == kUSBMSC_READ)
    {
        RWbuf_info->returnCode = LUN0_read(RWbuf_info->lba,
            RWbuf_info->bufferAddr,
            RWbuf_info->lbCount);  //Fetch a block from the medium
        USBMSC_bufferProcessed();  //Close the buffer operation
    }

	//Same as above, except for WRITE.  If operation ==
	//kUSBMSC_WRITE, then the API is waiting for us to
	//process the buffer by writing the contents to the storage
	//volume.
    while (RWbuf_info->operation == kUSBMSC_WRITE)
    {
        RWbuf_info->returnCode = LUN0_write(RWbuf_info->lba,
            RWbuf_info->bufferAddr,
            RWbuf_info->lbCount);  //Write the block to the medium
        USBMSC_bufferProcessed();  //Close the buffer operation
    }
}

/*
 * Writes a 512-byte block to flash.  Flash segments on the F550x, F552x, and
 * F563x/663x are 512 bytes in size -- same as our FAT blocks
 */
void flashWrite_LBA(uint8_t* flashAddr, uint8_t* data)
{
    uint16_t i;
    uint16_t bGIE;

    bGIE  = (__get_SR_register() & GIE);    //save interrupt status
    __disable_interrupt();

    //Erase the segment
    FCTL3 = FWKEY;                          //Clear the lock bit
    FCTL1 = FWKEY + ERASE;                  //Set the Erase bit
    *flashAddr = 0;                         //Dummy write, to erase the segment

    //Write the data to the segment
    FCTL1 = FWKEY + WRT;                    //Set WRT bit for write operation
    for (i = 0; i < 512; i++){
        *flashAddr++ = *data++;             //Write the block to flash
    }
    FCTL1 = FWKEY;                          //Clear WRT bit
    FCTL3 = FWKEY + LOCK;                   //Set LOCK bit

    __bis_SR_register(bGIE);                //restore interrupt status
}


/*
 * This function implements the "file system emulation" approach.  It reads a
 * block from the "storage medium", which in this case is internal flash.
 * It evaluates the block address (LBA) and uses memcpy() to exchange
 * "lbaCount" blocks.  The switch() statement can be thought of as a map to the
 * volume's block addresses.
 */
uint8_t LUN0_read(uint32_t LBA, uint8_t *buff,  uint8_t lbaCount)
{
    uint8_t i,ret = kUSBMSC_RWSuccess;
    for (i=0; i<lbaCount; i++)
    {
    	if (LBA > 84) {
    		 memset(buff, 0x00, BYTES_PER_BLOCK);
    		 ret = kUSBMSC_RWDevWriteFault;
    		 return (ret);
    	}
        switch(LBA)
        {
          // The Master Boot Record (MBR).  
          // This is always the first sector.  
          case 0:     
              memcpy(buff, MBR, BYTES_PER_BLOCK);
              break;
  
          // Partition block
          // The contents of the MBR had indicated that the partition block
          // start at 0x27.
          case 39:    // 0x27
              memcpy(buff, Partition, BYTES_PER_BLOCK);
              break;
  
          /*
           * File Allocation Tables
           * The FAT file system requires two copies of the FAT table.  In our
           * example, we actually store only one and report it in response to
           * the host requesting either.
           */
          case 43:	//First FAT
          case 44:	//Second FAT
              memcpy(buff, FAT, BYTES_PER_BLOCK);
              break;
  
          // Root directory
          // This area is actually 32 blocks long, but this example only stores
          // the first four;
          case 45:
              memcpy(buff, &Root_Dir[0], BYTES_PER_BLOCK);
              break;
          case 46:
              memcpy(buff, &Root_Dir[512], BYTES_PER_BLOCK);
              break;
          case 47:
              memcpy(buff, &Root_Dir[1024], BYTES_PER_BLOCK);
              break;
          case 48:
              memcpy(buff, &Root_Dir[1526], BYTES_PER_BLOCK);
              break;
          case 49:
              memcpy(buff, Data49, BYTES_PER_BLOCK);
              break;
          case 50:
              memcpy(buff, Data50, BYTES_PER_BLOCK);
              break;
          case 51:
              memcpy(buff, Data51, BYTES_PER_BLOCK);
              break;
          case 52:
              memcpy(buff, Data52, BYTES_PER_BLOCK);
              break;
          case 53:
              memcpy(buff, Data53, BYTES_PER_BLOCK);
              break;
          case 54:
              memcpy(buff, Data54, BYTES_PER_BLOCK);
              break;
          case 55:
              memcpy(buff, Data55, BYTES_PER_BLOCK);
              break;
          case 56:
              memcpy(buff, Data56, BYTES_PER_BLOCK);
              break;
          // For any other block address, return a blank block
          default:
              memset(buff, 0x00, BYTES_PER_BLOCK);
              //ret = kUSBMSC_RWLbaOutOfRange;
        }
        LBA++;
        buff += BYTES_PER_BLOCK;
    }
    return (ret);
}

/*
 * This function implements the "file system emulation" approach.
 * It writes a block to the "medium".  It supports writes to the first block of
 * the FAT table (which is sufficient for the number of limited numbers of files
 * we support); the first block of the root directory (same);
 * and blocks 49-56.  This is three data clusters, generally sufficient for
 * eight small files.
 */
uint8_t LUN0_write(uint32_t LBA, uint8_t *buff, uint8_t lbaCount)
{
    uint8_t i,ret = kUSBMSC_RWSuccess;
    for (i=0; i<lbaCount; i++)
    {    
    	if (LBA > 84) {
    		 ret = kUSBMSC_RWDevWriteFault;
    		 return (ret);
    	}
        switch(LBA)
        {
          case 43:   // First FAT               
          case 44:  // Second FAT
              flashWrite_LBA((uint8_t*)FAT,buff);
              break;
          case 45:  // Root directory
              flashWrite_LBA((uint8_t*)&Root_Dir[0],buff);
              break;
          case 46:  // Root directory
              flashWrite_LBA((uint8_t*)&Root_Dir[512],buff);
              break;
          case 47:  // Root directory
              flashWrite_LBA((uint8_t*)&Root_Dir[1024],buff);
              break;
          case 48:  // Root directory
              flashWrite_LBA((uint8_t*)&Root_Dir[1536],buff);
              break;
          case 49:  // Data block
              flashWrite_LBA((uint8_t*)Data49,buff);
              break;
          case 50:  // Data block
              flashWrite_LBA((uint8_t*)Data50,buff);
              break;
          case 51:  // Data block
              flashWrite_LBA((uint8_t*)Data51,buff);
              break;
          case 52:  // Data block
              flashWrite_LBA(Data52,buff);
              break;
          case 53:  // Data block
              flashWrite_LBA(Data53,buff);
              break;
          case 54:  // Data block
              flashWrite_LBA(Data54,buff);
              break;
          case 55:  // Data block
              flashWrite_LBA(Data55,buff);
              break;
          case 56:  // Data block
              flashWrite_LBA(Data56,buff);
              break;
        default:
              //ret = kUSBMSC_RWLbaOutOfRange;
              break;
        }
        LBA++;
        buff += BYTES_PER_BLOCK;
    }
    
    return ret;
}
//Released_Version_4_10_02
