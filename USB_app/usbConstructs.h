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
 * ======== usbConstructs.h ========
 */
uint8_t hidSendDataWaitTilDone (uint8_t* dataBuf,
    uint16_t size,
    uint8_t intfNum,
    uint32_t ulTimeout);
uint8_t hidSendDataInBackground (uint8_t* dataBuf,
    uint16_t size,
    uint8_t intfNum,
    uint32_t ulTimeout);
uint16_t hidReceiveDataInBuffer(uint8_t *,uint16_t,uint8_t);

uint8_t cdcSendDataWaitTilDone (uint8_t* dataBuf,
    uint16_t size,
    uint8_t intfNum,
    uint32_t ulTimeout);
uint8_t cdcSendDataInBackground (uint8_t* dataBuf,
    uint16_t size,
    uint8_t intfNum,
    uint32_t ulTimeout);
uint16_t cdcReceiveDataInBuffer(uint8_t *,uint16_t,uint8_t);

uint8_t phdcSendDataWaitTilDone (uint8_t* dataBuf,
    uint16_t size,
    uint8_t intfNum,
    uint32_t ulTimeout);
uint8_t phdcSendDataInBackground (uint8_t* dataBuf,
    uint16_t size,
    uint8_t intfNum,
    uint32_t ulTimeout);
uint16_t phdcReceiveDataInBuffer(uint8_t *,uint16_t,uint8_t);
//Released_Version_4_10_02
