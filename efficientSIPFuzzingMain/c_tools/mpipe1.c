#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXN 1024

int main(void){
	FILE *read_fp;
	char buffer[MAXN+1];
	int chars_read;
	memset(buffer, '\0', sizeof(buffer));
	read_fp = popen("/home/jinguodong/open_project/opensips_1_10/opensips >/dev/tty 2>/dev/null", "r");
	if(read_fp != NULL){
		chars_read = fread(buffer, sizeof(char), MAXN, read_fp);
		while(chars_read > 0){
			buffer[chars_read - 1] = '\0';
			printf("Output was:-\n%s\n", buffer);
			chars_read = fread(buffer, sizeof(char), MAXN, read_fp);
		}
		pclose(read_fp);
		exit(EXIT_SUCCESS);
	}
	exit(EXIT_FAILURE);

	return 0;
}

