#include "curl.h"

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Info.hpp>
#include <curl/curl.h>
#include <string>
#include <sstream>
#include <utilspp/clone_ptr.hpp>
#include <memory>

#include "../exceptions/exceptions.h"

#include "../helpers/functions.h"
#include "../helpers/logger.h"

std::shared_ptr<curlpp::Easy> create_request(std::string url, std::string params, std::shared_ptr<std::stringstream> ss) {
	auto request = std::make_shared<curlpp::Easy>();

	// If there are params, add to url
	if(params.length() > 1) {
		url += "?" + params;
	}

	request->setOpt<curlpp::options::Url>(url + "?" + params);
	
	request->setOpt<curlpp::options::WriteStream>(ss.get());

	return request;
}

void assert_http_status(std::shared_ptr<curlpp::Easy> request) {
	long http_code;
	curlpp::InfoGetter::get(*request, CURLINFO_RESPONSE_CODE, http_code);

	if(http_code < 200 || http_code > 299)
		throw FFS::BadHTTPStatusCode(http_code);
}

std::shared_ptr<std::stringstream> FFS::API::HTTP::get(std::string url, std::string params) {
	curlpp::Cleanup cleanup;
	auto ss = std::make_shared<std::stringstream>();

	auto request = create_request(url, params, ss);

	auto time_before = FFS::curr_nanoseconds();
	request->perform();
	auto time_after = FFS::curr_nanoseconds();

	auto delta_time = time_after - time_before;
	auto downloaded_bytes = FFS::stream_size(*ss);

	FFS::log << "Downloaded (GET): " << downloaded_bytes << " B in " << delta_time << " ns" << std::endl;

	assert_http_status(request);

	return ss;
}