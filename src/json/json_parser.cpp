#include "../api/json.h"
#include "json.h"

#include <sstream>
#include <iostream>
#include <string>


using FFS::API::JSON::JSONValue;
using FFS::API::JSON::JSONObject;

// Create array or object structure
void* dom_create_structure(int nesting, int is_obj){
	if(is_obj)
		return new JSONValue(sizeof(JSONObject*), new JSONObject(), JSON_OBJECT_BEGIN);
	else
		return new JSONValue(sizeof(std::vector<JSONValue*>*), new std::vector<JSONValue*>, JSON_ARRAY_BEGIN);
}

// Create primitive data
void* dom_create_data(int type, const char* data, uint32_t length) {
	void* ptr;
	uint32_t size;

	switch (type) {
	case JSON_INT:
		ptr = new long(std::stol(data));
		size = sizeof(long*);
		break;
	
	case JSON_FLOAT:
		ptr = new double(std::stod(data));
		size = sizeof(double*);
		break;

	case JSON_STRING:
		ptr = new std::string(data);
		size = sizeof(std::string*);
		break;

	case JSON_TRUE:
		ptr = new bool(true);
		size = sizeof(bool*);
		break;

	case JSON_FALSE:
		ptr = new bool(false);
		size = sizeof(bool*);
		break;

	case JSON_NULL:
		ptr = (void*) 0x1;
		size = sizeof(void*);
		break;

	default:
		std::cout << "Create data of unknown type!" << std::endl;
		ptr = nullptr;
		size = 0;
		break;
	}
	
	return new JSONValue(size, ptr, (json_type) type);
}

// Append new value to object or array
int dom_append(void* struc, char* key, uint32_t key_length, void* value){
	auto json_val = (JSONValue*) value;
	auto json_struc_val = (JSONValue*) struc;

	if(!json_val->data)
		return 1;

	// If key is not null, is a object. Else a list
	if(key) {
		JSONObject* object = (JSONObject*) json_struc_val->data;

		object->set(key, json_val);
	} else { // Is array
		auto arr = (std::vector<JSONValue*>*) json_struc_val->data;
		arr->push_back(json_val);
	}

	return 0;
}

const JSONObject* FFS::API::JSON::parse(std::istream& json_stream) {
	json_parser_dom dom;

	json_parser_dom_init(&dom, &dom_create_structure, &dom_create_data, &dom_append);

	json_parser parser;

	if (json_parser_init(&parser, NULL, json_parser_dom_callback, &dom)) {
		fprintf(stderr, "something wrong happened during init\n");
	}
 
	while(!json_stream.eof())
		json_parser_char(&parser, json_stream.get());

	if(!json_parser_is_done(&parser)) {
		FFS::err << "parser not done!" << std::endl;
		return nullptr;
	}

	JSONValue* root_val = (JSONValue*) dom.root_structure;
	JSONObject* root_obj = (JSONObject*) root_val->data;

	json_parser_dom_free(&dom);
	json_parser_free(&parser);

	return root_obj;
}