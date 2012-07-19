#ifndef _VBI2C_
#define _VBI2C_
#include "vgatypes.h"
#ifdef LINUX_KERNEL
/* Jong@08052009 */
// #include <linux/delay.h> /* udelay */
#endif 
#ifndef u32
#define u32 unsigned long
#define u8 uint8_t
#endif

/* Jong@08052009 */
//#ifndef DelayUS
//#define DelayUS(p) udelay(p)
//#endif

#define IND_CR48_GPIO_REG_I   0x48
#define IND_CR4A_GPIO_REG_III 0x4A

#define IND_SR11_DDC_REG           0x11 // index of DDC register
#define IND_SR1F_POWER_MANAGEMENT  0x1F // index of power management reg

#define INPUT_STATUS_1_COLOR        0x002A  // Input Status 1 register read
                                            // port in color mode

/* Jong 08/03/2009; added for I2C */
/*--------------------------------*/
#define I2C_CRT         0
#define I2C_DVI         1
#define I2C_FCNT        2
#define I2C_OPEN        1
#define I2C_CLOSE       0

#define I2C_RETRY_COUNT     10
#define I2C_DEBUG_MSG        0
#define MAX_I2C_CHANNEL      3

#define SEND_ACK        0
#define RECV_ACK        1

#define   HIDAT      1
#define   LODAT      0
#define   HICLK      1
#define   LOCLK      0

#define I2C_COMMAND_NULL         0X0000
#define I2C_COMMAND_READ         0X0001
#define I2C_COMMAND_WRITE        0X0002
#define I2C_COMMAND_STATUS       0X0004
#define I2C_COMMAND_RESET        0X0008

/* The following flags are provided on a READ or WRITE command */
#define I2C_FLAGS_START          0X0001 /* START + addx */
#define I2C_FLAGS_STOP           0X0002 /* STOP */
#define I2C_FLAGS_DATACHAINING   0X0004 /* STOP, START + addx  */
#define I2C_FLAGS_ACK            0X0010 /* ACKNOWLEDGE (normally set) */

/* The following status flags are returned on completion of the operation */
#define I2C_STATUS_NOERROR       0X0000  
#define I2C_STATUS_BUSY          0X0001
#define I2C_STATUS_ERROR         0X0002
#define I2C_MAX_CLOCK_RATE       (100*1000)           /* Hz */

#define ERROR_INVALID_PARAMETER		-1
#define NO_ERROR					0

#define VP_STATUS					char

// n'th bit set as 1
#define BIT(n)  (1 << (n))
// n bits set as 1 from bit(0) to bit(n-1)
#define BITS(n) ((1 << (n)) - 1)
// Select Large one from a:b
#define LARGE(n) ((1?n) > (0?n) ? (1?n) : (0?n))
// Select Small one from a:b
#define SMALL(n) ((1?n) < (0?n) ? (1?n) : (0?n))
// set bits as 1 between bit(a) and bit(b)
//#define BITSMASK(a,b)   ( (a)>(b) ? BITS((a)-(b)+1) << (b) : BITS((b)-(a)+1) << (a) )
// The same as BITMASK(a,b) instead of parameters' format
//#define MASK(n)         BITSMASK(1?n, 0?n)
// set bits as 1 between bit(a) and bit(b)
#define MASK(n)         ( BITS(LARGE(n)-SMALL(n)+1) << SMALL(n) )

#ifndef GETBITS
// get bits [a:b]'s binary value
#define GETBITS(b,n)    ( ((b) & MASK(n)) >> SMALL(n) ) /* Jong@08032009 */
#endif /* GETBITS */
// set binary value from [a:0] to [c:d]
#define SETBITS(b, n)   ( ( (b) << ((1?n) > (0?n) ? (0?n) : (1?n)) ) & MASK(n) )
// move bits value from [a:b] to [c:d]
#define MOVEBITS(b, m, n)   ( GETBITS(b, m) << ((1?n) > (0?n) ? (0?n) : (1?n)) )
/*--------------------------------*/

typedef struct _I2CControl {
        ULONG Command;          /*  I2C_COMMAND_* */
        u32   dwCookie;         /* Context identifier returned on Open */
        u8    Data;             /* Data to write, or returned UCHAR */
        u8    Reserved[3];      /* Filler */
        ULONG Flags;            /*  I2C_FLAGS_* */
        ULONG Status;           /* I2C_STATUS_*  */
        ULONG ClockRate;        /* Bus clockrate in Hz. */
} I2CControl, *PI2CControl;

typedef struct _I2CContext
{
    u32 dwI2CPortAcquired;            /* port busy between start and stop */
    u32 dwCookie;                  /* cookie image for this instance */
    u32 dwCurCookie;                  /* cookie of current I2C channel owner */
} I2C_CONTEXT, *PI2C_CONTEXT;


typedef struct _EDID_V1_  {
    ULONG       ulHeader0;
    ULONG       ulHeader1;
    struct {
            UCHAR       LoUCHAR;
            UCHAR       HiUCHAR;
    } IDManufactureName;
    USHORT      IDProductCode;
    ULONG       IDSerialNumber;
    UCHAR       WeekOfManufacture;
    UCHAR       YearOfManufacture;
    UCHAR       bEDIDVersion;           /* should be 1 */
    UCHAR       bEDIDRevision;          /* should be 0~3 */
    UCHAR       bVideoInput;
    UCHAR       bMaxHzImageSize;        /* cm */
    UCHAR       bMaxVtImageSize;        /* cm */
    UCHAR       bGamma;
    UCHAR       bFeatureSupport;
    UCHAR       bColorCharacteristics[10];
    UCHAR       bEstablishedTiming[3];
    USHORT      usStandardTiming[8];
    union  {
        struct {
            USHORT  usPixelClock;
            UCHAR   bHzActive;
            UCHAR   bHzBlank;
            UCHAR   bHzActiveBlank;
            UCHAR   bVtActive;
            UCHAR   bVtBlank;
            UCHAR   bVtActiveBlank;
            UCHAR   bHSyncOffset;
            UCHAR   bHSyncWidth;
            UCHAR   bVSyncOffsetWidth;
            UCHAR   bHzVtSyncOffsetWidth;
            UCHAR   bHzImageSize;           /* mm */
            UCHAR   bVtImageSize;           /* mm */
            UCHAR   bHzVtImageSize;         /* mm */
            UCHAR   bHzBorder;
            UCHAR   bVtBorder;
            UCHAR   bFlags;
        } DetailedTiming;
        struct {
            USHORT      usReserved0;
            UCHAR       bReserved1;
            UCHAR       bTag;
            UCHAR       bReserved2;
            UCHAR       bData[13];
        } MonitorDescriptor;
    } Descriptor[4];
    UCHAR       bExtensionFlag;
    UCHAR       bChecksum;
} EDID_V1, *PEDID_V1;

typedef struct _XGI_I2C_CONTROL
{
     ULONG                   I2CDelay;        /* 100ns units*/
} XGI_I2C_CONTROL, *PXGI_I2C_CONTROL;

/*extern  char I2CAccessBuffer(PXGI_HW_DEVICE_INFO pHWDE, PI2CControl I2CCntl, ULONG DevAddr, ULONG Offset, PUCHAR pBuffer, ULONG ulSize);
*/
extern  char vGetEDIDExtensionBlocks(PXGI_HW_DEVICE_INFO pHWDE, PI2CControl pI2C, PUCHAR pjBuffer, ULONG ulBufferSize);
extern  char vGetEnhancedEDIDBlock(PXGI_HW_DEVICE_INFO pHWDE, PI2CControl pI2C, ULONG ulBlockID, ULONG ulBlockTag, PUCHAR pjBuffer, ULONG ulBufferSize);


extern  char I2COpen (PXGI_HW_DEVICE_INFO  pHWDE,ULONG ulI2CEnable, ULONG ulChannelID, PI2CControl pI2CControl);
extern  char I2CAccess(PXGI_HW_DEVICE_INFO pHWDE, PI2CControl pI2CControl);
extern  BOOLEAN I2CNull( PXGI_HW_DEVICE_INFO pHWDE,  PI2CControl pI2CControl);
extern  BOOLEAN I2CRead(PXGI_HW_DEVICE_INFO pHWDE, PI2CControl pI2CControl);
extern  BOOLEAN I2CWrite(PXGI_HW_DEVICE_INFO pHWDE,  PI2CControl pI2CControl);
extern  BOOLEAN ResetI2C(PXGI_HW_DEVICE_INFO pHWDE,  PI2CControl pI2CControl);
extern  BOOLEAN I2CRead(PXGI_HW_DEVICE_INFO pHWDE,PI2CControl pI2CControl);
extern  BOOLEAN I2CWrite(PXGI_HW_DEVICE_INFO pHWDE,  PI2CControl pI2CControl);
extern  BOOLEAN ResetI2C(PXGI_HW_DEVICE_INFO pHWDE,  PI2CControl pI2CControl);
extern  BOOLEAN bGetEDID(PXGI_HW_DEVICE_INFO, ULONG , PUCHAR, ULONG);

#endif
