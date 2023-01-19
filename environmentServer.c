#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "simulator.h"

typedef union {
  char data[20];
 	struct{
 		char reqRep;	//Requset/Response
 		char id;	//Robot ID
 		Robot robot;
 	};
}RobotBuffer;


Environment    environment;  // The environment that contains all the robots

//add one function to check robot move to boundary or another robot
int checkCollision(int, float, float);


// Handle client requests coming in through the server socket.  This code should run
// indefinitiely.  It should repeatedly grab an incoming messages and process them. 
// The requests that may be handled are STOP, REGISTER, CHECK_COLLISION and STATUS_UPDATE.   
// Upon receiving a STOP request, the server should get ready to shut down but must
// first wait until all robot clients have been informed of the shutdown.   Then it 
// should exit gracefully.  
void *handleIncomingRequests(void *e) {
	char   online = 1;
  	// ... ADD SOME VARIABLE HERE... //
	struct sockaddr_in serverAddress;
  	struct sockaddr_in clientAddress;
  	socklen_t length = sizeof(clientAddress);
  	int server, result, len, serverLength;
  	int clients[MAX_ROBOTS] = { 0 };
  	RobotBuffer buffer;

  	// Initialize the server
  	server = socket(AF_INET, SOCK_DGRAM, 0);
  	memset(&serverAddress, 0, serverLength);
  	serverAddress.sin_family = AF_INET;
  	serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
  	serverAddress.sin_port = htons(SERVER_PORT);
	serverLength = sizeof(serverAddress);

  	result = bind(server, (struct sockaddr*)&serverAddress, serverLength);
  	if(result != -1){
		printf("server bind %s:%d\n", SERVER_IP, SERVER_PORT);
	}

  	//init robots
  	for(int i = 0; i < MAX_ROBOTS; ++i){
  		environment.robots[i].x = 0;
  		environment.robots[i].y = 0;
  		environment.robots[i].direction = 0;
  	}
  	environment.numRobots = 0;  	
  	

  	// Wait for clients now
	while (online) {
		// ... WRITE YOUR CODE HERE ... //
		float xX, yY;
		memset(buffer.data, 0, sizeof(buffer));
		len = recvfrom(server, buffer.data, sizeof(buffer), 0, (struct sockaddr*)&clientAddress, &length);
		if(buffer.reqRep == REGISTER){
			if(environment.shutDown == 1){
				buffer.reqRep = LOST_CONTACT;
				sendto(server, buffer.data, 1, 0, (struct sockaddr*)&clientAddress, length);
				continue;
			}
			int range = ENV_SIZE - ROBOT_RADIUS * 2;
			for(int i = 0; i < MAX_ROBOTS; i++){
				if(clients[i] == 0){
					do{
						xX = ROBOT_RADIUS + rand() % range;
						yY = ROBOT_RADIUS + rand() % range;
					}while(checkCollision(i, xX, yY) != OK);
					environment.robots[i].x = xX;
					environment.robots[i].y = yY;
					environment.robots[i].direction = (rand() % 360) - 180;
					environment.numRobots++;
					clients[i] = 1;
					result = i;
					break;
				}else{
					result = -1;
				}
			}
			if(result != -1){
				buffer.reqRep = OK;
				buffer.id = result;
				buffer.robot = environment.robots[result];
				sendto(server, buffer.data, sizeof(buffer), 0, (struct sockaddr*)&clientAddress, length);
			}else{
				buffer.reqRep = NOT_OK;
				sendto(server, buffer.data, 1, 0, (struct sockaddr*)&clientAddress, length);
			}
		}else if(buffer.reqRep == STOP){
			int clientNum = 0;
			environment.shutDown = 1;
			for(int i = 0; i < MAX_ROBOTS; i++){
				if(clients[i]){
					clientNum++;
				}
			}
			if(clientNum == 0){
				online = 0;
			}
		}else if(buffer.reqRep == CHECK_COLLISION){
			if(environment.shutDown == 1){
				int clientNum = 0;
				buffer.reqRep = LOST_CONTACT;
				sendto(server, buffer.data, 1, 0, (struct sockaddr*)&clientAddress, length);
				clients[buffer.id] = 0;
				environment.robots[buffer.id].x = 0;
				environment.robots[buffer.id].y = 0;
				environment.robots[buffer.id].direction = 0;

				for(int i = 0; i < MAX_ROBOTS; i++){
					if(clients[i]){
						clientNum++;
					}
				}
				if(clientNum == 0){
					online = 0;
				}
				continue;
			}

			xX = environment.robots[buffer.id].x + ROBOT_SPEED * cos(environment.robots[buffer.id].direction * (PI/180));
			yY = environment.robots[buffer.id].y + ROBOT_SPEED * sin(environment.robots[buffer.id].direction * (PI/180));
			buffer.reqRep = checkCollision(buffer.id, xX, yY);
			sendto(server, buffer.data, 1, 0, (struct sockaddr*)&clientAddress, length);
		}else if(buffer.reqRep == STATUS_UPDATE){
			environment.robots[buffer.id] = buffer.robot;
		}else{
			break;
		}
		
  	}
  	// ... WRITE ANY CLEANUP CODE HERE ... //
  	close(server);
}




int main() {
	// So far, the environment is NOT shut down
	environment.shutDown = 0;
	pthread_t serverP;
	pthread_t redrawP;
	//int check = 0;
	void* update;

	// Set up the random seed
	srand(time(NULL));

	// Spawn an infinite loop to handle incoming requests and update the display
	//while(check = 0){
	pthread_create(&serverP, 0, handleIncomingRequests, &environment);
	pthread_create(&redrawP, 0, redraw, &environment);
	//}
	// Wait for the update and draw threads to complete
	pthread_join(serverP, &update);
	pthread_join(redrawP, &update);
}


int checkCollision(int id, float x, float y){
	float distance, xDistance, yDistance;
	for(int i = 0; i < MAX_ROBOTS; i++){
		if(i != id){
			xDistance = pow(environment.robots[i].x - x, 2);
			yDistance = pow(environment.robots[i].y - y, 2);
			distance = sqrtf(xDistance + yDistance);
			if(distance < ROBOT_RADIUS * 2){
				return NOT_OK_COLLIDE;
			}
		}
	}

	// Check if robots want to go out from graph
	if((x - ROBOT_RADIUS) < 0 || (x + ROBOT_RADIUS) >= ENV_SIZE ||(y - ROBOT_RADIUS) < 0 ||(y + ROBOT_RADIUS) >= ENV_SIZE){
			return NOT_OK_BOUNDARY;
	}
	return OK;
	
}



























