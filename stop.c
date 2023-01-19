#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "simulator.h"


int main() {
	  // ... ADD SOME VARIABLES HERE ... //
	  int server, len;
	  char command = STOP;
	  struct sockaddr_in serverAddress;
	  socklen_t length = sizeof(serverAddress);

	  // Register with the server
	  // ... WRITE SOME CODE HERE ... //
	  server = socket(AF_INET, SOCK_DGRAM, 0);

	  memset(&serverAddress, 0, sizeof(serverAddress));
	  serverAddress.sin_family = AF_INET;
	  serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
	  serverAddress.sin_port = htons(SERVER_PORT);

	  // Send command string to server
	  // ... WRITE SOME CODE HERE ... //
	  len = sendto(server, &command, 1, 0, (struct sockaddr*)&serverAddress, length);

	  close(server);

	  printf("server closed\n");

}
