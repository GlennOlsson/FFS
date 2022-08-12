#ifndef FFS_JSON_H
#define FFS_JSON_H

#include <istream>
#include <string>
#include <vector>
#include <map>

namespace FFS::API::JSON {

class JSONValue {
public:
	JSONValue(uint32_t, void*, int);
	JSONValue(JSONValue&);

	uint32_t length;
	void* data;
	int type;
};
class JSONObject;

void print_obj(const JSONObject&, int, std::ostream&);

class JSONObject {
private:

	std::map<std::string, JSONValue*>* content;

public:
	JSONObject();
	JSONObject(JSONObject&);
	// ~JSONObject();

	void set(std::string, std::string&);
	// void set(std::string, char*);
	void set(std::string, long);
	void set(std::string, bool);
	void set(std::string, JSONObject&);
	void set(std::string, std::vector<JSONValue*>&);
	void set(std::string key, void* data, uint32_t length);
	void set(std::string, JSONValue*);

	template<class T>
	const T& get(std::string) const;

	const std::string& get_str(std::string) const;
	const long get_long(std::string) const;
	const bool get_bool(std::string) const;
	const JSONObject& get_obj(std::string) const;
	const std::vector<JSONValue*>& get_arr(std::string) const;
	const void* get_any(std::string) const;

	friend void print_obj(const JSONObject&, int, std::ostream&);
};

std::ostream& operator<<(std::ostream&, const JSONObject&);

const JSONObject& as_obj(JSONValue*);

const JSONObject* parse(std::istream&);

}

#endif