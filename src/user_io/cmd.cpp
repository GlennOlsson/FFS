// TODO:
//			Handle remove of file
//			Enable user to create only dir (mkdir)
//			Enable user to remove dir (rmdir)

#include "cmd.h"

#include "../filesystem/ffs.h"
#include "../filesystem/inode_table.h"
#include "../filesystem/storage.h"
#include "../filesystem/file_coder.h"
#include "../system/state.h"
#include "../helpers/constants.h"

#include "../exceptions/exceptions.h"

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

struct Traverser {
	FFS::Directory* dir;
	FFS::inode_id dir_inode;
	string filename;
};

// traverse a tree of directories, returning a structure containing the parent directory and its inode id, along
// with the filename (could be a directory name too). If create_on_nonexistant is false and one part of the path
// does not exist, a NoFileWithName exception will be thrown
struct Traverser* traverse_path(string path, bool create_on_nonexistant) {

	vector<string> dirs = path_parts(path);
	string filename = dirs.back();
	dirs.pop_back(); // Removes last element == filename

	FFS::InodeTable* table = FFS::State::get_inode_table();

	// Root dir entry
	FFS::InodeEntry* dir_entry = table->entry(FFS_ROOT_INODE);
	// Assumes directory is only 1 post
	auto blobs = FFS::Storage::get_file(dir_entry->post_blocks);
	// Root dir (/)
	FFS::Directory* dir = FFS::Storage::dir_from_blobs(blobs);

	FFS::inode_id inode_id = FFS_ROOT_INODE;;
	for(string dir_name: dirs) {
		try {
			inode_id = dir->get_file(dir_name);
			dir_entry = table->entry(inode_id);
			if(!dir_entry->is_dir) {
				throw FFS::BadFFSPath(path, dir_name);
			}
			blobs = FFS::Storage::get_file(dir_entry->post_blocks);
			dir = FFS::Storage::dir_from_blobs(blobs);
		} catch(const FFS::NoFileWithName& b) {
			if(create_on_nonexistant) {
				FFS::Directory* new_dir = new FFS::Directory();
				auto new_inode_id = FFS::Storage::upload(*new_dir);
				dir->add_entry(dir_name, new_inode_id);
				FFS::Storage::update(*dir, inode_id);

				dir = new_dir; // Switch pointer
				inode_id = new_inode_id;
			} else {
				throw b;
			}
		}
	}

	return new struct Traverser({dir, inode_id, filename});
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

		auto traverser = traverse_path(save_path, true);

		auto table = FFS::State::get_inode_table();

		if(traverser->dir->entries->contains(traverser->filename)) {
			cout << "file at path exists, overwriting" << endl;
			auto inode_id = traverser->dir->entries->at(traverser->filename);
			traverser->dir->entries->erase(traverser->filename);
			table->entries->erase(inode_id);
		}

		auto blobs = FFS::encode(src_path);
		auto post_ids = FFS::Storage::upload_file(blobs);

		fseek(file, 0L, SEEK_END);
		auto file_size = ftell(file);

		auto file_inode_id = table->new_file(post_ids, file_size, false);
		traverser->dir->add_entry(traverser->filename, file_inode_id);
		FFS::Storage::update(*traverser->dir, traverser->dir_inode);

		cout << "Saved file" << endl;

        fclose(file);
    } else {
        cout << "No file at " << src_path << endl;
    }
}

void print_dir(FFS::Directory* dir, string dir_name) {
	auto dir_content = dir->entries;
	cout << "Content of " << dir_name << endl;
	for(auto item: *dir_content) {
		cout << "\t" << item.first << " (inode " << item.second << ")" << endl;
	}
}

// Read file from FFS to stout
void read() {
	string ffs_path;
	cout << "Enter FFS path: ";
	cin >> ffs_path;

	auto traverser = traverse_path(ffs_path, false);

	auto table = FFS::State::get_inode_table();

	FFS::inode_id inode_id;
	try {
		inode_id = traverser->dir->get_file(traverser->filename);
	} catch(const FFS::NoFileWithName& b) {
		cout << "no file named " << traverser->filename << endl;
		return;
	}

	auto entry = table->entry(inode_id);
	auto blobs = FFS::Storage::get_file(entry->post_blocks);
	if(entry->is_dir) {
		auto dir = FFS::Storage::dir_from_blobs(blobs);
		print_dir(dir, traverser->filename);
	} else {
		FFS::decode(blobs, cout);
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
	FFS::InodeTable* table = FFS::State::get_inode_table();

	// Root dir entry
	FFS::InodeEntry* dir_entry = table->entry(FFS_ROOT_INODE);

	auto blocks = dir_entry->post_blocks;
	auto blobs = FFS::Storage::get_file(blocks);

	FFS::Directory* root_dir = FFS::Storage::dir_from_blobs(blobs);
	print_dir(root_dir, "/");
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
		print_dir(dir, "Directory");
	} else {
		FFS::decode(blobs, cout);
	}
}

// Erases it from inode table and directory entries, but not the actual blocks
void remove_file() {
	string path;
	cout << "Enter file to remove: ";
	cin >> path;
	auto traverser = traverse_path(path, false);

	auto table = FFS::State::get_inode_table();

	auto inode_id = traverser->dir->entries->at(traverser->filename);
	traverser->dir->entries->erase(traverser->filename);
	table->entries->erase(inode_id);
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