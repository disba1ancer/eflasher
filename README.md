# eflasher
KB930 flash read/write tool

Tool for reading and writing firmware to flash connected to embedded controller (EC) ENE KB930 through Index I/O ports.

Syntax:

* `eflasher [-p indexIOPort] -r <file> [address [size]]` - read to file
* `eflasher [-p indexIOPort] [-s sectorSizePow] -w <file> [address [size]]` - write from file
