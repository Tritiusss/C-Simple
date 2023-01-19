#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "simulator.h"

typedef union {
  char data[20];
 	struct{
 		char reqRep;  	// Request/Response
 		char id;	// Robot ID
 		Robot robot;
 	};
}RobotBuffer;

// This is the main function that simulates the "life" of the robot
// The code will exit whenever the robot fails to communicate with the server
int main() {
	// ... ADD SOME VARIABLE HERE ... //
	int client, len;
	struct sockaddr_in serverAddress;
	socklen_t length = sizeof(serverAddress);
	Robot robot;
	RobotBuffer buffer;

	// Set up the random seed
	srand(time(NULL));

	// Register with the server
	client = socket(AF_INET, SOCK_DGRAM, 0);
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
	serverAddress.sin_port = htons(SERVER_PORT);

	// Send register command to server.  Get back response data
	// and store it.   If denied registration, then quit.
	buffer.reqRep = REGISTER;
	sendto(client, buffer.data, 1, 0, (struct sockaddr*)&serverAddress, length);
	len = recvfrom(client, buffer.data, sizeof(buffer), 0, (struct sockaddr*)&serverAddress, &length);

	if(buffer.reqRep == OK){
		robot = buffer.robot;
		printf("Robot id=%2d connect ok\n", buffer.id + 1);
	}else if(buffer.reqRep == NOT_OK){
		printf("Cannot add more robot there is already 20.\n");
		return -1;
	}
	// Go into an infinite loop exhibiting the robot behavior
	while (1) {
		// Check if can move forward
		buffer.reqRep = CHECK_COLLISION;
		sendto(client, buffer.data, 2, 0, (struct sockaddr*)&serverAddress, length);	

		// Get response from server.
		len = recvfrom(client, buffer.data, sizeof(buffer), 0, (struct sockaddr*)&serverAddress, &length);
		
		// If ok, move forward
		// Otherwise, we could not move forward, so make a turn.
		// If we were turning from the last time we collided, keep
		// turning in the same direction, otherwise choose a random 
		// direction to start turning.
		if(buffer.reqRep == OK){
			//(newX, newY) = (x+S*cos(d), y+S*sin(d))
			//newX = x+S*cos(d)
			//newY = y+S*sin(d)
			robot.x = robot.x + ROBOT_SPEED * cos(robot.direction * (PI / 180)) ;
			robot.y = robot.y + ROBOT_SPEED * sin(robot.direction * (PI / 180));
		}else if(buffer.reqRep == NOT_OK_BOUNDARY){
			robot.direction += rand() % 360 - 180;
		}else if(buffer.reqRep == NOT_OK_COLLIDE){
			robot.direction += rand() % 360 - 180;
		}else if(buffer.reqRep == LOST_CONTACT){
			printf("client %d quit.\n", buffer.id + 1);
			close(client);
			return 0;
		}else{
			break;
		}
		// Send update to server
		buffer.reqRep = STATUS_UPDATE;
		buffer.robot = robot;
		sendto(client, buffer.data, sizeof(buffer), 0, (struct sockaddr*)&serverAddress, length);

		// Uncomment line below to slow things down a bit 
		// usleep(1000);   even use this the robot speed is still too fast
		// so that I make it more slowly in order to check i was correct or wrong
		usleep(15000);
	}
}

