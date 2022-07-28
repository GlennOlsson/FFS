#include "curl.h"

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Info.hpp>
#include <curl/curl.h>
#include <string>
#include <sstream>
#include <utilspp/clone_ptr.hpp>
#include <vector>
#include <utility>

std::stringstream* post(std::string base_url, std::string params, std::vector<std::pair<std::string, std::string>> forms_v) {
	curlpp::Cleanup myCleanup;
	curlpp::Easy request;

	request.setOpt<curlpp::options::Url>(base_url + "?" + params);
	
	auto ss = new std::stringstream();
	request.setOpt<curlpp::options::WriteStream>(ss);

	curlpp::Forms forms;
	for(auto form_entry: forms_v) {
		auto form = new curlpp::FormParts::Content(form_entry.first, form_entry.second);
		forms.push_back(utilspp::clone_ptr<curlpp::FormPart>(form));
	}
	request.setOpt<curlpp::options::HttpPost>(forms);

	request.perform();

	long http_code;
	curlpp::InfoGetter::get(request, CURLINFO_RESPONSE_CODE, http_code);

	std::cout << "got http code " << http_code << std::endl;
	std::cout << "Full url: " << base_url + "?" + params << std::endl;

	return ss;
}

std::stringstream* get(std::string base_url, std::string params) {
	curlpp::Cleanup myCleanup;
	curlpp::Easy request;

	request.setOpt<curlpp::options::Url>(base_url + "?" + params);
	
	std::cout << "full url: " << (base_url + "?" + params) << std::endl;

	auto ss = new std::stringstream();
	request.setOpt<curlpp::options::WriteStream>(ss);

	request.perform();

	long http_code;
	curlpp::InfoGetter::get(request, CURLINFO_RESPONSE_CODE, http_code);

	std::cout << "status code: " << http_code << std::endl;

	return ss;
}