src = $(wildcard *.cc)
obj = $(patsubst %.cc, %.o, $(src))

all: server client

server: server.o wrap.o
	gcc server.o wrap.o -o server -Wall
client: client.o wrap.o
	gcc client.o wrap.o -o client -Wall

%.o:%.cc
	gcc -c $< -Wall

.PHONY: clean all
clean: 
	-rm -rf server client $(obj)

