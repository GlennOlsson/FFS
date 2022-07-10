// TODO:
//			Handle remove of file
//			Enable user to create only dir (mkdir)
//			Enable user to remove dir (rmdir)

#include "cmd.h"

#include "../filesystem/fs.h"
#include "../filesystem/storage.h"
#include "../filesystem/file_coder.h"
#include "../system/state.h"

#include "../exceptions/exceptions.h"

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>

using namespace std;

// Save src file to FFS at save_path (it's name and parent directories)
void save() {
	string src_path;
	cout << "Enter file to save: ";
	cin >> src_path;

	if (FILE *file = fopen(src_path.c_str(), "r")) {
		string save_path;
		cout << "Enter FFS path name: ";
		cin >> save_path;

		ifstream i_file(src_path);
		FFS::FS::create_file(save_path, i_file);

		cout << "Saved file" << endl;

        fclose(file);
    } else {
        cout << "No file at " << src_path << endl;
    }
}

void print_dir(FFS::Directory* dir) {
	auto dir_content = dir->entries;
	cout << "Content: " << endl;
	for(auto item: *dir_content) {
		cout << "\t" << item.first << " (inode " << item.second << ")" << endl;
	}
}

// Read file from FFS to stout
void read() {
	string ffs_path;
	cout << "Enter FFS path: ";
	cin >> ffs_path;

	if(!FFS::FS::exists(ffs_path)) {
		cout << "File or directory does not exist" << endl;
		return;
	}

	if(FFS::FS::is_dir(ffs_path)) {
		auto dir = FFS::FS::read_dir(ffs_path);
		print_dir(dir);
	} else {
		FFS::FS::read_file(ffs_path, cout);
	}
}

void print_table() {
	auto table = FFS::State::get_inode_table();
	auto entries = table->entries;
	stringstream ss;

	for(auto entry: *entries) {
		auto id = entry.first;
		auto entry_obj = entry.second;

		ss << "inode " << id << ": "; 

		bool is_dir = entry_obj->is_dir;
		ss << (is_dir ? "directory" : "file") << " of " << entry_obj->length << " bytes at ";

		for(auto post_id: *entry_obj->post_blocks) {
			ss << post_id << ", ";
		}

		ss << endl;
	}
	cout << ss.str();
}

void print_root_dir() {
	auto dir = FFS::FS::read_dir("/");
	print_dir(dir);
}

void print_inode_content() {
	FFS::inode_id inode_id;
	cout << "Enter inode id to read: ";
	cin >> inode_id;

	auto table = FFS::State::get_inode_table();
	if(!table->entries->contains(inode_id)) {
		cout << "No entry with inode " << inode_id << endl;
		return;
	}

	auto inode_entry = table->entry(inode_id);
	auto blobs = FFS::Storage::get_file(inode_entry->post_blocks);
	if(inode_entry->is_dir) {
		auto dir = FFS::Storage::dir_from_blobs(blobs);
		print_dir(dir);
	} else {
		FFS::decode(blobs, cout);
	}
}

// Erases it from inode table and directory entries, but not the actual blocks
void remove_file() {
	string path;
	cout << "Enter file to remove: ";
	cin >> path;
	
	FFS::FS::remove(path);
}

void parse_input(string& cmd) {
	if(cmd == "save")
		save();
	else if(cmd == "read")
		read();
	else if(cmd == "table")
		print_table();
	else if(cmd == "ls")
		print_root_dir(); // Static, always at root dir
	else if(cmd == "inode")
		print_inode_content();
	else if(cmd == "rm")
		remove_file();
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