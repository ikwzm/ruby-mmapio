require_relative "lib/mmapio/version"

Gem::Specification.new do |spec|
  spec.name          = "mmapio"
  spec.version       = MMapIO::VERSION
  spec.summary       = "Memory Mapped IO Ruby gem"
  spec.authors       = "Ichiro Kawazome"
  spec.email         = "ichiro_k@ca2.so-net.ne.jp"
  spec.homepage      = "https://github.com/ikwzm/ruby-mmapio"
  spec.license       = "BSD-2-Clause"
  spec.files         = Dir["{lib,ext}/**/*", "LICENSE"]
  spec.require_paths = ["lib"]
  spec.extensions    = ["ext/memory_mapped_io/extconf.rb"]
end
