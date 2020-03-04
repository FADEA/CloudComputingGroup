#include <iostream>
#include <fstream>
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
	while(getline(inputfile,sudo)){
		cout<<sudo<<endl;
	}
	
	return 0;
}
