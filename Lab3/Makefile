src = $(wildcard *.cc)
obj = $(patsubst %.cc, %.o, $(src))

all: server client s_epoll  main

server: server.o wrap.o
	g++ server.o wrap.o -o server -Wall -lpthread
client: client.o wrap.o
	g++ client.o wrap.o -o client -Wall -lpthread
s_epoll: server_epoll.o wrap.o
	g++ server_epoll.o wrap.o -o s_epoll -Wall -lpthread
resp: resp.o wrap.o
	g++ resp.o wrap.o -o resp -Wall 
main: main.o wrap.o participant.o coordinator.o threadpool.o
	g++ main.o wrap.o  participant.o coordinator.o threadpool.o -o main -Wall -lpthread 

%.o:%.cc
	g++ -c $< -Wall -lpthread

.PHONY: clean all
clean: 
	-rm -rf server client s_epoll main resp  $(obj)

