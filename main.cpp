#include "FileCacheImpl.h"
#include <iostream>
#include <cstring>
#include <cstring>

using namespace std;
int main()
{

	FileCache * f = new FileCacheImpl(12,1);
	
	
	vector<string> files;
	
	for(int i = 0; i < 5; i++)
	{
		string filename = "/user/home/file.txt";
		filename.push_back(i+'A');
		files.push_back(filename);cout<<filename<<endl;
		
	}
	
	cout<<"Out";
	f->PinFiles(files);
	vector<string> files2;

	for(int i = 0; i < 5; i=i+2)
	{
		string filename = "/user/home/file.txt";
		filename.push_back(i+'A');
		files2.push_back(filename);
		
	}
	f->UnpinFiles(files2);
	f->PrintPinned();
	int loop = -9999990;
	while(loop < 9999999)
	{
		int i = loop;
		float j = 9.234;
		float t = i / j;
		loop++;
		if (loop%1000 == 0)cout<<loop;
	
	}
	loop = 0;
	while(loop < 9999999)
	{
		int i = loop;
		float j = 9.234;
		float t = i / j;
		loop++;
	
	}
	//f->DeleteFile(filename);
	files.clear();
	for(int i = 0; i < 10; i++)
	{
		string filename = "/user/home/file.txt";
		filename.push_back(i+'A');

		
		files.push_back(filename);cout<<filename<<endl;
		
	}
	f->PinFiles(files);
	f->PrintPinned();	
	cout<<"Done!";
}
