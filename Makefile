# Copyright 2019 SiFive, Inc #
# SPDX-License-Identifier: Apache-2.0 #

PROGRAM ?= bin-loader

$(PROGRAM): $(wildcard *.c) $(wildcard *.h) $(wildcard *.S) $(wildcard myprogram.bin)
	$(CC) $(CFLAGS) $(LDFLAGS) $(filter %.c %.S,$^) $(LOADLIBES) $(LDLIBS) -o $@

clean:
	rm -f $(PROGRAM) $(PROGRAM).hex
