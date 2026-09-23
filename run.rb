#!/usr/bin/env ruby

require 'open3'
require "json"

COMPILERS = {
  'gcc:4.9'                => ['2014-04-22'],
  'gcc:5'                  => ['2015-04-22'],
  'gcc:6'                  => ['2016-04-27'],
  'gcc:7'                  => ['2017-05-02'],
  'gcc:8'                  => ['2018-05-02'],
  'gcc:9'                  => ['2019-05-03'],
  'gcc:10'                 => ['2020-05-07'],
  'gcc:12'                 => ['2022-05-06'],
  'gcc:13'                 => ['2023-04-26'],
  'gcc:14'                 => ['2024-05-07'],
  'gcc:15'                 => ['2025-04-25'],
  'gcc:16'                 => ['2026-04-30'],

  'kunitoki/clang-3'       => ['2016-09-01'],
  'kunitoki/clang-4'       => ['2017-03-13'],
  'kunitoki/clang-5'       => ['2017-09-07'],
  'kunitoki/clang-6'       => ['2018-03-08'],
  'kunitoki/clang-7'       => ['2018-09-19'],
  'kunitoki/clang-8'       => ['2019-03-20'],
  'kunitoki/clang-9'       => ['2019-09-19'],
  'kunitoki/clang-10'      => ['2020-03-24'],
  'silkeh/clang:11'        => ['2020-10-12'],
  'silkeh/clang:12'        => ['2021-04-14'],
  'silkeh/clang:13'        => ['2021-10-04'],
  'silkeh/clang:14'        => ['2022-03-25'],
  'silkeh/clang:15-bullseye' => ['2022-09-06'],
  'silkeh/clang:16-bullseye' => ['2023-03-17'],
  'silkeh/clang:17-bullseye' => ['2023-09-09'],
  'silkeh/clang:18-bullseye' => ['2024-03-08'],
  'silkeh/clang:19-bullseye' => ['2024-09-17'],
  'silkeh/clang:20-bullseye' => ['2025-03-04'],
  'silkeh/clang:21'        => ['2025-08-26'],
  'silkeh/clang:22'        => ['2026-02-24'],
  'teeks99/clang-ubuntu' => ['2026-08-25'],
}

OPT_FLAGS = %w[-O0 -O1 -O2 -O3].freeze
SRC       = 'index.c'

def clean_version(raw)
  return '' if raw.nil? || raw.empty?

  s = raw.strip.lines.first.to_s.strip

  if s =~ /gcc\s*\(.*?\)\s*([\d.]+)/i
    "GCC #{$1}"
  elsif s =~ /clang\s+version\s+([\d.]+)/i
    "Clang #{$1}"
  elsif s =~ /gcc\s+([\d.]+)/i
    "GCC #{$1}"
  else
    s
  end
end

def run_one(image, flags)
  cc = image.include?('gcc') ? 'gcc' : 'clang'

  cmd = <<~SH
    cd /app
    rm -f /tmp/index
    echo versionstart
    #{cc} --version | head -1
    echo versionend
    echo compilestart
    { time #{cc} -Wno-format -std=gnu11 #{flags} #{SRC} -lm -lpthread -o /tmp/index ; } 2>&1
    echo compileend
    /tmp/index
  SH

  stdout, stderr, status = Open3.capture3(
    'docker', 'run', '--rm',
    '-v', "#{Dir.pwd}:/app", '-w', '/app',
    image, 'bash', '-c', cmd
  )

  unless status.success?
    warn "[FAIL] #{image} #{flags}"
    warn stderr.lines.last(10).join
    warn stdout.lines.last(10).join
    return nil
  end

  version = stdout[/versionstart\n(.*?)\nversionend/m, 1].to_s.strip

  block = stdout[/compilestart\n(.*?)\ncompileend/m, 1]
  return nil unless block

  compile_sec = block[/real\s+(\d+)m([\d.]+)s/, 1].to_f * 60 +
                block[/real\s+(\d+)m([\d.]+)s/, 2].to_f

  summary_line = stdout.lines.grep(/^Summary:/).last
  return nil unless summary_line

  m = summary_line.match(/Summary:\s*([\d.]+)s,\s*(\d+),\s*(\d+),\s*(\d+)/)
  return nil unless m

  if m[2].to_i != m[3].to_i && m[2].to_i != 50
    puts "Warning errors: #{[m[2].to_i, m[3].to_i, m[4].to_i].inspect}"
    return nil
  end

  {
    version: clean_version(version),
    compile_time: compile_sec,
    bench_time: m[1].to_f,
  }
end

array = []

COMPILERS.each do |image, (date)|
  OPT_FLAGS.each do |flags|
    print "#{image}: #{flags} ... "
    res = run_one(image, flags)
    unless res
      puts "[ERROR]"
      next
    else
      puts "[OK] (version=#{res[:version]}, compile=#{res[:compile_time].round(2)}s, runtime=#{res[:bench_time].round(2)}s)"
    end

    res[:image] = image
    res[:release_date] = date
    res[:flags] = flags

    array << res
    File.open("./history.js", "w") { |f| f.puts(array.to_json) }
  end
end

File.open("./history.js", "w") { |f| f.puts(array.to_json) }
