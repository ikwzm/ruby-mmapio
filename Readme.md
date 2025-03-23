ruby-mmapio
==================================================================================

Overview
----------------------------------------------------------------------------------

### Introduction of ruby-mmapio

A Ruby extension package written in C for low-level memory-mapped I/O accesses.

The high-level nature of the ruby interpreter often creates inconveniences when you want to do low-level memory-mapped I/O accesses.
For example, if you try to read a 32-bit word using the read method of the mmap module in arm64 architecture, the cpu cache gets in the way and does not access the word as expected.

ruby-mmapio solves these problems by being written in C.

License
----------------------------------------------------------------------------------

This project is licensed under the BSD 2-Clause License. See the [LICENSE](./LICENSE) file for details.
