#ifndef FILEKV_PARSER_H
#define FILEKV_PARSER_H

#include "common.h"

namespace com { 
namespace youku { 
namespace data {
namespace qkpack {
namespace util {


class FilekvParser
{
public:
    typedef std::map<std::string,std::string>::const_iterator MapIter;
public:
    FilekvParser();
    ~FilekvParser();
public:
	static bool Parse(const std::string file, std::string &str);
    static bool Parse(std::string file, std::map<std::string,std::string> &kv);

};

} /* namespace util */
} /* namespace qkpack */
} /* namespace data */
} /* namespace youku */
} /* namespace com */

#endif //
