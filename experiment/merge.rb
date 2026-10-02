#!/usr/bin/env ruby

require 'json'

if ARGV.size < 1
  warn "Usage: ruby merge.rb history1.js history2.js [...] > merged.js"
  exit 1
end

INPUTS = ARGV

def load_index(path)
  index = {}
  JSON.parse(File.read(path)).each do |row|
    index[[row['image'], row['flags']]] = row
  end
  index
end

files = INPUTS.map { |p| [p, load_index(p)] }

all_keys = files.flat_map { |_, idx| idx.keys }.uniq
             .sort_by { |img, fl| [img.to_s, fl.to_s] }

def mean(xs)
  return nil if xs.empty?
  xs.sum / xs.size.to_f
end

def half_range(xs)
  return nil if xs.size < 2
  (xs.max - xs.min) / 2.0
end

def pct_of(mean_val, hr)
  return nil if mean_val.nil? || hr.nil? || mean_val.zero?
  (hr / mean_val * 100.0).round(2)
end

result = []

all_keys.each do |key|
  present = files.map { |name, idx| [name, idx[key]] }
                 .reject { |_, row| row.nil? }

  next if present.empty?

  rows = present.map(&:last)

  compile_times = rows.map { |r| r['compile_time'].to_f }
  bench_times   = rows.map { |r| r['bench_time'].to_f }
  sizes         = rows.map { |r| r['binary_size'].to_i }
  sizes_strip   = rows.map { |r| r['binary_size_stripped'].to_i }

  base = rows.first

  entry = {
    'version'              => base['version'],
    'image'                => base['image'],
    'release_date'         => base['release_date'],
    'flags'                => base['flags'],

    'compile_time'         => mean(compile_times).round(4),
    'compile_time_avg'     => mean(compile_times).round(4),
    'compile_time_diff_pct'=> pct_of(mean(compile_times), half_range(compile_times)),

    'bench_time'           => mean(bench_times).round(4),
    'bench_time_avg'       => mean(bench_times).round(4),
    'bench_time_diff_pct'  => pct_of(mean(bench_times), half_range(bench_times)),

    'binary_size'          => mean(sizes).round,
    'binary_size_stripped' => mean(sizes_strip).round,

    'runs'                 => rows.size,
  }

  result << entry
end

puts JSON.pretty_generate(result)
