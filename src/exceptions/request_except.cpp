#include "exceptions.h"

#include <string>

FFS::BadHTTPStatusCode::BadHTTPStatusCode(int status):
	FFS::RequestException("Bad HTTP request, status code: " + std::to_string(status))
{}