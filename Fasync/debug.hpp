#ifndef DEBUG_HEADER
#define DEBUG_HEADER

#include <iostream>
#include <sstream>

#define DEBUG_UTILS 0

#if DEBUG_UTILS

#define DEBUG_LOG(x) \
do {\
	std::stringstream s; \
	s << x << "\n";\
	std::cout << s.str();\
} while(0)\

#else
#define DEBUG_LOG(x)
#endif // DEBUG_UTILS


#endif // !DEBUG_HEADER
