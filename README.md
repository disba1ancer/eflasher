# eflasher
KB930 flash read/write tool

Tool for reading and writing firmware to flash connected to embedded controller (EC) ENE KB930 through Index I/O ports.

Currently reading/writing supported only for last sector (last 4K bytes), and used next syntax:

* `eflasher rdmsr` - read status register of flash connected to EC
* `eflasher read <file>` - read last sector to file
* `eflasher write <file>` - write last sector from file
