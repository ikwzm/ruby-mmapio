#!/usr/bin/env ruby
# SPDX-License-Identifier: BSD-2-Clause
# Copyright (c) 2025 Ichiro Kawazome

require_relative './system'
require_relative '../memory_mapped_io'

module MMapIO

  class Uio

    def self.find_device_by_name(name)
      device_name = nil
      r = Regexp.compile('/sys/class/uio/(uio\d+)/name')
      Dir.glob("/sys/class/uio/uio*/name") do | uio_name_file |
        File.open(uio_name_file, "r") do |file|
          uio_name = file.gets.chomp
          if uio_name == name then
            m = r.match(uio_name_file)
            device_name = m[1]
          end
        end
      end
      return device_name
    end

    attr_reader :device_file
    attr_reader :device_name
    attr_reader :name

    def initialize(_name=nil, name:nil, device_name: nil)
      if    not device_name.nil? then
        @device_name = device_name
        @name        = read_class_attribute("name")
      else
        @name        = name || _name
        if @name.nil? then
          raise ArgumentError, "name or device_name is required"
        end
        @device_name = Uio.find_device_by_name(@name)
        if @device_name.nil? then
          raise RuntimeError , "Not found device by name(=#{@name})"
        end
      end
      file_name = File.join("", "dev", @device_name)
      @device_file = open(file_name, "rb+")
      @memmap_dict = {}
    end
    
    def class_attribute_file_name(name)
      return File.join("", "sys", "class", "uio", @device_name , name)
    end

    def read_class_attribute(name)
      file_name = class_attribute_file_name(name)
      attribute = nil
      File.open(file_name, "r") do |file|
        attribute = file.gets.chomp
      end
      return attribute
    end

    def read_class_integer(name)
      file_name = class_attribute_file_name(name)
      File.open(file_name, "r") do |file|
        value = file.gets.chomp
        if value.match?(/^0[xX][0-9a-fA-F]+$/)
          return value.hex
        end
        if value.match?(/^[0-9]+$/)
          return value.to_i
        end
        raise RuntimeError, "Invalid value (file_name=#{file_name},value=#{value})"
      end
    end

    def get_map_addr(index=0)
      return read_class_integer(File.join("maps", "map#{index}", "addr"))
    end
    
    def get_map_size(index=0)
      return read_class_integer(File.join("maps", "map#{index}", "size"))
    end
    
    def get_map_offset(index=0)
      return read_class_integer(File.join("maps", "map#{index}", "offset"))
    end

    def get_map_info(index=0)
      map_addr   = get_map_addr(index)
      map_size   = get_map_size(index)
      map_offset = get_map_offset(index)
      return {addr: map_addr, size: map_size, offset: map_offset}
    end
    
    def irq_on()
      @device_file.write([1,0,0,0].pack("C*"))
    end

    def irq_off()
      @device_file.write([0,0,0,0].pack("C*"))
    end

    def wait_irq(timeout:nil)
      ready, _, _ = IO.select([@device_file], nil, nil, timeout)
      puts("#{ready}")
      if ready.nil? then
        return nil
      else
        bytes = @device_file.read(4)
        count = bytes.unpack('V').first
        return count
      end
    end

    def regs(offset=0, length:nil, index:0)
      if @memmap_dict.has_key?(index) then
        memmap_info = @memmap_dict[index]
        memmap      = memmap_info[:memmap]
        mmap_offset = memmap_info[:offset]
        mmap_size   = memmap_info[:size]
        mmap_addr   = memmap_info[:addr]
      else
        page_size   = System.getpagesize
        map_info    = get_map_info(index)
        mmap_addr   = map_info[:addr]
        mmap_offset = ((map_info[:addr] + map_info[:offset])           ) % page_size
        mmap_size   = ((map_info[:size] + page_size - 1).div(page_size)) * page_size
        memmap      = System.mmap(nil, 
                                  mmap_size,
                                  System::PROT_READ | System::PROT_WRITE,
                                  System::MAP_SHARED,
                                  @device_file.fileno,
                                  index*page_size)
        memmap_info = {memmap: memmap, size: mmap_size, addr: mmap_addr, offset: mmap_offset}
        ## puts(memmap_info)
        @memmap_dict[index] = memmap_info
      end
      regs_offset = mmap_offset + offset
      if    length.nil? then
        regs_length = mmap_size - regs_offset
      elsif regs_offset + length <= mmap_size then
        regs_length = length
      else
        raise RuntimeError, "region range error (index=#{index},offset=#{offset},length=#{length})"
      end
      return MMapIO::MemoryMappedIO.new(memmap, regs_offset, regs_length)
    end
  end
end

