/*
 * Copyright (c) 2008 Juan Romero Pardines
 * Copyright (c) 2008 Mark Kettenis
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>

#ifdef HAVE_MTRR
#include <machine/sysarch.h>
#include <machine/mtrr.h>
#define netbsd_set_mtrr(mr, num)	_X86_SYSARCH_L(set_mtrr)(mr, num)
#endif

#include <dev/pci/pcidevs.h>
#include <dev/pci/pciio.h>
#include <dev/pci/pcireg.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#include <pci.h>

#include "pciaccess.h"
#include "pciaccess_private.h"

static int pcifd;

static int
pci_read(int bus, int dev, int func, uint32_t reg, uint32_t *val)
{
	uint32_t rval;

	if (pcibus_conf_read(pcifd, bus, dev, func, reg, &rval) == -1)
		return (-1);

	*val = rval;

	return 0;
}

static int
pci_write(int bus, int dev, int func, uint32_t reg, uint32_t val)
{
	return pcibus_conf_write(pcifd, bus, dev, func, reg, val);
}

static int
pci_nfuncs(int bus, int dev)
{
	uint32_t hdr;

	if (pci_read(bus, dev, 0, PCI_BHLC_REG, &hdr) != 0)
		return -1;

	return (PCI_HDRTYPE_MULTIFN(hdr) ? 8 : 1);
}

static int
pci_device_netbsd_map_range(struct pci_device *dev,
    struct pci_device_mapping *map)
{
#ifdef HAVE_MTRR
	struct mtrr m;
	int n = 1;
#endif
	int prot, fd, ret = 0;

	prot = PROT_READ;

	if (map->flags & PCI_DEV_MAP_FLAG_WRITABLE)
		prot |= PROT_WRITE;

	fd = open("/dev/mem", O_RDWR);
	if (fd == -1)
		return errno;
	map->memory = mmap(NULL, map->size, prot, MAP_SHARED, fd, map->base);
	if (map->memory == MAP_FAILED)
		return errno;

#ifdef HAVE_MTRR
	memset(&m, 0, sizeof(m));

	/* No need to set an MTRR if it's the default mode. */
	if ((map->flags & PCI_DEV_MAP_FLAG_CACHABLE) ||
	    (map->flags & PCI_DEV_MAP_FLAG_WRITE_COMBINE)) {
		m.base = base;
		m.flags = MTRR_VALID | MTRR_PRIVATE;
		m.len = size;
		m.owner = getpid();
		if (map->flags & PCI_DEV_MAP_FLAG_CACHEABLE)
			m.type = MTRR_TYPE_WB;
		if (map->flags & PCI_DEV_MAP_FLAG_WRITE_COMBINE)
			m.type = MTRR_TYPE_WC;

		if ((netbsd_set_mtrr(&m, &n)) == -1)
			ret = errno;
	}
#endif

	close(fd);

	return ret;
}

static int
pci_device_netbsd_unmap_range(struct pci_device *dev,
    struct pci_device_mapping *map)
{
#ifdef HAVE_MTRR
	struct mtrr m;
	int n = 1;

	memset(&m, 0, sizeof(m));

	if ((map->flags & PCI_DEV_MAP_FLAG_CACHABLE) ||
	    (map->flags & PCI_DEV_MAP_FLAG_WRITE_COMBINE)) {
		m.base = map->base;
		m.flags = 0;
		m.len = size;
		m.type = MTRR_TYPE_UC;
		(void)netbsd_set_mtrr(&m, &n);
	}
#endif

	return pci_device_generic_unmap_range(dev, map);
}

static int
pci_device_netbsd_read(struct pci_device *dev, void *data,
    pciaddr_t offset, pciaddr_t size, pciaddr_t *bytes_read)
{
	u_int reg, rval;

	*bytes_read = 0;
	while (size > 0) {
		int toread = MIN(size, 4 - (offset & 0x3));

		reg = (offset & ~0x3);

		if ((pcibus_conf_read(pcifd, dev->bus, dev->dev, dev->func,
		    reg, &rval)) == -1)
			return errno;

		rval = htole32(rval);
		rval >>= ((offset & 0x3) * 8);

		memcpy(data, &rval, toread);

		offset += toread;
		data = (char *)data + toread;
		size -= toread;
		*bytes_read += toread;
	}

	return 0;
}

static int
pci_device_netbsd_write(struct pci_device *dev, const void *data,
    pciaddr_t offset, pciaddr_t size, pciaddr_t *bytes_written)
{
	u_int reg, val;

	if ((offset % 4) != 0 || (size % 4) != 0)
		return EINVAL;

	*bytes_written = 0;
	while (size > 0) {
		reg = offset;
		memcpy(&val, data, 4);

		if ((pcibus_conf_write(pcifd, dev->bus, dev->dev, dev->func,
		   reg, val)) == -1)
			return errno;

		offset += 4;
		data = (char *)data + 4;
		size -= 4;
		*bytes_written += 4;
	}

	return 0;
}

static void
pci_system_netbsd_destroy(void)
{
	close(pcifd);
	free(pci_sys);
	pci_sys = NULL;
}

static int
pci_device_netbsd_probe(struct pci_device *device)
{
	struct pci_device_private *priv = (struct pci_device_private *)device;
	struct pci_mem_region *region;
	uint64_t reg64, size64;
	uint32_t bar, reg, size;
	int bus, dev, func, err;

	bus = device->bus;
	dev = device->dev;
	func = device->func;

	err = pci_read(bus, dev, func, PCI_BHLC_REG, &reg);
	if (err)
		return err;

	priv->header_type = PCI_HDRTYPE_TYPE(reg);
	if (priv->header_type != 0)
		return 0;

	region = device->regions;
	for (bar = PCI_MAPREG_START; bar < PCI_MAPREG_END;
	     bar += sizeof(uint32_t), region++) {
		err = pci_read(bus, dev, func, bar, &reg);
		if (err)
			return err;

		/* Probe the size of the region. */
		err = pci_write(bus, dev, func, bar, ~0);
		if (err)
			return err;
		pci_read(bus, dev, func, bar, &size);
		pci_write(bus, dev, func, bar, reg);

		if (PCI_MAPREG_TYPE(reg) == PCI_MAPREG_TYPE_IO) {
			region->is_IO = 1;
			region->base_addr = PCI_MAPREG_IO_ADDR(reg);
			region->size = PCI_MAPREG_IO_SIZE(size);
		} else {
			if (PCI_MAPREG_MEM_PREFETCHABLE(reg))
				region->is_prefetchable = 1;
			switch(PCI_MAPREG_MEM_TYPE(reg)) {
			case PCI_MAPREG_MEM_TYPE_32BIT:
			case PCI_MAPREG_MEM_TYPE_32BIT_1M:
				region->base_addr = PCI_MAPREG_MEM_ADDR(reg);
				region->size = PCI_MAPREG_MEM_SIZE(size);
				break;
			case PCI_MAPREG_MEM_TYPE_64BIT:
				region->is_64 = 1;

				reg64 = reg;
				size64 = size;

				bar += sizeof(uint32_t);

				err = pci_read(bus, dev, func, bar, &reg);
				if (err)
					return err;
				reg64 |= (uint64_t)reg << 32;

				err = pci_write(bus, dev, func, bar, ~0);
				if (err)
					return err;
				pci_read(bus, dev, func, bar, &size);
				pci_write(bus, dev, func, bar, reg64 >> 32);
				size64 |= (uint64_t)size << 32;

				region->base_addr = PCI_MAPREG_MEM64_ADDR(reg64);
				region->size = PCI_MAPREG_MEM64_SIZE(size64);
				region++;
				break;
			}
		}
	}

	return 0;
}

static const struct pci_system_methods netbsd_pci_methods = {
	pci_system_netbsd_destroy,
	NULL,
	NULL,
	pci_device_netbsd_probe,
	pci_device_netbsd_map_range,
	pci_device_netbsd_unmap_range,
	pci_device_netbsd_read,
	pci_device_netbsd_write,
	pci_fill_capabilities_generic
};

int
pci_system_netbsd_create(void)
{
	struct pci_device_private *device;
	int bus, dev, func, ndevs, nfuncs;
	uint32_t reg;

	pcifd = open("/dev/pci0", O_RDWR);
	if (pcifd == -1)
		return ENXIO;

	pci_sys = calloc(1, sizeof(struct pci_system));
	if (pci_sys == NULL) {
		close(pcifd);
		return ENOMEM;
	}

	pci_sys->methods = &netbsd_pci_methods;

	ndevs = 0;
	for (bus = 0; bus < 256; bus++) {
		for (dev = 0; dev < 32; dev++) {
			nfuncs = pci_nfuncs(bus, dev);
			for (func = 0; func < nfuncs; func++) {
				if (pci_read(bus, dev, func, PCI_ID_REG,
				    &reg) != 0)
					continue;
				if (PCI_VENDOR(reg) == PCI_VENDOR_INVALID ||
				    PCI_VENDOR(reg) == 0)
					continue;

				ndevs++;
			}
		}
	}

	pci_sys->num_devices = ndevs;
	pci_sys->devices = calloc(ndevs, sizeof(struct pci_device_private));
	if (pci_sys->devices == NULL) {
		free(pci_sys);
		close(pcifd);
		return ENOMEM;
	}

	device = pci_sys->devices;
	for (bus = 0; bus < 256; bus++) {
		for (dev = 0; dev < 32; dev++) {
			nfuncs = pci_nfuncs(bus, dev);
			for (func = 0; func < nfuncs; func++) {
				if (pci_read(bus, dev, func, PCI_ID_REG,
				    &reg) != 0)
					continue;
				if (PCI_VENDOR(reg) == PCI_VENDOR_INVALID ||
				    PCI_VENDOR(reg) == 0)
					continue;

				device->base.domain = 0;
				device->base.bus = bus;
				device->base.dev = dev;
				device->base.func = func;
				device->base.vendor_id = PCI_VENDOR(reg);
				device->base.device_id = PCI_PRODUCT(reg);

				if (pci_read(bus, dev, func, PCI_CLASS_REG,
				    &reg) != 0)
					continue;

				device->base.device_class =
				    PCI_INTERFACE(reg) | PCI_CLASS(reg) << 16 |
				    PCI_SUBCLASS(reg) << 8;
				device->base.revision = PCI_REVISION(reg);

				if (pci_read(bus, dev, func, PCI_SUBSYS_ID_REG,
				    &reg) != 0)
					continue;

				device->base.subvendor_id = PCI_VENDOR(reg);
				device->base.subdevice_id = PCI_PRODUCT(reg);

				device++;
			}
		}
	}

	return 0;
}
