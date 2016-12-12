#ifndef NV_DMA_H
#define NV_DMA_H

#define NVDEBUG if (NV_DMA_DEBUG) ErrorF

enum DMAObjects {
	NvNullObject		= 0x00000000,
	NvContextSurfaces	= 0x80000010, 
	NvRop			= 0x80000011, 
	NvImagePattern		= 0x80000012, 
	NvClipRectangle		= 0x80000013, 
	NvSolidLine		= 0x80000014, 
	NvImageBlit		= 0x80000015, 
	NvRectangle		= 0x80000016, 
	NvScaledImage		= 0x80000017, 
	NvMemFormat		= 0x80000018,
	Nv3D			= 0x80000019,
	NvImageFromCpu		= 0x8000001A,
	NvContextBeta1		= 0x8000001B,
	NvContextBeta4		= 0x8000001C,
	Nv2D			= 0x80000020,
	NvSW			= 0x80000021,
	NvDmaFB			= 0xbeef0201,
	NvDmaTT			= 0xbeef0202,
	NvDmaNotifier0		= 0xD8000003,
	NvVBlankSem		= 0xD8000004,
};

#endif /* NV_DMA_H */
