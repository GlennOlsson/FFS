#include "cmd.h"

#include "../filesystem/ffs.h"
#include "../filesystem/inode_table.h"
#include "../filesystem/storage.h"
#include "../filesystem/file_coder.h"
#include "../system/state.h"
#include "../helpers/constants.h"

#include <iostream>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

// Expected in the form 
//  	foo/bar/fizz[ext]
// or
//		/for/bar/fizz[ext]
// TODO: Test
vector<string> path_parts(string path) {
	vector<string> v;
	size_t prev_pos = 0;
	size_t pos = path.find("/");
	if(pos == 0) { // if first char, skip it as to not fuck up dir list
		path = path.substr(1);
		pos = path.find("/");
	}

	while(pos != string::npos) {
		size_t len = pos - prev_pos;
		string s = path.substr(prev_pos, len);

		v.push_back(s);

		prev_pos = pos + 1; // Skip the /
		pos = path.find("/", pos + 1);
	}

	string filename = path.substr(prev_pos);
	v.push_back(filename);

	return v;
}

// Save src file to FFS at save_path (it's name and parent directories)
void save() {
	string src_path;
	cout << "Enter file to save: ";
	cin >> src_path;

	if (FILE *file = fopen(src_path.c_str(), "r")) {
		string save_path;
		cout << "Enter FFS path name: ";
		cin >> save_path;

		vector<string> dirs = path_parts(save_path);
		string filename = dirs.back();
		dirs.pop_back(); // Removes last element == filename

		FFS::InodeTable* table = FFS::State::get_inode_table();

		// Root dir entry
		FFS::InodeEntry* dir_entry = table->entry(FFS_ROOT_INODE);
		// Assumes directory is only 1 post
		auto blob = FFS::Storage::get_file(dir_entry->post_blocks->at(0));
		// Root dir (/)
		FFS::Directory* dir = FFS::Storage::dir_from_blob(blob);
		for(string dir_name: dirs) {
			auto inode_id = dir->get_file(dir_name);
			dir_entry = table->entry(inode_id);
			blob = FFS::Storage::get_file(dir_entry->post_blocks->at(0));
			dir = FFS::Storage::dir_from_blob(blob);
		}

		auto blobs = FFS::encode(src_path);
		auto post_ids = FFS::Storage::upload_file(blobs);

		fseek(file, 0L, SEEK_END);
		auto file_size = ftell(file);

		table->new_file(post_ids, file_size, false);

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

void print_table() {
	auto table = FFS::State::get_inode_table();
	auto entries = table->entries;
	for(auto entry: *entries) {
		auto id = entry.first;
		auto entry_obj = entry.second;

		stringstream ss;

		ss << "inode " << id << ": ";

		bool is_dir = entry_obj->is_dir;
		ss << (is_dir ? "directory" : "file") << " of " << entry_obj->length << " bytes at ";

		for(auto post_id: *entry_obj->post_blocks) {
			ss << post_id << ", ";
		}
		
		cout << ss.str() << "\n";
	}
}

void parse_input(string& cmd) {
	if(cmd == "save")
		save();
	else if(cmd == "read")
		read();
	else if(cmd == "ls")
		cout << "/" << endl; // Static, always at root dir
	else if(cmd == "table")
		print_table();
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