#!/usr/bin/env ruby
# SPDX-License-Identifier: BSD-2-Clause
# Copyright (c) 2025 Ichiro Kawazome

require 'fiddle'
require 'fiddle/import'

module MMapIO
  module System
    extend Fiddle::Importer
    dlload Fiddle::dlopen(nil)
    extern 'int getpagesize()'
    extern 'void* mmap(void*, size_t, int, int, int, long)'
    extern 'int munmap(void*, size_t)'
    PROT_READ  = 0x1
    PROT_WRITE = 0x2
    MAP_SHARED = 0x1
  end
end
