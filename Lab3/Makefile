src = $(wildcard *.cc)
obj = $(patsubst %.cc, %.o, $(src))

all: client kvstore2pcsystem

client: client.o wrap.o resp.o
	g++ client.o wrap.o  resp.o -o client -Wall -lpthread
kvstore2pcsystem: main.o wrap.o participant.o coordinator.o threadpool.o 
	g++ main.o wrap.o  participant.o coordinator.o threadpool.o -o kvstore2pcsystem -Wall -lpthread 

%.o:%.cc
	g++ -c $< -Wall -lpthread

.PHONY: clean all
clean: 
	-rm -rf client kvstore2pcsystem  $(obj)

