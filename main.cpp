#include "Request.hpp"
#include <iostream>

int main() {
    Request req;
    req.parseText("GET /index.html HTTP/1.1\nHost: localhost:8080\nConnection: keep-alive\nCache-Control: max-age=0\nsec-ch-ua: \" Not A;Brand\";v=\"99\", \"Chromium\";v=\"101\", \"Google Chrome\";v=\"101\"\nsec-ch-ua-mobile: ?0\nsec-ch-ua-platform: \"macOS\"\nUpgrade-Insecure-Requests: 1\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_14_6) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/101.0.4951.64 Safari/537.36\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\nSec-Fetch-Site: none\nSec-Fetch-Mode: navigate\nSec-Fetch-User: ?1\nSec-Fetch-Dest: document\nAccept-Encoding: gzip, deflate, br\nAccept-Language: ru-RU,ru;q=0.9,en-US;q=0.8,en;q=0.7,tt;q=0.6\n\n<body><h2>\n</h2></body>\n");

}