#include "exceptions.h"

#include <string>

FFS::BadHTTPStatusCode::BadHTTPStatusCode(long status):
	FFS::RequestException("Bad HTTP request, status code: " + std::to_string(status))
{}