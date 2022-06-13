#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

int main(){

	char buff[10000];
	while (read(0, buff, 1)  > 1)
		(void )1;
	int fd = open("file.txt", O_RDWR );
	write(fd , buff, 10000);
	printf("STATUS 200 OKasdasd");
	return 0;
}