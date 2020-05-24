src = $(wildcard *.cc)
obj = $(patsubst %.cc, %.o, $(src))

all: server client s_epoll threadpool

server: server.o wrap.o
	g++ server.o wrap.o -o server -Wall -lpthread
client: client.o wrap.o
	g++ client.o wrap.o -o client -Wall -lpthread
s_epoll: server_epoll.o wrap.o
	g++ server_epoll.o wrap.o -o server_epoll -Wall -lpthread
threadpool: threadpool.o wrap.o
	g++ threadpool.o wrap.o -o threadpool -Wall -lpthread

%.o:%.cc
	g++ -c $< -Wall

.PHONY: clean all
clean: 
	-rm -rf server client s_epoll threadpool  $(obj)

