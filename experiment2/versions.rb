#!/usr/bin/env ruby

TOOLS = {
  "clang"  => ["clang", "--version"],
  "gcc"    => ["gcc", "--version"],
  "tcc"    => ["tcc", "--version"],
  "cproc"  => ["cproc", "--version"],
  "mycc"   => ["mycc", "--version"],
  "antcc"  => ["antcc", "--version"],
  "madc"   => ["madc", "--version"],
  "zig"   => ["zig", "version"]
}.freeze

def executable?(name)
  ENV.fetch("PATH", "").split(File::PATH_SEPARATOR).any? do |dir|
    File.executable?(File.join(dir, name))
  end
end

TOOLS.each do |name, command|
  print "%-10s: " % name

  unless executable?(command.first)
    puts "not found"
    next
  end

  output = IO.popen(command, err: [:child, :out], &:read)
  version = output.lines.first&.strip

  puts version.nil? || version.empty? ? "unknown version" : version
rescue StandardError => e
  puts "error: #{e.message}"
end
