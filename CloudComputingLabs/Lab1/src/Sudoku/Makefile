cc = g++ 
prom = sudoku
source = main1.cc neighbor.cc sudoku_basic.cc sudoku_min_arity.cc sudoku_min_arity_cache.cc sudoku_dancing_links.cc

$(prom): $(source)
	$(cc) -O2 -pthread -o $(prom) $(source) 
