#ifndef FFS_LOGGER_H
#define FFS_LOGGER_H

#include "../config.h"

#include <fstream>
#include <iostream>

namespace FFS {
#ifdef DEBUG_LOGGING
	inline static std::ostream& log = std::cout;
	inline static std::ostream& err = std::cerr;
#endif 
#ifdef NO_LOGGING
	// Open empty file as ostream, won't log anything
	inline static std::ofstream _log = std::ofstream();
	inline static std::ofstream _err = std::ofstream();

	inline static std::ostream& log = _log;
	inline static std::ostream& err = _err;
#endif
#ifdef FILE_LOGGING
	inline static std::ofstream _log = std::ofstream("ffs.log.nosync");
	inline static std::ofstream _err = std::ofstream("ffs_err.log.nosync");

	inline static std::ostream& log = _log;
	inline static std::ostream& err = _err;
#endif
}

#endif
