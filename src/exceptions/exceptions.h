#pragma once

#include "../helpers/types.h"
#include "../helpers/constants.h"

#include <string>
#include <stdexcept>

namespace FFS {

class Exception: public std::runtime_error {
protected:
	Exception(std::string reason) : std::runtime_error(reason) {}
};

class BadFFSFile: public FFS::Exception {
public:
	BadFFSFile(std::string reason) : Exception(reason) {}
};

class BadFFSHeader: public BadFFSFile {
public:
	BadFFSHeader(std::string why);
};

class UnexpectedEOF: public BadFFSFile {
public:
	UnexpectedEOF(uint64_t eof_location);
};

class FFSFileNotInodeTable: public BadFFSFile {
public:
	FFSFileNotInodeTable(): BadFFSFile("The image deserialized as an inode table is not an inode table") {}
};

class FFSFileNotDirectory: public BadFFSFile {
public:
	FFSFileNotDirectory(): BadFFSFile("The image deserialized as a directory is not a directory") {}
};

class StorageException: public FFS::Exception {
public:
	StorageException(std::string reason) : Exception(reason) {}
};

class NoFileWithName: public StorageException {
public:
	NoFileWithName(std::string file_name);
};

class NoPathWithName: public StorageException {
public:
	NoPathWithName(std::string path);
};

class NoOpenFileWithFH: public StorageException {
public:
	NoOpenFileWithFH(FFS::file_handle_t fh) : 
		FFS::StorageException("No open file or directory with file handle " + std::to_string(fh)) {}
};

class FileAlreadyExists: public StorageException {
public:
	FileAlreadyExists(std::string file_name);
};

class NoFileWithInode: public StorageException {
public:
	NoFileWithInode(FFS::inode_t inode);
};

// when part of the path is a file rather than a directory
class BadFFSPath: public StorageException {
public:
	BadFFSPath(std::string path, std::string bad_part);
};

class APIException: public FFS::Exception {
public:
	APIException(std::string reason): FFS::Exception(reason){}
};

class BadHTTPStatusCode: public FFS::APIException {
public:
	BadHTTPStatusCode(long code);
};

class FlickrException: public FFS::APIException {
public:
	FlickrException(std::string reason): FFS::APIException(reason){}
};

class BadFlickrKeys: public FFS::FlickrException {
public:
	BadFlickrKeys(): FFS::FlickrException("Could not verify API keys"){}
};

class NoPhotoWithTag: public FFS::FlickrException {
public:
	NoPhotoWithTag(std::string tag): FFS::FlickrException("Could not find any photos with tag " + tag) {}
};

class NoPhotoWithID: public FFS::FlickrException {
public:
	NoPhotoWithID(std::string id): FFS::FlickrException("Could not find any photos with id " + id) {}
};

class NoPhotosOnFlickr: public FFS::FlickrException {
public:
	NoPhotosOnFlickr(): FFS::FlickrException("No photos found on Flickr"){}
};

class JSONException: public FFS::APIException {
public:
	JSONException(std::string why): FFS::APIException(why) {}
};

class JSONKeyNonexistant: public FFS::JSONException {
public:
	JSONKeyNonexistant(std::string key): FFS::JSONException("JSONValue has no key \"" + key + "\"") {}
};

class CipherIntegrityCompromised: public FFS::Exception {
public:
	CipherIntegrityCompromised(): FFS::Exception("The encrypted cipher text's integrity has been compromised. Could not verify integrity") {}
};

class NoEncryptionSecret: public FFS::Exception {
public:
	NoEncryptionSecret(): FFS::Exception("Must set " + std::string(FFS_ENCRYPTION_SECRET_KEY) + " in environment") {}
};
};