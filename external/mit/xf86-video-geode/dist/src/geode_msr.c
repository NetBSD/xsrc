/*
 * Copyright (c) 2008 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Neither the name of the Advanced Micro Devices, Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>
#include "os.h"
#include "geode.h"

static int
_msr_open(void)
{
    static int msrfd = 0;

    if (msrfd == 0) {
        msrfd = open("/dev/cpu/0/msr", O_RDWR);
        if (msrfd == -1)
            ErrorF("Unable to open /dev/cpu/0/msr: %d\n", errno);
    }

    return msrfd;
}

int
GeodeReadMSR(unsigned long addr, unsigned long *lo, unsigned long *hi)
{
    unsigned int data[2];
    int fd = _msr_open();
    int ret;

    if (fd == -1)
        return -1;

    ret = lseek(fd, (off_t) addr, SEEK_SET);

    if (ret == -1)
        return -1;

    ret = read(fd, (void *) data, sizeof(data));

    if (ret != 8)
        return -1;

    *hi = data[1];
    *lo = data[0];

    return 0;
}

int
GeodeWriteMSR(unsigned long addr, unsigned long lo, unsigned long hi)
{
    unsigned int data[2];
    int fd = _msr_open();

    if (fd == -1)
        return -1;

    if (lseek(fd, (off_t) addr, SEEK_SET) == -1)
        return -1;

    data[0] = lo;
    data[1] = hi;

    if (write(fd, (void *) data, 8) != 8)
        return -1;

    return 0;
}
