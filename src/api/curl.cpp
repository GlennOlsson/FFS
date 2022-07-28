#include "curl.h"

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Info.hpp>
#include <curl/curl.h>
#include <string>
#include <sstream>
#include <utilspp/clone_ptr.hpp>

#include "../exceptions/exceptions.h"

curlpp::Easy* create_request(std::string url, std::string params, std::stringstream* ss) {
	auto request = new curlpp::Easy();

	// If there are params, add to url
	if(params.length() > 1) {
		url += "?" + params;
	}

	request->setOpt<curlpp::options::Url>(url + "?" + params);
	
	request->setOpt<curlpp::options::WriteStream>(ss);

	return request;
}

void assert_http_status(curlpp::Easy* request) {
	long http_code;
	curlpp::InfoGetter::get(*request, CURLINFO_RESPONSE_CODE, http_code);

	if(http_code < 200 || http_code > 299)
		throw FFS::BadHTTPStatusCode(http_code);
}

std::stringstream* post(std::string url, std::string params) {
	curlpp::Cleanup cleanup;
	auto ss = new std::stringstream();

	auto request = create_request(url, params, ss);

	curlpp::Forms forms;
	request->setOpt<curlpp::options::HttpPost>(forms);

	request->perform();

	assert_http_status(request);

	return ss;
}

std::stringstream* get(std::string url, std::string params) {
	curlpp::Cleanup cleanup;
	auto ss = new std::stringstream();

	auto request = create_request(url, params, ss);

	request->perform();

	assert_http_status(request);

	return ss;
}