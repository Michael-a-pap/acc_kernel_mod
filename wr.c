#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#define DEVICE_PATH "/dev/fpga_accelerator"


int main(void) 
{
	int fd = open(DEVICE_PATH, O_WRONLY);
//	const char message[] = { 'H', 'e', 'l', 'l', 'o', 't', 'e', 's', 't' };
//	const char message[5] = { 'H', 'e', 'l', 'l', 'o' };
	const char *message ={"Hello"};

	if (fd < 0){
		printf("Failed to open the device");
		return 1;
	}
	
	//int bytes_written = write(fd, message, strnlen(message, sizeof(message)));
	
	int bytes_written = write(fd, message, strlen(message) + 1);

	if (bytes_written < 0){
		printf("Failed to write to the device");
		close(fd);
		return 1;
	}
	
	printf("Wrote %zd bytes to %s\n", bytes_written, DEVICE_PATH);

	return 0;
}
