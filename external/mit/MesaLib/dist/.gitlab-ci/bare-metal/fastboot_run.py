#!/usr/bin/env python3
#
# Copyright © 2020 Google LLC
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

import argparse
import os
import re
from serial_buffer import SerialBuffer
import sys
import threading

class FastbootRun:
    def __init__(self, args):
        self.powerup = args.powerup
        # We would like something like a 1 minute timeout, but the piglit traces
        # jobs stall out for long periods of time.
        self.ser = SerialBuffer(args.dev, "results/serial-output.txt", "R SERIAL> ", timeout=600)
        self.fastboot="fastboot boot -s {ser} artifacts/fastboot.img".format(ser=args.fbserial)

    def print_error(self, message):
        RED = '\033[0;31m'
        NO_COLOR = '\033[0m'
        print(RED + message + NO_COLOR)

    def logged_system(self, cmd):
        print("Running '{}'".format(cmd))
        return os.system(cmd)

    def run(self):
        if self.logged_system(self.powerup) != 0:
            return 1

        fastboot_ready = False
        for line in self.ser.lines():
            if re.search("fastboot: processing commands", line) or \
                re.search("Listening for fastboot command on", line):
                fastboot_ready = True
                break

            if re.search("data abort", line):
                self.print_error("Detected crash during boot, restarting run...")
                return 2

        if not fastboot_ready:
            self.print_error("Failed to get to fastboot prompt, restarting run...")
            return 2

        if self.logged_system(self.fastboot) != 0:
            return 1

        for line in self.ser.lines():
            if re.search("---. end Kernel panic", line):
                return 1

            # The db820c boards intermittently reboot.  Just restart the run
            # when if we see a reboot after we got past fastboot.
            if re.search("PON REASON", line):
                self.print_error("Detected spontaneous reboot, restarting run...")
                return 2

            # db820c sometimes wedges around iommu fault recovery
            if re.search("watchdog: BUG: soft lockup - CPU.* stuck", line):
                self.print_error(
                    "Detected kernel soft lockup, restarting run...")
                return 2

            # If the network device dies, it's probably not graphics's fault, just try again.
            if re.search("NETDEV WATCHDOG", line):
                self.print_error(
                    "Detected network device failure, restarting run...")
                return 2

            result = re.search("hwci: mesa: (\S*)", line)
            if result:
                if result.group(1) == "pass":
                    return 0
                else:
                    return 1

        self.print_error("Reached the end of the CPU serial log without finding a result, restarting run...")
        return 2

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--dev', type=str, help='Serial device (otherwise reading from serial-output.txt)')
    parser.add_argument('--powerup', type=str, help='shell command for rebooting', required=True)
    parser.add_argument('--powerdown', type=str, help='shell command for powering off', required=True)
    parser.add_argument('--fbserial', type=str, help='fastboot serial number of the board', required=True)
    args = parser.parse_args()

    fastboot = FastbootRun(args)

    while True:
        retval = fastboot.run()
        if retval != 2:
            break

        fastboot = FastbootRun(args)

    fastboot.logged_system(args.powerdown)

    sys.exit(retval)

if __name__ == '__main__':
    main()
