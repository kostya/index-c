#!/usr/bin/env ruby

TOOLS = {
  "clang"  => { command: ["clang", "--version"],       url: "https://clang.llvm.org" },
  "gcc"    => { command: ["gcc", "--version"],         url: "https://gcc.gnu.org" },
  "circle" => { command: ["circle", "--version"],      url: "https://github.com/seanbaxter/circle" },

  "tcc"    => { command: "tcc --version", url: "" },
  "cproc"  => { command: "cproc --version", url: "" },
  "mycc"   => { command: "mycc --version", url: "" },
  "antcc"  => { command: "antcc --version", url: "" },
  "madc"   => { command "madc --version", url: "" },
  "zig-cc"   => { command: "zig version", url: "" },

}.freeze

def executable?(name)
  ENV.fetch("PATH", "").split(File::PATH_SEPARATOR).any? do |dir|
    File.executable?(File.join(dir, name))
  end
end

TOOLS.each do |name, info|
  command = info[:command]
  url     = info[:url]

  unless executable?(command.first)
    puts "* [#{name}](#{url}): `not found`"
    next
  end

  output  = IO.popen(command, err: [:child, :out], &:read)
  version = output.lines.first&.strip
  version = "unknown version" if version.nil? || version.empty?

  puts "* [#{name}](#{url}): `#{version}`"
rescue StandardError => e
  puts "* [#{name}](#{url}): `error: #{e.message}`"
end