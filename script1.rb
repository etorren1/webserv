#!/usr/bin/ruby

print "Status: 200\r\n"
print "Content-Type: text/plain\r\n"
puts "Environment variables:"
ENV.each do |key, value|
  puts "#{key}: #{value}"
end

puts; puts "Body:"
puts STDIN.read