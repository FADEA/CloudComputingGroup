#include <iostream>
#include <string>
#include <fstream>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "sudoku.h"
using namespace std;

int main(){
	ifstream inputfile;
	string fileName;
	cin>>fileName;
	
	if(fileName[0]=='.'&&fileName[1]=='/'){
		fileName.erase(0,2);
	}
	
	inputfile.open(fileName,ios::in);
	
	string sudo;
    int total_solved = 0;
    int total = 0;
	bool (*solve)(int) = solve_sudoku_basic;
	while(getline(inputfile,sudo)){
		cout<<sudo<<endl;
		if (sudo.length() >= N) {
		  ++total;
		  input(sudo.c_str());
		  init_cache();
		  //if (solve_sudoku_min_arity_cache(0)) {
		  //if (solve_sudoku_min_arity(0))
		  //if (solve_sudoku_basic(0)) {
		  if (solve(0)) {
		    ++total_solved;
		    if (!solved())
		      assert(0);
		  }
		  else {
		    printf("No: %s", sudo);
		  }
		}
	    for(int i=0;i<N;i++){
        //    if(i%9==0){cout<<endl;}
            cout<<board[i];
	    }
	    cout<<endl;
	}
	
	return 0;
}
