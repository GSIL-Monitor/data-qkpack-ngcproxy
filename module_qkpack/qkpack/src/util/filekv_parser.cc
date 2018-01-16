#include "util/filekv_parser.h"
#include "util/string_util.h"
#include <fstream>
#include <streambuf>

using namespace com::youku::data::qkpack::util;


FilekvParser::FilekvParser(){
}

FilekvParser::~FilekvParser() { 
}

bool FilekvParser::Parse(std::string file, std::map<std::string,std::string> &kv)
{
	std::ifstream fp( file.c_str() );
    if ( !fp.is_open() ) {
        fprintf(stderr, "Can't open config file:%s\n", file.c_str());
        return false;
    }

	std::string line, key, value;
    while ( getline( fp, line ) ) {
        uint32_t i = 0;
        while ( isspace(line[i]) == true && i < line.length() ) ++i;
        if ( i >= line.length() ) {
            continue;
        } else {
            if ( line[i] == '#' )
                continue;
        }
    
        key.clear();
        value.clear();
		std::stringstream   linestream(line);
        getline( linestream, key, '=');
        linestream  >> value;
    
        if ( key.empty() || value.empty() ) {
            fprintf( stderr, "Invalid config:%s\n", line.c_str() );
            continue;
        }
    
        kv[StringUtil::TrimString(key)] = StringUtil::TrimString(value);
    }
  
    fp.close();
    return true;
}

bool FilekvParser::Parse(const std::string file, std::string &str)
{
	std::ifstream t(file.c_str());
	str.append((std::istreambuf_iterator<char>(t)),std::istreambuf_iterator<char>());
	return true;
}
