#!/usr/bin/ruby


require 'cgi'
require 'pp'


DOWNLOAD_DIRECTORY = "/Users/#{`whoami`.strip}/sand/webserv/site/downloads"


def upload_file(file)
  server_file = File.join(
    DOWNLOAD_DIRECTORY,
    file.original_filename
  )
  File.open(server_file.untaint, 'w') do |f|
    f << file.read
  end
end


def send_debug_info(cgi)
  env_vars = ENV.map{ |k, v| "#{k}: #{v}" }.join("\n")
  cgi_object = cgi.pretty_inspect
  cgi.out("text/plain") { [env_vars, cgi_object].join("\n\n") }
  exit 0
end


cgi = CGI.new
params = cgi.params


# DEBUG INFO in browser
send_debug_info(cgi)

if params.has_key?("file")
  upload_file(params["file"].first)
end


html = <<~END
  <!DOCTYPE html>
    <html lang="en">
    <head>
      <meta charset="UTF-8">
      <meta http-equiv="X-UA-Compatible"
            content="IE=edge">
      <meta name="viewport"
            content="width=device-width, initial-scale=1.0">
      <title></title>
    </head>
    <body>
      <h1>Upload successful!</h1>
    </body>
  </html>
END

cgi.out("text/html") { html }


# print "Status: 200\r\n"
# print "Content-Type: text/plain\r\n"

# puts ENV.map { |k, v| "#{k}: #{v}" }.join("\n")

# body = STDIN.read
# puts body
