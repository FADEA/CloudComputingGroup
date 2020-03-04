#include <iostream>
#include <fstream>
using namespace std;

int main(){
	ifstream inputfile;
	string flieName;
	cin>>fileName;
	
	if(fileName[0]=='.'&&flieName[1]=='/'){
		fileName.erase(0,2);
	}
	
	inputfile.open(fileName,ios::in);
	
	string sudo;
	inputfile>>sudo;
	cout<<sudo;
	return 0;
}
