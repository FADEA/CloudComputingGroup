cc= g++
prom = haha

obj = server.o

$(prom):$(obj)
	$(cc) -o $(prom) $(obj) -pthread

%.o:%.cc 
	$(cc) -c $< -o $@

clean:
	rm -rf $(obj) $(prom)
