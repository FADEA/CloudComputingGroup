cc = g++
prom = http_server
deps = server.h
obj = server.o main.o

$(prom):$(obj)
	$(cc) -o $(prom) $(obj) -pthread

%.o:%.cc $(deps)
	$(cc) -c $< -o $@

clean:
	rm -rf $(obj) $(prom)
