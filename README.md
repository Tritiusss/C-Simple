# C-Simple

list:
display.c
environmentServer.c
robotClient.c
simulator.h
stop.c
makefile
ReadMe.txt

run my code:
make
./environmentServer &
./robotClient &                          //For this commind you want my program have 
					 // how many robots then run how many times.
					 // if you want to add more than 20 will rejust
					 // and give you commind warning.
./stop
ps
make clean



Special Note: The robots speed is too fast so that I change the usleep(1000)to 
usleep(15000) and if you want to change it back, i keep the usleep(1000) there with
// so that delete my usleep(15000) and // before usleep(1000) is ok
