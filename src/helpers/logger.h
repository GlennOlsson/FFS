#ifndef FFS_LOGGER_H
#define FFS_LOGGER_H

#include "../config.h"

#include <fstream>
#include <iostream>

namespace FFS {
#ifdef DEBUG
	inline static std::ostream& log = std::cout;
	inline static std::ostream& err = std::cerr;
#else
	inline static std::ofstream _log = std::ofstream("ffs.log");
	inline static std::ofstream _err = std::ofstream("ffs_err.log");

	inline static std::ostream& log = _log;
	inline static std::ostream& err = _err;
#endif
}

#endif