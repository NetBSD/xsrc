/* Jong 03/12/2009; added for supporting Xorg 7.0 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>
#include "osdef.h"
#include "vgatypes.h"
/* #include "vb_util.h" */ /* Jong@08032009 */
#include "vb_def.h"

#ifdef WIN2000
#include <dderror.h>
#include <devioctl.h>
#include <miniport.h>
#include <ntddvdeo.h>
#include <video.h>
#include "xgiv.h"
#include "dd_i2c.h"
#include "tools.h"
#endif /* WIN2000 */

#ifdef LINUX_XF86
#include "xf86.h"
#include "xf86PciInfo.h"
#include "xgi.h"
#include "xgi_regs.h"
#include "vb_i2c.h"
#endif

#ifdef LINUX_KERNEL
#include <linux/version.h>
#include <asm/io.h>
#include <linux/types.h>
#include "vb_i2c.h"

/* Jong@08052009 */
/* #include <linux/delay.h> */ /* udelay */

#include "XGIfb.h"

#endif



/*char I2CAccessBuffer(PXGI_HW_DEVICE_INFO pHWDE, PI2CControl I2CCntl, ULONG DevAddr, ULONG Offset, PUCHAR pBuffer, ULONG ulSize); */
char vGetEDIDExtensionBlocks(PXGI_HW_DEVICE_INFO pHWDE, PI2CControl pI2C, PUCHAR pjBuffer, ULONG ulBufferSize);
char vGetEnhancedEDIDBlock(PXGI_HW_DEVICE_INFO pHWDE, PI2CControl pI2C, ULONG ulBlockID, ULONG ulBlockTag, PUCHAR pjBuffer, ULONG ulBufferSize);

char I2COpen (PXGI_HW_DEVICE_INFO  pHWDE,ULONG ulI2CEnable, ULONG ulChannelID, PI2CControl pI2CControl);
char I2CAccess(PXGI_HW_DEVICE_INFO pHWDE, PI2CControl pI2CControl);
BOOLEAN I2CNull( PXGI_HW_DEVICE_INFO pHWDE,  PI2CControl pI2CControl);
BOOLEAN I2CRead(PXGI_HW_DEVICE_INFO pHWDE, PI2CControl pI2CControl);
BOOLEAN I2CWrite(PXGI_HW_DEVICE_INFO pHWDE,  PI2CControl pI2CControl);
BOOLEAN ResetI2C(PXGI_HW_DEVICE_INFO pHWDE,  PI2CControl pI2CControl);
BOOLEAN I2CRead(PXGI_HW_DEVICE_INFO pHWDE,PI2CControl pI2CControl);
BOOLEAN I2CWrite(PXGI_HW_DEVICE_INFO pHWDE,  PI2CControl pI2CControl);
BOOLEAN ResetI2C(PXGI_HW_DEVICE_INFO pHWDE,  PI2CControl pI2CControl);
BOOLEAN Ack (PXGI_HW_DEVICE_INFO pHWDE,  bool fPut);
BOOLEAN NoAck(PXGI_HW_DEVICE_INFO pHWDE);
BOOLEAN Start( PXGI_HW_DEVICE_INFO pHWDE);
BOOLEAN Stop(PXGI_HW_DEVICE_INFO pHWDE);
BOOLEAN WriteUCHARI2C(PXGI_HW_DEVICE_INFO pHWDE,  UCHAR cData);
BOOLEAN ReadUCHARI2C(PXGI_HW_DEVICE_INFO pHWDE,  PUCHAR pBuffer);
UCHAR ReverseUCHAR(UCHAR data);

VOID vWriteClockLineDVI(PXGI_HW_DEVICE_INFO pHWDE, UCHAR data);
VOID vWriteDataLineDVI(PXGI_HW_DEVICE_INFO pHWDE, UCHAR data);
BOOLEAN bReadClockLineDVI(PXGI_HW_DEVICE_INFO pHWDE);
BOOLEAN bReadDataLineDVI(PXGI_HW_DEVICE_INFO pHWDE);

BOOLEAN bEDIDCheckSum(PUCHAR pjEDIDBuf,ULONG ulBufSize);

VOID vWriteClockLineCRT(PXGI_HW_DEVICE_INFO pHWDE, UCHAR data);
VOID vWriteDataLineCRT(PXGI_HW_DEVICE_INFO pHWDE, UCHAR data);
BOOLEAN bReadClockLineCRT(PXGI_HW_DEVICE_INFO pHWDE);
BOOLEAN bReadDataLineCRT(PXGI_HW_DEVICE_INFO pHWDE);

/* Jong@08102009 */
VOID vWriteClockLineFCNT(PXGI_HW_DEVICE_INFO pHWDE, UCHAR data);
VOID vWriteDataLineFCNT(PXGI_HW_DEVICE_INFO pHWDE, UCHAR data);
BOOLEAN bReadClockLineFCNT(PXGI_HW_DEVICE_INFO pHWDE);
BOOLEAN bReadDataLineFCNT(PXGI_HW_DEVICE_INFO pHWDE);

/* #define CRT_I2C */
VOID vWriteClockLine(PXGI_HW_DEVICE_INFO pHWDE, UCHAR data);
VOID vWriteDataLine(PXGI_HW_DEVICE_INFO pHWDE, UCHAR data);
BOOLEAN bReadClockLine(PXGI_HW_DEVICE_INFO pHWDE);
BOOLEAN bReadDataLine(PXGI_HW_DEVICE_INFO pHWDE);

/* Jong@08052009 */
extern UCHAR XGI_GetRegByte(XGIIOADDRESS port);

/* Jong@08052009 */
void I2C_DelayUS(ULONG MicroSeconds)
{
	ErrorF("");
	/* udelay(MicroSeconds); */
}


typedef enum _I2C_ACCESS_CMD
{
    I2C_WRITE = 0,
    I2C_READ 
/* Jong 08/18/2008; for XFree86 */
    /* WRITE = 0,
    READ */
} I2C_ACCESS_CMD;

/* For XG21 */

#define ENABLE_GPIOA          0x01
#define ENABLE_GPIOB          0x02
#define ENABLE_GPIOC          0x04
VOID
EnableGPIOA(
XGIIOADDRESS pjIOPort, I2C_ACCESS_CMD CmdType)
{
	PDEBUGI2C(ErrorF("EnableGPIOA()-pjIOPort=0x%x...\n", pjIOPort));

    UCHAR ujCR4A = XGI_GetReg(pjIOPort, IND_CR4A_GPIO_REG_III);

    if (CmdType == I2C_WRITE)
    {
        ujCR4A &= ~ENABLE_GPIOA;
    }
    else
    {
        ujCR4A |= ENABLE_GPIOA;
    }

    XGI_SetReg(pjIOPort, IND_CR4A_GPIO_REG_III, ujCR4A);
}

VOID
EnableGPIOB(
XGIIOADDRESS pjIOPort, I2C_ACCESS_CMD CmdType)
{
    UCHAR ujCR4A = XGI_GetReg(pjIOPort, IND_CR4A_GPIO_REG_III);

    if (CmdType == I2C_WRITE)
    {
        ujCR4A &= ~ENABLE_GPIOB;
    }
    else
    {
        ujCR4A |= ENABLE_GPIOB;
    }

    XGI_SetReg(pjIOPort, IND_CR4A_GPIO_REG_III, ujCR4A);
}

VOID
EnableGPIOC(
XGIIOADDRESS pjIOPort, I2C_ACCESS_CMD CmdType)
{
    UCHAR ujCR4A = XGI_GetReg(pjIOPort, IND_CR4A_GPIO_REG_III);

    if (CmdType == I2C_WRITE)
    {
        ujCR4A &= ~ENABLE_GPIOC;
    }
    else
    {
        ujCR4A |= ENABLE_GPIOC;
    }

    XGI_SetReg(pjIOPort, IND_CR4A_GPIO_REG_III, ujCR4A);
}




/**
*  Function: getGPIORWTranser()
*
*  Description: This function is used based on Z9. Because of wrongly wired deployment by HW
*               the CR4A and CR48 for GPIO pins have reverse sequence. For example,
*               D[7:0] for read function is ordered as GPIOA to GPIOH, but D[7:0] for read
*               is as GPIOH~GPIOA
*/
UCHAR
getGPIORWTransfer(
UCHAR ujDate)
{
    UCHAR  ujRet = 0;
    UCHAR  i = 0;

    for (i=0; i<8; i++)
	{
    	ujRet = ujRet << 1;
		ujRet |= GETBITS(ujDate >> i, 0:0);
    }

	return ujRet;
}



char I2CAccessBuffer(PXGI_HW_DEVICE_INFO pHWDE, PI2CControl pI2CControl, ULONG ulDevAddr,
    ULONG ulOffset, PUCHAR pBuffer,ULONG ulSize)
{

    I2CControl  I2C;
    ULONG       i;

    if ((ulSize == 0) || (pBuffer == NULL))  {
        return -1;
    }
    if (ulDevAddr & 1)  {
        return -1;
    }
    if ((ulDevAddr > 0xFF) || (ulOffset > 0xFF))  {
        return -1;
    }

    I2C.Command = pI2CControl->Command;
    I2C.dwCookie = pI2CControl->dwCookie;
    I2C.Data = pI2CControl->Data;
    I2C. Flags = pI2CControl->Flags;
    I2C.Status = pI2CControl->Status;
    I2C.ClockRate = pI2CControl->ClockRate;
    switch (pI2CControl->Command)  {
    case I2C_COMMAND_READ:
        /* Reset I2C Bus */
        I2C.Command = I2C_COMMAND_RESET;
        I2CAccess(pHWDE, &I2C);
        if (I2C.Status != I2C_STATUS_NOERROR)  {
            pI2CControl->Status = I2C.Status;
            break;
        }

        /* Write Device Address */
        I2C.Command = I2C_COMMAND_WRITE;
        I2C.Flags = I2C_FLAGS_START | I2C_FLAGS_ACK;
        I2C.Data = (UCHAR)ulDevAddr;
        I2CAccess(pHWDE, &I2C);
        if (I2C.Status != I2C_STATUS_NOERROR)  {
            pI2CControl->Status = I2C.Status;
            break;
        }

        /* Write Register Offset */
        I2C.Command = I2C_COMMAND_WRITE;
        I2C.Flags = I2C_FLAGS_ACK | I2C_FLAGS_STOP;
        I2C.Data = (UCHAR)ulOffset;
        I2CAccess(pHWDE, &I2C);
        if (I2C.Status != I2C_STATUS_NOERROR)  {
            pI2CControl->Status = I2C.Status;
            break;
        }

        /* Write Device Read Address */
        I2C.Command = I2C_COMMAND_WRITE;
        I2C.Flags = I2C_FLAGS_START | I2C_FLAGS_ACK;
        I2C.Data = (UCHAR)ulDevAddr + 1;
        I2CAccess(pHWDE, &I2C);
        if (I2C.Status != I2C_STATUS_NOERROR)  {
            pI2CControl->Status = I2C.Status;
            break;
        }

        /* Read Data */
        for (i=0; i< ulSize; i++)  {
            I2C.Command = I2C_COMMAND_READ;
            I2C.Flags = I2C_FLAGS_ACK;
            if (i == ulSize - 1)  {     /* Read Last UCHAR */
                I2C.Flags |= I2C_FLAGS_STOP;
            }
            I2CAccess(pHWDE, &I2C);
            if (I2C.Status != I2C_STATUS_NOERROR)  {
                pI2CControl->Status = I2C.Status;
                break;
            }
            *pBuffer = I2C.Data;
            pBuffer++;
        }
        pI2CControl->Status = I2C.Status;
        break;

    case I2C_COMMAND_WRITE:
        /* Reset I2C Bus */
        I2C.Command = I2C_COMMAND_RESET;
        I2CAccess(pHWDE, &I2C);
        if (I2C.Status != I2C_STATUS_NOERROR)  {
            pI2CControl->Status = I2C.Status;
            break;
        }

        /* Write Device Address */
        I2C.Command = I2C_COMMAND_WRITE;
        I2C.Flags = I2C_FLAGS_START | I2C_FLAGS_ACK;
        I2C.Data = (UCHAR)ulDevAddr;
        I2CAccess(pHWDE, &I2C);
        if (I2C.Status != I2C_STATUS_NOERROR)  {
            pI2CControl->Status = I2C.Status;
            break;
        }

        /* Write Register Offset */
        I2C.Command = I2C_COMMAND_WRITE;
        I2C.Flags = I2C_FLAGS_ACK;
        I2C.Data = (UCHAR)ulOffset;
        I2CAccess(pHWDE, &I2C);
        if (I2C.Status != I2C_STATUS_NOERROR)  {
            pI2CControl->Status = I2C.Status;
            break;
        }

        /* Write Data */
        for (i=0; i< ulSize; i++)  {
            I2C.Command = I2C_COMMAND_WRITE;
            I2C.Flags = I2C_FLAGS_ACK;
            if (i == ulSize - 1)  {     /* Read Last UCHAR */
                I2C.Flags |= I2C_FLAGS_STOP;
            }
            I2C.Data = *pBuffer;
            I2CAccess(pHWDE, &I2C);
            if (I2C.Status != I2C_STATUS_NOERROR)  {
                pI2CControl->Status = I2C.Status;
                break;
            }
            pBuffer++;
        }
        pI2CControl->Status = I2C.Status;
        break;
    }

    if (pI2CControl->Status == I2C_STATUS_NOERROR)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}


/*************************************************************************
// char vGetEDIDExtensionBlocks
//
// Routine Description:
//      Reads the extension part behind a 128KB block of EDID which is in 1.3
//      format. The extension part can be distributed into 128KB-blocks.
//
// Arguments:
//      pHWDE           -   Hardware extension object pointer
//      pI2C            -   I2C pointer
//      pjBuffer        -   EDID Buffer
//      ulBufferSize    -   Buffer size
//
//
// Return Value:
//      Status of returning EDID.
//      NO_ERROR                -   success
//      ERROR_INVALID_PARAMETER -   failed
//
****************************************************************************/
char vGetEDIDExtensionBlocks(
    PXGI_HW_DEVICE_INFO  pHWDE,
    PI2CControl pI2C,
    PUCHAR      pjBuffer,
    ULONG       ulBufferSize)
{
    char   status;
    ULONG       ulBlockTag;
    ULONG       i;
    PUCHAR      pBlockMap;


    if ((ulBufferSize < 128) || (pjBuffer == NULL))
    {
        return -1;
    }

    pI2C->Command = I2C_COMMAND_READ;
    status = I2CAccessBuffer(pHWDE, pI2C, 0xA0, 128, pjBuffer, 128);
    if ((status != 0) || (pI2C->Status != I2C_STATUS_NOERROR))
    {
        return status;
    }

    if (bEDIDCheckSum(pjBuffer, 128) != 0)  {
        return -1;
    }

    if (*pjBuffer == 0xF0)
    {   /* A Block Map, we should read other extension blocks*/
        pBlockMap = pjBuffer;
        for (i=1; i<=126; i++)
        {
            ulBlockTag = *(pBlockMap + i);
            if (ulBlockTag)
            {
                pjBuffer += 128;
                ulBufferSize -= 128;
                status = vGetEnhancedEDIDBlock(pHWDE, pI2C, i+1, ulBlockTag, pjBuffer, ulBufferSize);
                if ((status != 0) || (pI2C->Status != I2C_STATUS_NOERROR))
                {
                    return -1;
                }
            }
            else
            {
                if (i > 1)
                {
                    return 0;  /* All Extension blocks must be sequential, no holes allowed. (VESA E-EDID)*/
                }
                else
                {
                    return -1;
                }
            }
        }
        /* We should read block # 128 */
        pjBuffer += 128;
        ulBufferSize -= 128;
        ulBlockTag = 0xF0;
        status = vGetEnhancedEDIDBlock(pHWDE, pI2C, 128, ulBlockTag, pjBuffer, ulBufferSize);
        if ((status != 0) || (pI2C->Status != I2C_STATUS_NOERROR))
        {
            return 0;    /* Monitor may has only 128 blocks (0~127) */
        }

        pBlockMap = pjBuffer;
        for (i=1; i<=126; i++)
        {   /* Read Block # 128 ~ 254 */
            ulBlockTag = *(pBlockMap + i);
            if (ulBlockTag)
            {
                pjBuffer += 128;
                ulBufferSize -= 128;
                status = vGetEnhancedEDIDBlock(pHWDE, pI2C, i+128, ulBlockTag, pjBuffer, ulBufferSize);
                if ((status != 0) || (pI2C->Status != I2C_STATUS_NOERROR))
                {
                    return -1;
                }
            }
            else
            {
                if (i > 1)
                {
                    return 0;  /* All Extension blocks must be sequential, no holes allowed. (VESA E-EDID) */
                }
                else
                {
                    return -1;
                }
            }
        }
    }

    return 0;
}

/*************************************************************************
// char vGetEnhancedEDIDBlock
//
// Routine Description:
//  Get the EDID which is in Enhanced-EDID format via I2C. The parse-in block
//  tag(ulBlockTag) and the first UCHAR of the retrieved buffer should be identical.
//  Returns error when they are not, otherwise return NO_ERROR.
//
// Arguments:
//      pHWDE           -   Hardware extension object pointer
//      pI2C            -   I2C pointer
//      ulBlockID       -   Block ID
//      ulBlockTag      -   Block Tag
//      pjBuffer        -   EDID Buffer
//      ulBufferSize    -   Buffer size
//
//
// Return Value:
//      Status of returning EDID.
//      NO_ERROR                -   success
//      ERROR_INVALID_PARAMETER -   failed
//
****************************************************************************/
char vGetEnhancedEDIDBlock(
    PXGI_HW_DEVICE_INFO  pHWDE,
    PI2CControl pI2C,
    ULONG       ulBlockID,
    ULONG       ulBlockTag,
    PUCHAR      pjBuffer,
    ULONG       ulBufferSize)
{
    ULONG       ulOffset, SegmentID;
    char   status;

    if ((ulBufferSize < 128) || (pjBuffer == NULL))
    {
        return -1;
    }

    SegmentID = ulBlockID / 2;
    ulOffset = (ulBlockID % 2) * 128;

    pI2C->Command = I2C_COMMAND_WRITE;
    status = I2CAccessBuffer(pHWDE, pI2C, 0x60, 0, (PUCHAR)&SegmentID, 1);
    if ((status == NO_ERROR) && (pI2C->Status == I2C_STATUS_NOERROR))
    {
        pI2C->Command = I2C_COMMAND_READ;
        status = I2CAccessBuffer(pHWDE, pI2C, 0xA0, ulOffset, pjBuffer, 128);
        if ((status == 0) && (pI2C->Status == I2C_STATUS_NOERROR))
        {
            if (*pjBuffer != (UCHAR)ulBlockTag)
            {
                return -1;
            }

            if (bEDIDCheckSum(pjBuffer, 128) != 0)
            {
                return -1;
            }
        }
        else
        {
            return ERROR_INVALID_PARAMETER;
        }
    }
    else
    {
          return ERROR_INVALID_PARAMETER;
    }


    return NO_ERROR;
}
char I2COpen (PXGI_HW_DEVICE_INFO  pHWDE, ULONG ulI2CEnable, ULONG ulChannelID, PI2CControl pI2CControl)
{
/*
//    printk("\nI2COpen(%d) : Channel ID = %d\n", ulI2CEnable, ulChannelID);
    // we need to determine the Context area for each command we receive
    // i.e. which hardware I2C bus is the command for.
    // this is unimplemented here!
*/
    if (ulChannelID >= MAX_I2C_CHANNEL)
    {
        return ERROR_INVALID_PARAMETER;
    }
    if (ulI2CEnable)    /* Open I2C Port */
    {
/*        // verify clock rate. If you cannot implement the given rate,
        // enable a lower clock rate closest to the request clock rate.
        //
        // put a better check here if your device can only do discrete
        // clock rate values.
*/
        if (pI2CControl->ClockRate > I2C_MAX_CLOCK_RATE)
        {
            pI2CControl->ClockRate = I2C_MAX_CLOCK_RATE;
        }
        pI2CControl->Status = I2C_STATUS_NOERROR;
    }
    else    /* Close I2C Port*/
    {
        /* Set Acquired state to FALSE */

        pI2CControl->dwCookie = 0;
        /* Set status */
        pI2CControl->Status = I2C_STATUS_NOERROR;
    }



    return 0;
}
/* end of I2COpen */
char I2CAccess(PXGI_HW_DEVICE_INFO pHWDE, PI2CControl pI2CControl)
{
    ULONG                       ulChannel = pI2CControl->dwCookie % MAX_I2C_CHANNEL;


    if (pI2CControl->ClockRate > I2C_MAX_CLOCK_RATE)
    {
        pI2CControl->ClockRate = I2C_MAX_CLOCK_RATE;
    }
    if (pI2CControl->ClockRate == 0)  {
        pI2CControl->ClockRate = 20000;
    }

    pHWDE->I2CDelay = (1000000 / pI2CControl->ClockRate) * 10 * 2; /* in 100ns */
    /* pHWDE->I2CDelay = (1000000 / pI2CControl->ClockRate) * 10; */ /* in 100ns */
    /* pHWDE->I2CDelay = 100; */
	/* PDEBUG(ErrorF("I2CAccess()-I2CDelay = %d...\n", pHWDE->I2CDelay)); */

	/* pHWDE->I2CDelay = 100; */ /* Jong@08032009 */

    switch (pI2CControl->Command)
    {
        /* Issue a STOP or START without a READ or WRITE Command */
        case I2C_COMMAND_NULL:
            if (I2CNull(pHWDE, pI2CControl) == FALSE)  break;
/*          if (pI2CControl->Flags & I2C_FLAGS_STOP)  {
                pI2CContext->dwI2CPortAcquired = FALSE;
            }
*/          break;

        /* READ or WRITE Command */
        case I2C_COMMAND_READ:
            if (I2CRead(pHWDE, pI2CControl) == FALSE)  break;
/*          if (pI2CControl->Flags & I2C_FLAGS_STOP)  {
                pI2CContext->dwI2CPortAcquired = FALSE;
            }
*/          break;

        case I2C_COMMAND_WRITE:
            if (I2CWrite(pHWDE, pI2CControl) == FALSE)  break;
/*          if (pI2CControl->Flags & I2C_FLAGS_STOP)  {
                pI2CContext->dwI2CPortAcquired = FALSE;
            }
*/          break;

        case I2C_COMMAND_STATUS:
         pI2CControl->Status = I2C_STATUS_NOERROR;
            break;

        case I2C_COMMAND_RESET:
            /* Reset I2C bus */
            if (ResetI2C(pHWDE, pI2CControl) == FALSE)  break;
            break;

        default:
            /* Invalid Command */
            return ERROR_INVALID_PARAMETER;
    }
    

/*    printk("\nI2CAccess(): I2C Cmd = 0x%X I2C Flags = 0x%X I2C Status = 0x%X I2C Data = 0x%X", pI2CControl->Command, pI2CControl->Flags, pI2CControl->Status, pI2CControl->Data); */

    return NO_ERROR;
}


/*^^*
 * Function:    I2CNull
 *
 * Purpose:             To complete an I2C instruction.
 *
 * Inputs:              lpI2C_ContextData : PI2C_CONTEXT, pointer to data struct associated
 *                                          with card being accessed
 *                                I2CCntl : PI2CControl, pointer to I2C control structure
 *
 * Outputs:             void.
 *^^*/
BOOLEAN I2CNull( PXGI_HW_DEVICE_INFO pHWDE,  PI2CControl pI2CControl)
{
    pI2CControl->Status = I2C_STATUS_ERROR;

/*///// Issue STOP - START, if I2C_FLAGS_DATACHAINING ////////*/
    if (pI2CControl->Flags & I2C_FLAGS_DATACHAINING)
    {
        if (Stop(pHWDE) == FALSE)  return FALSE;
        if (Start(pHWDE) == FALSE)  return FALSE;
    }

/*///// Issue START ////////*/
    if (pI2CControl->Flags & I2C_FLAGS_START)
    {
        if (Start(pHWDE) == FALSE)  return FALSE;
    }

/*////// Issue STOP /////////*/
    if (pI2CControl->Flags & I2C_FLAGS_STOP)
    {
        if (Stop(pHWDE) == FALSE)  return FALSE;
    }

    pI2CControl->Status = I2C_STATUS_NOERROR;
    return TRUE;
}/* end of I2CNull()*/

/*^^*
 * Function:    I2CRead
 *
 * Purpose:             To complete an I2C instruction.
 *
 * Inputs:              lpI2C_ContextData : PI2C_CONTEXT, pointer to data struct associated
 *                                          with card being accessed
 *                                I2CCntl : PI2CControl, pointer to I2C control structure
 *
 * Outputs:             void.
 *^^*/
BOOLEAN I2CRead(PXGI_HW_DEVICE_INFO pHWDE, PI2CControl pI2CControl)
{
    pI2CControl->Status = I2C_STATUS_ERROR;

/*///// Issue STOP - START, if I2C_FLAGS_DATACHAINING ////////*/
    if (pI2CControl->Flags & I2C_FLAGS_DATACHAINING)
    {
        if (Stop(pHWDE) == FALSE)  return FALSE;
        if (Start(pHWDE) == FALSE)  return FALSE;
    }

/*///// Issue START ////////*/
    if (pI2CControl->Flags & I2C_FLAGS_START)
    {
        if (Start(pHWDE) == FALSE)  return FALSE;
    }

/*///// Read Command ///////*/
    if (ReadUCHARI2C(pHWDE, &pI2CControl->Data) == FALSE)
        return FALSE;

    if (pI2CControl->Flags & I2C_FLAGS_STOP)  {
        if (NoAck(pHWDE) == FALSE)  return FALSE;
    }
    else  {  /* Should we issue ACK*/
        if (pI2CControl->Flags & I2C_FLAGS_ACK)  {
            if (Ack(pHWDE, SEND_ACK) == FALSE)  return FALSE;
        }
    }

/*////// Issue STOP /////////*/
    if (pI2CControl->Flags & I2C_FLAGS_STOP)
    {
        if (Stop(pHWDE) == FALSE)  return FALSE;
    }

    pI2CControl->Status = I2C_STATUS_NOERROR;
    return TRUE;
} /* end of I2CRead() */

/*^^*
 * Function:    I2CWrite
 *
 * Purpose:             To complete an I2C instruction.
 *
 * Inputs:              lpI2C_ContextData : PI2C_CONTEXT, pointer to data struct associated
 *                                          with card being accessed
 *                                I2CCntl : PI2CControl, pointer to I2C control structure
 *
 * Outputs:             void.
 *^^*/
BOOLEAN I2CWrite(PXGI_HW_DEVICE_INFO pHWDE,  PI2CControl pI2CControl)
{
    pI2CControl->Status = I2C_STATUS_ERROR;

/*///// Issue STOP - START, if I2C_FLAGS_DATACHAINING ////////*/
    if (pI2CControl->Flags & I2C_FLAGS_DATACHAINING)
    {
        if (Stop(pHWDE) == FALSE) 
        {
             
             return FALSE;
        }
        if (Start(pHWDE) == FALSE)  
        {
            
 
            return FALSE;
        }
    }

/*///// Issue START ////////*/
    if (pI2CControl->Flags & I2C_FLAGS_START)
    {
        if (Start(pHWDE) == FALSE) 
         {
            return FALSE;
          }
    }

/*///// Write Command ///////*/
    if (WriteUCHARI2C(pHWDE, pI2CControl->Data) == FALSE)
    {
   
       return FALSE;
     }

    if (pI2CControl->Flags & I2C_FLAGS_ACK)  {
        if (Ack(pHWDE, RECV_ACK) == FALSE) 
        {
            return FALSE;
        }
    }

/*////// Issue STOP /////////*/
    if (pI2CControl->Flags & I2C_FLAGS_STOP)
    {
        if (Stop(pHWDE) == FALSE) 
        {
           return FALSE;
        }
    }

    pI2CControl->Status = I2C_STATUS_NOERROR;
    return TRUE;
} /* end of I2CWrite() */

/*^^*
 * Function:    ResetI2C
 *
 * Purpose:             To reset I2CBus
 *
 * Inputs:              lpI2C_ContextData : PI2C_CONTEXT, pointer to data struct associated with
 *                                          lpI2C_ContextData being accessed
 *
 * Outputs:             UCHAR, the data that was read.
 *^^*/
BOOLEAN ResetI2C(PXGI_HW_DEVICE_INFO pHWDE,  PI2CControl pI2CControl)
{

    if (Stop(pHWDE) == TRUE)  {
        pI2CControl->Status = I2C_STATUS_NOERROR;
        return TRUE;
    }
    else  {
        pI2CControl->Status = I2C_STATUS_ERROR;
        return FALSE;
    }
} /* ResetI2C() */




/*^^*
 * Function:    Ack
 *
 * Purpose:             To ask the I2C bus for an acknowledge.
 *
 * Inputs:              lpI2C_ContextData : PI2C_CONTEXT, pointer to data struct associated with
 *                      lpI2C_ContextData being accessed
 *
 * Outputs:             char, ack status.
 *^^*/
BOOLEAN Ack (PXGI_HW_DEVICE_INFO pHWDE,  bool fPut)
{
    BOOLEAN     status = FALSE;
    ULONG       i, delay, delay2;
    ULONG       ack;

    delay = pHWDE->I2CDelay / 10 / 2;
    if (fPut == SEND_ACK)   /* Send Ack into I2C bus */
    {
        vWriteDataLine(pHWDE, LODAT);
        I2C_DelayUS(delay);

        vWriteClockLine(pHWDE, HICLK);
        I2C_DelayUS(delay);
        if (bReadClockLine(pHWDE) != HICLK)  {
            i = 0;
            delay2 = delay * 2;
            I2C_DelayUS(delay2); /* Jong@08052008 */
            do {
                vWriteClockLine(pHWDE, HICLK);
                I2C_DelayUS(delay2);
                if (bReadClockLine(pHWDE) == HICLK)  break;
                i++;
                delay2 *= 2;
                if (i >= I2C_RETRY_COUNT)  return FALSE;
            } while (1);
        }


        I2C_DelayUS(delay); /* Jong@08052008 */
        vWriteClockLine(pHWDE, LOCLK);
        I2C_DelayUS(delay);

        return TRUE;
    }
    else
    {
        /* Receive Ack from I2C bus */
        vWriteDataLine(pHWDE, HIDAT);
        I2C_DelayUS(delay);
        ack = bReadDataLine(pHWDE);

        I2C_DelayUS(delay); /* Jong@08052008 */
        vWriteClockLine(pHWDE, HICLK);
        I2C_DelayUS(delay);
        if (bReadClockLine(pHWDE) != HICLK)  {
            i = 0;
            delay2 = delay * 2;
            do {
	            I2C_DelayUS(delay2); /* Jong@08052008 */
                vWriteClockLine(pHWDE, HICLK);
                I2C_DelayUS(delay2);
                if (bReadClockLine(pHWDE) == HICLK)  break;
                i++;
                delay2 *= 2;
                if (i >= I2C_RETRY_COUNT)  return FALSE;
            } while (1);
        }

        I2C_DelayUS(delay); /* Jong@08052008 */
        status = bReadDataLine(pHWDE);

        I2C_DelayUS(delay); /* Jong@08052008 */
        vWriteClockLine(pHWDE, LOCLK);
        I2C_DelayUS(delay);

        if (status != LODAT)  {
            if (ack == LODAT)  {
                status = LODAT;
            }

            else  {
               
            }

        }
        return (BOOLEAN)(status == LODAT);
    }
}/* end of Ack() */


BOOLEAN NoAck(PXGI_HW_DEVICE_INFO pHWDE)
{
    ULONG       i, delay, delay2;

    delay = pHWDE->I2CDelay / 10 / 2;

    vWriteDataLine(pHWDE, HIDAT);
    I2C_DelayUS(delay);

    vWriteClockLine(pHWDE, HICLK);
    I2C_DelayUS(delay);
    if (bReadClockLine(pHWDE) != HICLK)  {
        delay2 = delay * 2;
        i = 0;
        do  {
	        I2C_DelayUS(delay2); /* Jong@08052008 */
            vWriteClockLine(pHWDE, HICLK);
            I2C_DelayUS(delay2);
            if (bReadClockLine(pHWDE) == HICLK)  break;
            i++;
            delay2 *= 2;
            if (i >= I2C_RETRY_COUNT) return FALSE;
        } while (1);
    }

    I2C_DelayUS(delay); /* Jong@08052008 */
    vWriteClockLine(pHWDE, LOCLK);
    I2C_DelayUS(delay);

    return TRUE;
}


/*^^*
 * Function:    Start
 *
 * Purpose:             To start a transfer on the I2C bus.
 *
 * Inputs:              lpI2C_ContextData : PI2C_CONTEXT, pointer to data struct associated with
 *                      lpI2C_ContextData being accessed
 *
 * Outputs:             void.
 *^^*/
BOOLEAN Start( PXGI_HW_DEVICE_INFO pHWDE)
{
    ULONG       i, delay, delay2;

    delay = pHWDE->I2CDelay / 10 / 2;

    vWriteDataLine(pHWDE, HIDAT);
    I2C_DelayUS(delay);
    if (bReadDataLine(pHWDE) != HIDAT)  {
        delay2 = delay * 2;
        i = 0;
        do  {
	        I2C_DelayUS(delay2); /* Jong@08052008 */
            vWriteDataLine(pHWDE, HIDAT);
            I2C_DelayUS(delay2);
            if (bReadDataLine(pHWDE) == HIDAT)  break;
            i++;
            delay2 *= 2;
            if (i >= I2C_RETRY_COUNT)  return FALSE;
        } while (1);
    }

    I2C_DelayUS(delay); /* Jong@08052008 */
    vWriteClockLine(pHWDE, HICLK);
    I2C_DelayUS(delay);
    if (bReadClockLine(pHWDE) != HICLK)  {
        delay2 = delay * 2;
        i = 0;
        do  {
	        I2C_DelayUS(delay2); /* Jong@08052008 */
            vWriteClockLine(pHWDE, HICLK);
            I2C_DelayUS(delay2);
            if (bReadClockLine(pHWDE) == HICLK)  break;
            i++;
            delay2 *= 2;
            if (i >= I2C_RETRY_COUNT) return FALSE;
        } while (1);
    }

    I2C_DelayUS(delay); /* Jong@08052008 */
    vWriteDataLine(pHWDE, LODAT);
    I2C_DelayUS(delay);

    vWriteClockLine(pHWDE, LOCLK);
    I2C_DelayUS(delay);

    return TRUE;
}/* end of Start */

/*^^*
 * Function:    Stop
 *
 * Purpose:             To stop a transfer on the I2C bus.
 *
 * Inputs:              lpI2C_ContextData : PI2C_CONTEXT, pointer to data struct associated with
 *                                          lpI2C_ContextData being accessed
 *
 * Outputs:             void.
 *^^*/
BOOLEAN Stop(PXGI_HW_DEVICE_INFO pHWDE)
{
    ULONG       i, delay, delay2;

    delay = pHWDE->I2CDelay / 10 / 2;
	PDEBUGI2C(ErrorF("Stop()-begin-pHWDE->I2CDelay=%d, delay=%d...\n", pHWDE->I2CDelay, delay));

    vWriteDataLine(pHWDE, LODAT);
    I2C_DelayUS(delay);

    vWriteClockLine(pHWDE, HICLK);
    I2C_DelayUS(delay);
    if (bReadClockLine(pHWDE) != HICLK)  {
        i = 0;
        delay2 = delay * 2;
        do {
	        I2C_DelayUS(delay2); /* Jong@08052008 */
            vWriteClockLine(pHWDE, HICLK);
            I2C_DelayUS(delay2);
            if (bReadClockLine(pHWDE) == HICLK)  break;
            i++;
            delay2 *= 2;
            if (i >= I2C_RETRY_COUNT)  return FALSE;
        } while (1);
    }

    I2C_DelayUS(delay); /* Jong@08052008 */
    vWriteDataLine(pHWDE, HIDAT);
    I2C_DelayUS(delay);

    return (BOOLEAN)(bReadDataLine(pHWDE) == HIDAT);
}/* end of Stop*/

/*^^*
 * Function:    WriteUCHARI2C
 *
 * Purpose:             To write a UCHAR of data to the I2C bus.
 *
 * Inputs:              lpI2C_ContextData : PI2C_CONTEXT, pointer to data struct associated with
 *                                          lpI2C_ContextData being accessed
 *                                   cData: UCHAR, the data to write
 *
 * Outputs:             void.
 *^^*/
BOOLEAN WriteUCHARI2C(PXGI_HW_DEVICE_INFO pHWDE,  UCHAR cData)
{
    ULONG  i, j, delay, delay2;

    cData = ReverseUCHAR(cData);

    delay = pHWDE->I2CDelay / 10 / 2;

    for (j=0; j<8; j++, cData>>=1)
    {
        I2C_DelayUS(delay); /* Jong@08052008 */
        vWriteDataLine(pHWDE, cData);
        I2C_DelayUS(delay);

        vWriteClockLine(pHWDE, HICLK);
        I2C_DelayUS(delay);
        if (bReadClockLine(pHWDE) != HICLK)  {
            i = 0;
            delay2 = delay * 2;
            do {
		        I2C_DelayUS(delay2); /* Jong@08052008 */
                vWriteClockLine(pHWDE, HICLK);
                I2C_DelayUS(delay2);
                if (bReadClockLine(pHWDE) == HICLK)  break;
                i++;
                delay2 *= 2;
                if (i >= I2C_RETRY_COUNT)  return FALSE;
            } while (1);
        }

        I2C_DelayUS(delay); /* Jong@08052008 */
        vWriteClockLine(pHWDE, LOCLK);
        I2C_DelayUS(delay);
    }
    return TRUE;
}/* end of WriteUCHARI2C */

/*^^*
 * Function:    ReadUCHARI2C
 *
 * Purpose:             To read a UCHAR of data from the I2C bus.
 *
 * Inputs:              lpI2C_ContextData : PI2C_CONTEXT, pointer to data struct associated with
 *                                          lpI2C_ContextData being accessed
 *
 * Outputs:             UCHAR, the data that was read.
 *^^*/
BOOLEAN ReadUCHARI2C(PXGI_HW_DEVICE_INFO pHWDE,  PUCHAR pBuffer)
{
    ULONG       ulReadData, data, i, j, delay, delay2;

    delay = pHWDE->I2CDelay / 10 / 2;

    vWriteDataLine(pHWDE, HIDAT);
    I2C_DelayUS(delay);

    ulReadData = 0;
    for (j = 0; j < 8; j++)
    {
        vWriteClockLine(pHWDE, HICLK);
        I2C_DelayUS(delay);
        if (bReadClockLine(pHWDE) != HICLK)  {
            i = 0;
            delay2 = delay * 2;
            do {
		        I2C_DelayUS(delay2); /* Jong@08052008 */
                vWriteClockLine(pHWDE, HICLK);
                I2C_DelayUS(delay2);
                if (bReadClockLine(pHWDE) == HICLK)  break;
                i++;
                delay2 *= 2;
                if (i >= I2C_RETRY_COUNT)  return FALSE;
            } while (1);
        }

        I2C_DelayUS(delay); /* Jong@08052008 */
        data = bReadDataLine(pHWDE);
        ulReadData = (ulReadData << 1) | (data & 1);

        I2C_DelayUS(delay); /* Jong@08052008 */
        vWriteClockLine(pHWDE, LOCLK);
        I2C_DelayUS(delay);

        vWriteDataLine(pHWDE, HIDAT);
        I2C_DelayUS(delay);
    }

    *pBuffer = (UCHAR) ulReadData;
    return TRUE;
}


UCHAR ReverseUCHAR(UCHAR data)
{
    UCHAR  rdata = 0;
    int   i;

    for (i=0; i<8; i++)
    {
        rdata <<= 1;
        rdata |= (data & 1);
        data  >>= 1;
    }

    return(rdata);
}

/************************************************************************************
 For DVI I2C Interface
***************************************************************************************/

/*************************************************************************
// VOID vWriteClockLineDVI()
// IOReg xx14, index 0F is defined as follows:
//
//      ...    1      0
// --------|------|-------|
//      ...| Data | Clock |
// ------------------------
*************************************************************************/
VOID vWriteClockLineDVI(PXGI_HW_DEVICE_INFO pHWDE, UCHAR data)
{
    UCHAR        temp;
    XGIIOADDRESS pjI2cIOBase;

	PDEBUGI2C(ErrorF("vWriteClockLineDVI()...begin\n"));

    if ((pHWDE->jChipType < XG21)&&(pHWDE->jChipType != XG27))
    {
		ErrorF("vWriteClockLineDVI()...0\n");
    }
    else
    {
		PDEBUGI2C(ErrorF("vWriteClockLineDVI()...1\n"));
        pjI2cIOBase = pHWDE->pjIOAddress + CRTC_ADDRESS_PORT_COLOR;

        /* Enable GPIOA Write */

        EnableGPIOA(pjI2cIOBase, I2C_WRITE);

		PDEBUGI2C(ErrorF("*1 - pHWDE->ucI2cDVI = %d\n", pHWDE->ucI2cDVI));
        pHWDE->ucI2cDVI = (pHWDE->ucI2cDVI & MASK(1:1)) | SETBITS(data, 0:0);
		PDEBUGI2C(ErrorF("*2 - pHWDE->ucI2cDVI = %d\n", pHWDE->ucI2cDVI));

        temp = XGI_GetReg(pjI2cIOBase, IND_CR48_GPIO_REG_I);
		PDEBUGI2C(ErrorF("IND_CR48_GPIO_REG_I = %d\n", temp));

        {
            UCHAR temp2 = getGPIORWTransfer(temp);
			PDEBUGI2C(ErrorF("temp2 = %d\n", temp2));
            temp = temp2;
        }

        temp = (temp & (~MASK(1:0))) | pHWDE->ucI2cDVI;
		PDEBUGI2C(ErrorF("temp= %d\n", temp));
        XGI_SetReg(pjI2cIOBase,IND_CR48_GPIO_REG_I, temp);
    }

	PDEBUGI2C(ErrorF("vWriteClockLineDVI()...end\n"));  
}

/*************************************************************************
// VOID vWriteDataLineDVI()
// IOReg xx14, index 0F is defined as follows:
//
//      ...    1      0
// --------|------|-------|
//      ...| Data | Clock |
// ------------------------
*************************************************************************/
VOID vWriteDataLineDVI(PXGI_HW_DEVICE_INFO pHWDE, UCHAR data)
{
    UCHAR        temp;
    XGIIOADDRESS pjI2cIOBase;

	PDEBUGI2C(ErrorF("vWriteDataLineDVI()...begin\n"));

    if ((pHWDE->jChipType < XG21)&&(pHWDE->jChipType != XG27))
    {   
		ErrorF("vWriteDataLineDVI()...0\n");
    }
    else
    {
		PDEBUGI2C(ErrorF("vWriteDataLineDVI()...1\n"));
        pjI2cIOBase = pHWDE->pjIOAddress + CRTC_ADDRESS_PORT_COLOR;

        
        /* Enable GPIOB Write */
        
        EnableGPIOB(pjI2cIOBase, I2C_WRITE);

 		PDEBUGI2C(ErrorF("*1 - pHWDE->ucI2cDVI = %d\n", pHWDE->ucI2cDVI));
        pHWDE->ucI2cDVI = (pHWDE->ucI2cDVI & MASK(0:0)) | SETBITS(data, 1:1);
 		PDEBUGI2C(ErrorF("*2 - pHWDE->ucI2cDVI = %d\n", pHWDE->ucI2cDVI));

        temp = XGI_GetReg(pjI2cIOBase, IND_CR48_GPIO_REG_I);
		PDEBUGI2C(ErrorF("IND_CR48_GPIO_REG_I = %d\n", temp));

        {
            UCHAR temp2 = getGPIORWTransfer(temp);
			PDEBUGI2C(ErrorF("temp2 = %d\n", temp2));

            temp = temp2;
        }

        temp = (temp & (~MASK(1:0))) | pHWDE->ucI2cDVI;
		PDEBUGI2C(ErrorF("temp = %d\n", temp));

        XGI_SetReg(pjI2cIOBase,IND_CR48_GPIO_REG_I, temp);
    }

	PDEBUGI2C(ErrorF("vWriteDataLineDVI()...end\n")); 
}

/*************************************************************************
// BOOLEAN bReadClockLineDVI()
// IOReg xx14, index 0F is defined as follows:
//
//      ...    1      0
// --------|------|-------|
//      ...| Data | Clock |
// ------------------------
*************************************************************************/
BOOLEAN bReadClockLineDVI(PXGI_HW_DEVICE_INFO pHWDE)
{
    UCHAR        cPortData;
    XGIIOADDRESS pjI2cIOBase;

	PDEBUGI2C(ErrorF("bReadClockLineDVI()...begin\n"));

    if ((pHWDE->jChipType != XG21)&&(pHWDE->jChipType != XG27))
    {
		ErrorF("bReadClockLineDVI()...0\n");
    }
    else
    {  
		PDEBUGI2C(ErrorF("bReadClockLineDVI()...1\n"));
        pjI2cIOBase = pHWDE->pjIOAddress + CRTC_ADDRESS_PORT_COLOR;

        /* Enable GPIOA READ */

        EnableGPIOA(pjI2cIOBase, I2C_READ);

        cPortData = XGI_GetReg(pjI2cIOBase, IND_CR48_GPIO_REG_I);
		PDEBUGI2C(ErrorF("*1 - cPortData = %d...\n", cPortData));

        cPortData = GETBITS(cPortData, 7:7);
		PDEBUGI2C(ErrorF("*2 - cPortData = %d...\n", cPortData));
    }

	PDEBUGI2C(ErrorF("bReadClockLineDVI()...return(cPortData=%d)\n", cPortData));
    return cPortData;
}

/*************************************************************************
// BOOLEAN bReadDataLineDVI()
// IOReg xx14, index 0F is defined as follows:
//
//      ...    1      0
// --------|------|-------|
//      ...| Data | Clock |
// ------------------------
*************************************************************************/
BOOLEAN bReadDataLineDVI(PXGI_HW_DEVICE_INFO pHWDE)
{
    UCHAR       cPortData;
    XGIIOADDRESS      pjI2cIOBase; 

	PDEBUGI2C(ErrorF("bReadDataLineDVI()...begin\n"));

    if ((pHWDE->jChipType != XG21)&&(pHWDE->jChipType != XG27))
    {
		ErrorF("bReadDataLineDVI()...0\n");
    }
    else
    {
		PDEBUGI2C(ErrorF("bReadDataLineDVI()...1\n"));
        pjI2cIOBase = pHWDE->pjIOAddress + CRTC_ADDRESS_PORT_COLOR;

        
        /* Enable GPIOB Write */
        
        EnableGPIOB(pjI2cIOBase, I2C_READ);

        cPortData = XGI_GetReg(pjI2cIOBase, IND_CR48_GPIO_REG_I);
		PDEBUGI2C(ErrorF("*1 - cPortData = %d...\n", cPortData));
        cPortData = GETBITS(cPortData, 6:6);
		PDEBUGI2C(ErrorF("*2 - cPortData = %d...\n", cPortData));
    }


	PDEBUGI2C(ErrorF("bReadDataLineDVI()...return(cPortData=%d)\n", cPortData));
    return cPortData;
}

//*************************************************************************//
// VOID vWaitForCRT1VsyncActive()
// IoReg 3DA
//      ...        3           ...
// --------|--------------|-------|
//      ...| Vertical Sync|       |
// --------------------------------
//*************************************************************************//
VOID vWaitForCRT1HsyncActive(PXGI_HW_DEVICE_INFO  pHWDE)
{
    XGIIOADDRESS  pjPort = pHWDE->pjIOAddress + INPUT_STATUS_1_COLOR;
    ULONG   i;

    for (i = 0; i < 0x00FFFF; i++)
    {
       if ( (XGI_GetRegByte(pjPort) & 0x01) == 0)
       {
          // Inactive
          break;
       } //if
    } //for-loop

    for (i = 0; i < 0x00FFFF; i++)
    {
       if ( (XGI_GetRegByte(pjPort) & 0x01) != 0)
       {
          // active
          break;
       } //if
    } //for-loop

} //vWaitForCRT1HsyncActive()

////////////////////////////////////////////////////////////////////////////////////
//
// For CRT I2C Interface
//
////////////////////////////////////////////////////////////////////////////////////

//*************************************************************************//
// VOID vWriteClockLineCRT()
// IOReg SR11 is defined as follows:
//
//      ...    1      0
// --------|------|-------|
//      ...| Data | Clock |
// ------------------------
//*************************************************************************//
VOID vWriteClockLineCRT(PXGI_HW_DEVICE_INFO pHWDE, UCHAR data)
{
    UCHAR       temp, ujSR1F;
    XGIIOADDRESS      pjI2cIOBase = pHWDE->pjIOAddress + SEQ_ADDRESS_PORT;

    PDEBUGI2C(ErrorF("I2C:Write CRT clock = %x\n", data & 1));

    ujSR1F  =  XGI_GetReg(pjI2cIOBase, IND_SR1F_POWER_MANAGEMENT);

    pHWDE->ucI2cCRT = (pHWDE->ucI2cCRT & MASK(1:1)) | SETBITS(data, 0:0);

    temp = XGI_GetReg(pjI2cIOBase, IND_SR11_DDC_REG);
    temp = (temp & (~MASK(1:0))) | pHWDE->ucI2cCRT;

    //wait for CRT1 retrace only when CRT1 is enabled!
    /* if (pHWDE->bMonitorPoweredOn) */ /* jong@08042009; ignore here */
    {
        if(!(data & 1) && ((ujSR1F & 0xC0)==0) )
        {
            vWaitForCRT1HsyncActive(pHWDE);
        }
    }

    XGI_SetReg(pjI2cIOBase, IND_SR11_DDC_REG, temp);
}

//*************************************************************************//
// VOID vWriteDataLineCRT()
// IOReg SR11 is defined as follows:
//
//      ...    1      0
// --------|------|-------|
//      ...| Data | Clock |
// ------------------------
//*************************************************************************//
VOID vWriteDataLineCRT(PXGI_HW_DEVICE_INFO pHWDE, UCHAR data)
{
    UCHAR       temp, ujSR1F;
    XGIIOADDRESS      pjI2cIOBase = pHWDE->pjIOAddress + SEQ_ADDRESS_PORT;

    PDEBUGI2C(ErrorF("I2C:Write CRT data = %x\n", data & 1));

    ujSR1F  = XGI_GetReg(pHWDE->pjIOAddress + SEQ_ADDRESS_PORT, IND_SR1F_POWER_MANAGEMENT);

    pHWDE->ucI2cCRT = (pHWDE->ucI2cCRT & MASK(0:0)) | SETBITS(data, 1:1);
    temp = XGI_GetReg(pjI2cIOBase, IND_SR11_DDC_REG);
    temp = (temp & (~MASK(1:0))) | pHWDE->ucI2cCRT;

    //wait for CRT1 retrace only when CRT1 is enabled!
    /* if (pHWDE->bMonitorPoweredOn) */ /* Jong@08042009; ignore checking */
    {
        if(!(data & 1) && ((ujSR1F & 0xC0)==0) )
        {
            vWaitForCRT1HsyncActive(pHWDE);
        }
    }

    XGI_SetReg(pjI2cIOBase, IND_SR11_DDC_REG, temp);
}

//*************************************************************************//
// BOOLEAN bReadClockLineCRT()
// IOReg SR11 is defined as follows:
//
//      ...    1      0
// --------|------|-------|
//      ...| Data | Clock |
// ------------------------
//*************************************************************************//
BOOLEAN bReadClockLineCRT(PXGI_HW_DEVICE_INFO pHWDE)
{
    UCHAR       cPortData;
    XGIIOADDRESS      pjI2cIOBase = pHWDE->pjIOAddress + SEQ_ADDRESS_PORT;

    cPortData = XGI_GetReg(pjI2cIOBase, IND_SR11_DDC_REG);
    cPortData = GETBITS(cPortData, 0:0);

    PDEBUGI2C(ErrorF("I2C:Read Channel CRT clock = %x\n", cPortData));

    return cPortData;
}

//*************************************************************************//
// BOOLEAN bReadDataLineCRT()
// IOReg SR11 is defined as follows:
//
//      ...    1      0
// --------|------|-------|
//      ...| Data | Clock |
// ------------------------
//*************************************************************************//
BOOLEAN bReadDataLineCRT(PXGI_HW_DEVICE_INFO pHWDE)
{
    UCHAR cPortData;
    XGIIOADDRESS      pjI2cIOBase = pHWDE->pjIOAddress + SEQ_ADDRESS_PORT;

    cPortData = XGI_GetReg(pjI2cIOBase, IND_SR11_DDC_REG);
    cPortData = GETBITS(cPortData, 1:1);

    PDEBUGI2C(ErrorF("I2C:Read Channel CRT data = %x\n", cPortData));

    return cPortData;
}

////////////////////////////////////////////////////////////////////////////////////
//
// For Feature Connector I2C Interface
//
////////////////////////////////////////////////////////////////////////////////////

//*************************************************************************//
// VOID vWriteClockLineFCNT()
// IOReg SR11 is defined as follows:
//
//      ...    3      2        ...
// --------|------|-------|-------|
//      ...| Data | Clock |       |
// --------------------------------
//*************************************************************************//
VOID vWriteClockLineFCNT(PXGI_HW_DEVICE_INFO pHWDE, UCHAR data)
{
    UCHAR       temp;
    XGIIOADDRESS      pjI2cIOBase = pHWDE->pjIOAddress + SEQ_ADDRESS_PORT;

    PDEBUGI2C(ErrorF("I2C:Write FCNT clock = %x\n", data & 1));

    pHWDE->ucI2cFCNT = (pHWDE->ucI2cFCNT & MASK(3:3)) | SETBITS(data, 2:2);

    temp = XGI_GetReg(pjI2cIOBase, IND_SR11_DDC_REG);
    temp = (temp & (~MASK(3:2))) | pHWDE->ucI2cFCNT;

    XGI_SetReg(pjI2cIOBase, IND_SR11_DDC_REG, temp);
}

//*************************************************************************//
// VOID vWriteDataLineFCNT()
// IOReg SR11 is defined as follows:
//
//      ...    3      2        ...
// --------|------|-------|-------|
//      ...| Data | Clock |       |
// --------------------------------
//*************************************************************************//
VOID vWriteDataLineFCNT(PXGI_HW_DEVICE_INFO pHWDE, UCHAR data)
{
    UCHAR       temp, temp2, temp3;
    XGIIOADDRESS      pjI2cIOBase = pHWDE->pjIOAddress + SEQ_ADDRESS_PORT;

    PDEBUGI2C(ErrorF("I2C:Write FCNT data = %x\n", data & 1));

    pHWDE->ucI2cFCNT = (pHWDE->ucI2cFCNT & MASK(2:2)) | SETBITS(data, 3:3);

    temp = XGI_GetReg(pjI2cIOBase, IND_SR11_DDC_REG);
    temp = (temp & (~MASK(3:2))) | pHWDE->ucI2cFCNT;

    XGI_SetReg(pjI2cIOBase, IND_SR11_DDC_REG, temp);
}

//*************************************************************************//
// BOOLEAN bReadClockLineFCNT()
// IOReg SR11 is defined as follows:
//
//      ...    3      2        ...
// --------|------|-------|-------|
//      ...| Data | Clock |       |
// --------------------------------
//*************************************************************************//
BOOLEAN bReadClockLineFCNT(PXGI_HW_DEVICE_INFO pHWDE)
{
    UCHAR       cPortData;
    XGIIOADDRESS      pjI2cIOBase = pHWDE->pjIOAddress + SEQ_ADDRESS_PORT;

    cPortData = XGI_GetReg(pjI2cIOBase, IND_SR11_DDC_REG);
    cPortData = GETBITS(cPortData, 2:2);

    PDEBUGI2C(ErrorF("I2C:Read Channel FCNT clock = %x\n", cPortData));

    return cPortData;
}

//*************************************************************************//
// BOOLEAN bReadDataLineFCNT()
// IOReg SR11 is defined as follows:
//
//      ...    3      2        ...
// --------|------|-------|-------|
//      ...| Data | Clock |       |
// --------------------------------
//*************************************************************************//
BOOLEAN bReadDataLineFCNT(PXGI_HW_DEVICE_INFO pHWDE)
{
    UCHAR       cPortData;
    XGIIOADDRESS      pjI2cIOBase = pHWDE->pjIOAddress + SEQ_ADDRESS_PORT;

    cPortData = XGI_GetReg(pjI2cIOBase, IND_SR11_DDC_REG);
    cPortData = GETBITS(cPortData, 3:3);

    PDEBUGI2C(ErrorF("I2C:Read Channel FCNT data = %x\n", cPortData));

    return cPortData;
}

/*=======================================================*/
VOID vWriteClockLine(PXGI_HW_DEVICE_INFO pHWDE, UCHAR data)
{
	if(pHWDE->crtno == 0)
		vWriteClockLineCRT(pHWDE, data); 
	else if(pHWDE->crtno == 1)
		vWriteClockLineDVI(pHWDE, data);
	else if(pHWDE->crtno == 2)
		vWriteClockLineFCNT(pHWDE, data);
	else
		ErrorF("Error(XGI) : Unkonwn output device! \n");
}

VOID vWriteDataLine(PXGI_HW_DEVICE_INFO pHWDE, UCHAR data)
{
	if(pHWDE->crtno == 0)
		vWriteDataLineCRT(pHWDE, data); 
	else if(pHWDE->crtno == 1)
	   vWriteDataLineDVI(pHWDE, data);
	else if(pHWDE->crtno == 2)
		vWriteDataLineFCNT(pHWDE, data);
	else
		ErrorF("Error(XGI) : Unkonwn output device! \n");
}

BOOLEAN bReadClockLine(PXGI_HW_DEVICE_INFO pHWDE)
{
	if(pHWDE->crtno == 0)
		return(bReadClockLineCRT(pHWDE)); 
	else if(pHWDE->crtno == 1)
		return(bReadClockLineDVI(pHWDE));
	else if(pHWDE->crtno == 2)
		return(bReadClockLineFCNT(pHWDE));
	else {
		ErrorF("Error(XGI) : Unkonwn output device! \n");
		return FALSE;
	}
}

BOOLEAN bReadDataLine(PXGI_HW_DEVICE_INFO pHWDE)
{
	if(pHWDE->crtno == 0)
		return(bReadDataLineCRT(pHWDE)); 
	else if(pHWDE->crtno == 1)
		return(bReadDataLineDVI(pHWDE));
	else if(pHWDE->crtno == 2)
		return(bReadDataLineFCNT(pHWDE));
	else {
		ErrorF("Error(XGI) : Unkonwn output device! \n");
		return FALSE;
	}
}

BOOLEAN bEDIDCheckSum(PUCHAR  pjEDIDBuf,ULONG   ulBufSize)
{
    ULONG  i;
    UCHAR  ujSum = 0;
    PUCHAR pujPtr;

    pujPtr = pjEDIDBuf;

    for (i = 0; i < ulBufSize; i++)
    {
  /*     printk("pujPtr=%x, ",*pujPtr); */
       ujSum += *(pujPtr++);
    } /*for-loop */

    return(ujSum);

} 

/* Jong 08/03/2009; Get EDID functions; ported from MS Windows */
//*************************************************************************\\
// VP_STATUS vGetEDID_1
//
// Routine Description:
// Get the EDID which is in version 1.x format via I2C.
//
// Arguments:
//      pHWDE           -   Hardware extension object pointer
//      pI2C            -   I2C pointer
//      pjBuffer        -   EDID Buffer
//      ulBufferSize    -   Buffer size
//
//
// Return Value:
//      Status of returning EDID.
//      NO_ERROR                -   success
//      ERROR_INVALID_PARAMETER -   failed
//
//****************************************************************************
VP_STATUS vGetEDID_1(
    PXGI_HW_DEVICE_INFO  pHWDE,
    PI2CControl pI2C,
    PUCHAR      pjBuffer,
    ULONG       ulBufferSize)
{
    VP_STATUS   status;

    PDEBUGI2C(ErrorF("vGetEDID_1()\n"));

    if ((ulBufferSize < 128) || (pjBuffer == NULL))  {
        return ERROR_INVALID_PARAMETER;
    }

    // Set Segment Block ID as 0 if Monitor support Enhanced-EDID
    /*
    pI2C->Command = I2C_COMMAND_WRITE_SEGMENT; // to replace I2C_COMMAND_WRITE
    pI2C->Data = 0;
    I2CAccessBuffer(pHWDE, pI2C, 0x60, 0, &(pI2C->Data), 0);
    */
    pI2C->Command = I2C_COMMAND_WRITE;
    pI2C->Data = 0;
    I2CAccessBuffer(pHWDE, pI2C, 0x60, 0, &(pI2C->Data), 0);

    pI2C->Command = I2C_COMMAND_READ;
    status = I2CAccessBuffer(pHWDE, pI2C, 0xA0, 0, pjBuffer, 128);
    if ((status == NO_ERROR) && (pI2C->Status == I2C_STATUS_NOERROR))
    {
        // Check Block 0 EDID Header and its checksum
        if ((*((PULONG)(pjBuffer  )) != 0xFFFFFF00) ||
            (*((PULONG)(pjBuffer+4)) != 0x00FFFFFF))
        {
            PDEBUGI2C(ErrorF("vGetEDID_1(): EDID Header Incorrect!!\n"));
            return ERROR_INVALID_PARAMETER;
        }

        if (bEDIDCheckSum(pjBuffer, 128) != 0)
        {
            if ((*((PULONG)(pjBuffer+0x60)) ==0x4d636e79)&& (*((PULONG)(pjBuffer+0x64)) ==0x65747361))
            {
                 return NO_ERROR;   //To Fix SyncMaster Checksum error issue
            }	

            PDEBUGI2C(ErrorF("vGetEDID_1(): EDID Checksum Error!!\n"));
            return ERROR_INVALID_PARAMETER;
        }

    }
    else
    {
        PDEBUGI2C(ErrorF("vGetEDID_1 : call I2CAccessBuffer(0xA0) fail !!!\n"));
        return status;
    }

    return NO_ERROR;
}

//*************************************************************************\\
// VP_STATUS vGetEDID_2
//
// Routine Description:
// Get the EDID which is in version 2.0 format via I2C.
//
// Arguments:
//      pHWDE           -   Hardware extension object pointer
//      pI2C            -   I2C pointer
//      pjBuffer        -   EDID Buffer
//      ulBufferSize    -   Buffer size
//
//
// Return Value:
//      Status of returning EDID.
//      NO_ERROR                -   success
//      ERROR_INVALID_PARAMETER -   failed
//
//****************************************************************************
VP_STATUS vGetEDID_2(
    PXGI_HW_DEVICE_INFO  pHWDE,
    PI2CControl pI2C,
    PUCHAR      pjBuffer,
    ULONG       ulBufferSize)
{
    VP_STATUS   status;

    PDEBUGI2C(ErrorF("vGetEDID_2()\n"));

    if ((ulBufferSize < 256) || (pjBuffer == NULL))  {
        return ERROR_INVALID_PARAMETER;
    }

    pI2C->Command = I2C_COMMAND_READ;
    status = I2CAccessBuffer(pHWDE, pI2C, 0xA2, 0, pjBuffer, 256);
    if ((status != NO_ERROR) || (pI2C->Status != I2C_STATUS_NOERROR))  {
        PDEBUGI2C(ErrorF("vGetEDID_2 : call I2CAccessBuffer(0xA2) fail !!!\n"));
        usleep(5);
        status = I2CAccessBuffer(pHWDE, pI2C, 0xA6, 0, pjBuffer, 256);
        if ((status != NO_ERROR) || (pI2C->Status != I2C_STATUS_NOERROR))  {
            PDEBUGI2C(ErrorF("vGetEDID_2 : call I2CAccessBuffer(0xA6) fail !!!\n"));
            return ERROR_INVALID_PARAMETER;
        }
    }

    if (*pjBuffer != 0x20)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (bEDIDCheckSum(pjBuffer, 256) != 0)
    {
        return ERROR_INVALID_PARAMETER;
    }

    return NO_ERROR;
}

//*************************************************************************\\
// BOOLEAN bGetEDID
//
// Routine Description:
//      For driver to get the monitor EDID through I2C. This function works similar
//      with VideoPortDDCMonitorHelper() does.
//
//
// Arguments:
//      pHWDE           -   Hardware extension object pointer
//      ulChannelID     -   Channel ID
//      pjEDIDBuffer    -   EDID Buffer
//      ulBufferSize    -   Buffer size
//
//
// Return Value:
//      Status of returning EDID.
//      TRUE            -   success
//      FALSE           -   failed
//
//****************************************************************************
BOOLEAN bGetEDID(
    PXGI_HW_DEVICE_INFO  pHWDE, 
    ULONG  ulChannelID,
    PUCHAR pjEDIDBuffer,
    ULONG  ulBufferSize)
{
    I2CControl  I2C;
    VP_STATUS   status;

    PDEBUGI2C(ErrorF("bGetEDID() is called.\n"));

    if ((ulBufferSize != 0) && (pjEDIDBuffer != NULL))
    {
        memset(pjEDIDBuffer, 0, ulBufferSize);
    }
    else
    {
	    PDEBUGI2C(ErrorF("bGetEDID()-(ulBufferSize == 0) || (pjEDIDBuffer == NULL)\n"));
        return FALSE;
    }

    if (I2COpen(pHWDE, I2C_OPEN, ulChannelID, &I2C) != NO_ERROR)
    {
	    PDEBUGI2C(ErrorF("bGetEDID()-I2COpen()-fail!\n"));
        return FALSE;
    }

    // Force Monitor using DDC2 protocal...
    I2C.ClockRate = I2C_MAX_CLOCK_RATE;
    I2C.Command = I2C_COMMAND_WRITE;
    I2C.Flags = I2C_FLAGS_STOP;
    I2C.Data = 0xFF;
    I2CAccess(pHWDE, &I2C);

    // Reset I2C bus
    I2C.Command = I2C_COMMAND_RESET;
    I2CAccess(pHWDE, &I2C);
    if (I2C.Status != I2C_STATUS_NOERROR)
    {
        PDEBUGI2C(ErrorF("bGetEDID() fail: Reset I2C bus fail.\n"));
        return FALSE;
    }

    status = vGetEDID_2(pHWDE, &I2C, pjEDIDBuffer, ulBufferSize);
    PDEBUGI2C(ErrorF("bGetEDID()-vGetEDID_2-status=%d\n", status == NO_ERROR ? 1:0));

    if (status != NO_ERROR)
    {
        usleep(5);
        status = vGetEDID_1(pHWDE, &I2C, pjEDIDBuffer, ulBufferSize);
	    PDEBUGI2C(ErrorF("bGetEDID()-vGetEDID_1-status=%d\n", status == NO_ERROR ? 1:0));

        if (status == NO_ERROR)
        {
            if (*(pjEDIDBuffer+0x7E) != 0)
            {
                vGetEDIDExtensionBlocks(pHWDE, &I2C, pjEDIDBuffer+128, ulBufferSize-128);
			    PDEBUGI2C(ErrorF("bGetEDID()-vGetEDIDExtensionBlocks()\n"));
            }
        }
        else
        {
            ErrorF( "bGetEDID() fail !!\n");
        }
    }

    I2COpen(pHWDE, I2C_CLOSE, ulChannelID, &I2C);

   PDEBUGI2C(ErrorF("bGetEDID()-return(%d)\n", status == NO_ERROR ? 1:0));
    return (status == NO_ERROR);
}
