cc = g++
prom = http_server
deps = threadpool.h http_conn.h locker.h 
obj = main.o http_conn.o

$(prom):$(obj)
	$(cc) -o $(prom) $(obj) $(deps) -pthread

%.o:%.cc $(deps)
	$(cc) -c $< -o $@

clean:
	rm -rf $(obj) $(prom)
