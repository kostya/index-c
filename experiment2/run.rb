require "fileutils"
require "open3"

SOURCE = "../index.c"
LIBS = ["-lm", "-lpthread"]

def measure
  start = Process.clock_gettime(Process::CLOCK_MONOTONIC)
  result = yield
  elapsed = Process.clock_gettime(Process::CLOCK_MONOTONIC) - start
  [elapsed, result]
end

def parse_summary(output)
  m = output.match(/Summary:\s*([\d.]+)s/)
  m ? m[1].to_f : nil
end

def run_and_check(binary)
  unless File.executable?(binary)
    return [nil, false]
  end

  elapsed, result = measure do
    Open3.capture3(binary)
  end

  stdout, stderr, status = result
  output = stdout + stderr

  ok = status.success? && output.include?("50, 50, 0")
  runtime = parse_summary(output)

  [runtime || elapsed, ok]
end

def compile_and_run(name, backend, flags, command, binary)
  flag_label = flags.empty? ? "(default)" : flags.join(" ")

  printf "%-10s %-6s %-20s ... ", name, backend, flag_label

  FileUtils.rm_f(binary)

  compile_time, result = measure do
    Open3.capture3(*command)
  end

  stdout, stderr, status = result

  unless status.success? && File.executable?(binary)
    puts "COMPILE ERR"

    error = (stderr + stdout).strip
    puts "  #{error.lines.last(5).join}" unless error.empty?

    return [name, backend, flag_label, compile_time, nil, "COMPILE ERR"]
  end

  runtime, ok = run_and_check(binary)

  status_label = ok ? "OK" : "ERR"
  puts "#{status_label} compile=#{compile_time.round(4)}s run=#{runtime&.round(4)}s"

  [
    name,
    backend,
    flag_label,
    compile_time,
    runtime,
    status_label
  ]
rescue StandardError => e
  puts "ERROR: #{e.message}"
  [name, backend, flag_label, nil, nil, "ERROR"]
ensure
  FileUtils.rm_f(binary)
end

def c_run(name, command_prefix, flags, backend = "-")
  binary = "./bin_index_#{name}_#{flags.join('_').gsub(/[^a-zA-Z0-9]/, '_')}"

  command = command_prefix + flags + [
    SOURCE,
    *LIBS,
    "-o", binary
  ]

  compile_and_run(name, backend, flags, command, binary)
end

def myc_run(backend, flags)
  name = "mycc"
  flag_label = flags.empty? ? "default" : flags.join("_")

  binary = "./bin_index_myc_#{backend}_#{flag_label.gsub(/[^a-zA-Z0-9]/, '_')}"

  command = [
    "mycc",
    SOURCE,
    "c",
    "--backend", backend,
    *flags,
    binary
  ]

  compile_and_run(name, backend, flags, command, binary)
end

MATRIX = [
  {
    name: "gcc",
    command: ["gcc"],
    flags: %w[-O0 -O1 -O2 -O3]
  },

  {
    name: "clang",
    command: ["clang"],
    flags: %w[-O0 -O1 -O2 -O3],
    backend: 'LLVM',
  },

  {
    name: "zig-cc",
    command: ["zig", "cc"],
    flags: %w[-O0 -O3],
    backend: 'LLVM',
  },

  {
    name: "tcc",
    command: ["tcc"],
    flags: [""]
  },

  {
    name: "cproc",
    command: ["cproc"],
    flags: [""],
    backend: "QBE"
  },

  {
    name: "antcc",
    command: ["antcc"],
    flags: %w[-O0 -O3]
  },

  {
    name: "madc",
    command: ["madc"],
    flags: %w[-O0 -O1 -O2],
    backend: 'MIR',
  },

  {
    name: "mucc",
    command: ["mucc"],
    flags: %w[-O0],
  }
]

MYC_BACKENDS = {
  "LLVM" => [[], ["--final"]],
  "C"    => [[], ["--final"]],
  "GM"   => [[], ["--final"]],
  "QBE"  => [[]]
}

results = []

MATRIX.each do |compiler|
  name = compiler[:name]
  command = compiler[:command]

  flags_list = compiler[:flags]

  flags_list.each do |flag|
    flags = flag.empty? ? [] : [flag]
    results << c_run(
      name,
      command,
      flags,
      compiler[:backend] || "-"
    )
  end

  puts
end

MYC_BACKENDS.each do |backend, flag_sets|
  flag_sets.each do |flags|
    results << myc_run(backend, flags)
  end
end

puts "| Compiler | Backend | Flags | Compile (s) | Run (s) | Status |"
puts "|----------|---------|-------|-------------|---------|--------|"

results.each do |name, backend, flags, compile_time, runtime, status|
  ct = compile_time ? compile_time.round(4) : ""
  rt = runtime ? runtime.round(4) : ""

  puts "| #{name} | #{backend} | #{flags} | #{ct} | #{rt} | #{status} |"
end

puts
puts "Total configurations: #{results.size}"
puts "Successful: #{results.count { |r| r[5] == 'OK' }}"
puts "Failed: #{results.count { |r| r[5] != 'OK' }}"
