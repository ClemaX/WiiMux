#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

#define PORT 11000
#define IP_ADDR "127.0.0.1"
#define DELAY 1000

int main(int argc, char *argv[])
{
	int client_s;
	struct sockaddr_in server_addr;
	char send_str[8];
	if ((client_s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("\n Socket creation error \n");
		return -1;
	}
	int flags = fcntl     (client_s, F_GETFL, 0);
	fcntl(client_s, F_SETFL, flags | O_NONBLOCK);

	server_addr.sin_family = AF_INET;     // Address family to use
	server_addr.sin_port = htons(PORT);     // Port num to use
	server_addr.sin_addr.s_addr = inet_addr(IP_ADDR);     // IP address to use

	struct timespec ts;
	ts.tv_sec = DELAY / 1000;
	ts.tv_nsec = (DELAY % 1000) * 1000000;

	while (1) {
		sprintf(send_str, "%d,%d", 1024, 1024);
		sendto(client_s, send_str, strlen(send_str)+1, 0,
		       (struct sockaddr *)&server_addr, sizeof(server_addr));
		nanosleep(&ts, NULL);
	}
}
