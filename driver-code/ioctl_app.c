#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "mydriver.h"


char dane[] = "Hello_from_app";
char *readBuff; 
char *writeBuff; 
int main(void) {

	printf("Hello from app!\n");
	
	int fd = open("/dev/my_device_driver", O_RDWR);
	
	if (fd < 0) {	printf("Unable to open driver\n"); 	return 0;	}
	
	printf("Opened the driver!\n");
	
	//-------------------------------------------------------------------------------------------
	
	readBuff = (char*)malloc(50);
	//writeBuff = "2";
		
	//if (write(fd, writeBuff, 4 ) < 0) {  printf("Unable to write to driver\n");	return 0;	}
	
	//printf("Writing IOCTL!\n");	
	
	if (read(fd, readBuff, 50 ) < 0) {  printf("Unable to read from driver\n");	return 0;	}
	
	printf("RECIVED FROM DRIVER: %s\n",readBuff);
	  
  	printf("Setting counter to: 5\n");
	
	my_ioctl_data data ={5};
	
	if (ioctl(fd, MY_IOCTL_IN, &data) < 0) {printf("Unable to handle IOCTL\n"); perror("Error:");	return 0;	}
	
	printf("Written IOCTL\n");
		
	if (read(fd, readBuff, 50 ) < 0) {  printf("Unable to read from driver\n");	return 0;	}
	
	printf("RECIVED FROM DRIVER: %s\n",readBuff);
		
	
    	
   	free(readBuff);
	//-------------------------------------------------------------------------------------------
	printf("Time to close\n");
	
	close(fd);
	
	printf("Closed :) Bye\n");
	return 0;
}
