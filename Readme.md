ruby-mmapio
==================================================================================

Overview
----------------------------------------------------------------------------------

### Introduction of ruby-mmapio

A Ruby extension package written in C for low-level memory-mapped I/O accesses.

The high-level nature of the ruby interpreter often creates inconveniences when you want to do low-level memory-mapped I/O accesses.
For example, if you try to read a 32-bit word using the read method of the mmap module in arm64 architecture, the cpu cache gets in the way and does not access the word as expected.

ruby-mmapio solves these problems by being written in C.

Installation
----------------------------------------------------------------------------------

### From source:

#### 1. Install required tools:

 * GCC or Clang compiler
 * Ruby development headers (e.g., `ruby-dev` on Linux)

#### 2. Clone the repository:

```console
shell$ git clone https://github.com/ikwzm/ruby-mmapio.git
shell$ cd ruby-mmapio
```

#### 3. Build mmapio gem

```console
shell$ gem build mmapio.gemspec
  Successfully built RubyGem
  Name: mmapio
  Version: 0.1.0
  File: mmapio-0.1.0.gem
```

#### 4. Install mmapio gem

```console
shell$ sudo gem install mmapio-0.1.0.gem 
Building native extensions. This could take a while...
Successfully installed mmapio-0.1.0
Parsing documentation for mmapio-0.1.0
Done installing documentation for mmapio after 2 seconds
1 gem installed
```

Usage
----------------------------------------------------------------------------------

### Importing the package

```ruby
require 'mmapio'
```

### Example 1: Using MMapIO::MemoryMappedIO

```console
shell# irb
irb(main):001:0> require 'mmapio'
=> true
irb(main):002:0> device_file = open('/dev/uio0', "rb+")
=> #<File:/dev/uio0>
irb(main):003:0> device_mmap = MMapIO::System.mmap(nil, 0x1000, MMapIO::System::PROT_READ | MMapIO::System::PROT_WRITE, MMapIO::System::MAP_SHARED, device_file.fileno, 0)
=> #<Fiddle::Pointer:0x0000002ae2f92dd0 ptr=0x0000003fbb30a000 size=0 free...
irb(main):004:0> regs = MMapIO::MemoryMappedIO.new(device_mmap, 0x0000, 0x1000)
=> #<MMapIO::MemoryMappedIO:0x0000002ae2fbda80>
irb(main):005:0> puts format("0x%08X", regs.read_word(0x00))
0x00020064
=> nil
irb(main):006:0> regs.write_word(0x64,0xDEADBEAF)
=> nil
irb(main):007:0> puts format("0x%08X", regs.read_word(0x64))
0xDEADBEAF
=> nil
irb(main):008:0> exit
```

### Example 2: Using MMapIO::Uio 

```console
shell# irb
irb(main):001:0> require 'mmapio'
=> true
irb(main):002:0> uio = MMapIO::Uio.new('dma-controller@60010000')
=> 
#<MMapIO::Uio:0x0000002aae990768
...
irb(main):003:0> regs = uio.regs()
=> #<MMapIO::MemoryMappedIO:0x0000002aae9ab248>
irb(main):004:0> regs.write_word(0x64,0xDEADBEAF)
=> nil
irb(main):005:0> puts format("0x%08X", regs.read_word(0x64))
0xDEADBEAF
=> nil
irb(main):006:0> exit
```

License
----------------------------------------------------------------------------------

This project is licensed under the BSD 2-Clause License. See the [LICENSE](./LICENSE) file for details.
