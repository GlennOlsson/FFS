#include "../api/json.h"
#include "json.h"
#include "../exceptions/exceptions.h"

#include <memory>
#include <iostream>
#include <ostream>
#include <sstream>

using FFS::API::JSON::JSONValue;
using FFS::API::JSON::JSONObject;

JSONValue::JSONValue(uint32_t length, void* data, int type) : 
	length(length), 
	data(data), 
	type(type) 
{}

JSONValue::JSONValue(JSONValue& val) {
	this->length = val.length;
	this->data = val.data;
	this->type = val.type;
}

JSONObject::JSONObject() {
	this->content = new std::map<std::string, JSONValue*>();
}

JSONObject::JSONObject(JSONObject& obj) {
	auto new_map = new std::map<std::string, JSONValue*>(*obj.content);
	
	this->content = new_map;
}

void JSONObject::set(std::string key, JSONValue* val) {
	this->content->insert({key, val});
}

template<class T>
const T& as(JSONValue* val) {
	return *(T*) val->data;
}

template<class T>
const T& JSONObject::get(std::string key) const {
	if(!this->content->contains(key))
		throw FFS::JSONKeyNonexistant(key);
		
	auto val = this->content->at(key);
	return as<T>(val);
}

const std::string& JSONObject::get_str(std::string key) const {
	return get<std::string>(key);
}
const long JSONObject::get_long(std::string key) const {
	return get<long>(key);
}
const bool JSONObject::get_bool(std::string key) const {
	return get<bool>(key);
}
const JSONObject& JSONObject::get_obj(std::string key) const {
	return get<JSONObject>(key);
}
const std::vector<JSONValue*>& JSONObject::get_arr(std::string key) const {
	return get<std::vector<JSONValue*>>(key);
}
const void* JSONObject::get_any(std::string key) const {
	return this->content->at(key)->data;
}

const JSONObject& FFS::API::JSON::as_obj(JSONValue* val) {
	return as<JSONObject>(val);
}

template<class T>
const T& as(void* data) {
	return *(T*) data;
}

void print_type(int, void*, std::ostream&, int);
void print_arr(const std::vector<JSONValue*>&, int, std::ostream&);

std::string get_tabs(int depth) {
	std::stringstream ss;
	for(int i = 0; i < depth + 1; i++) {
		ss << "\t";
	}
	return ss.str();
}

void print_type(int type, void* data, std::ostream& stream, int depth) {
	switch (type) {
	case JSON_INT:
		stream << std::to_string(as<long>(data)) ;
		break;

	case JSON_STRING:
		stream << "\"" << as<std::string>(data) << "\"";
		break;

	case JSON_TRUE:
		stream << "true";
		break;

	case JSON_FALSE:
		stream << "false";
		break;

	case JSON_NULL:
		stream << "null";
		break;
	
	case JSON_OBJECT_BEGIN:
		stream << "{\n";
		FFS::API::JSON::print_obj(as<JSONObject>(data), depth + 1, stream);
		stream << get_tabs(depth) << "}";
		break;

	case JSON_ARRAY_BEGIN:
		stream << "[\n";
		print_arr(as<std::vector<JSONValue*>>(data), depth + 1, stream);
		stream << get_tabs(depth) << "]";
		break;

	default:
		std::cout << "Type " << type << " not covered" << std::endl;;
		break;
	}
	stream << ",\n";
}

void print_arr(const std::vector<JSONValue*>& vec, int depth, std::ostream& stream) {
	auto tabs = get_tabs(depth);

	for(auto val: vec) {
		stream << tabs;
		print_type(val->type, val->data, stream, depth);
	}
}

void FFS::API::JSON::print_obj(const JSONObject& obj, int depth, std::ostream& stream) {
	auto tabs = get_tabs(depth);

	for(auto k_v: *obj.content) {
		auto key = k_v.first;

		stream << tabs << "\"" << key << "\": ";

		auto type = k_v.second->type;
		auto val = k_v.second->data;

		print_type(type, val, stream, depth);
	}
}

std::ostream& operator<<(std::ostream& stream, const JSONObject& obj) {
	stream << "{\n";
	
 	FFS::API::JSON::print_obj(obj, 0, stream);

	stream << "}";

	return stream;
}