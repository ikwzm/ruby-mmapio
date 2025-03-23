#!/usr/bin/env ruby
# SPDX-License-Identifier: BSD-2-Clause
# Copyright (c) 2025 Ichiro Kawazome

require 'mkmf'

extension_name = 'memory_mapped_io'
dir_config(extension_name)
create_makefile(extension_name)
