#include "cmd.h"

#include <iostream>
#include <string>

using namespace std;

// Save src file to FFS at save_path (it's name and parent directories)
void save() {
	string src_path;
	cout << "Enter file to save: ";
	cin >> src_path;

	if (FILE *file = fopen(src_path.c_str(), "r")) {
		string save_path;
		cout << "Enter path name: ";
		cin >> save_path;


		cout << "Saved file" << endl;

        fclose(file);
    } else {
        cout << "No file at " << src_path << endl;
    }
}

// Read file from FFS to stout
void read() {
	string ffs_path;
	cout << "Enter FFS path: ";
	cin >> ffs_path;

	cout << "Lorem ipsum dolor..." << endl;
}

void parse_input(string& cmd) {
	if(cmd == "save")
		save();
	else if(cmd == "read")
		read();
	else if(cmd == "ls")
		cout << "/" << endl; // Static, always at root dir
	else {
		cout << cmd << " does not match any commands" << endl;
	}
}

void interact() {
	
	string input;
	while(1) {
		cout << "Enter command: ";
		cin >> input;
		parse_input(input);
		cout << endl;
	}
}