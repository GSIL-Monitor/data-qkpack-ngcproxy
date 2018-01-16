#include "snappys/snappy_compress.h"
#include <snappy.h>
#include <iostream>
#include <sstream>

#include "snappys/snappystream.h"


using namespace com::youku::data::qkpack::snappys;


/**
 * 
 * 压缩
 * 
*/
int SnappyCompress::Compress(const std::string &src,std::string &dest) 
{
	std::ostringstream					ostr;
	snappys::oSnappyStream				osnstrm(ostr);
	
	osnstrm << src.c_str();
	osnstrm.flush();
	ostr.flush();

	dest = ostr.str();
	
	return dest.length();
}


/**
 * 
 * 解压
 * 
*/
bool SnappyCompress::Uncompress(const std::string &src,std::string &dest) 
{
	std::stringstream					ss;
	std::istringstream					isstr(src,std::ios_base::in);
	snappys::iSnappyStream				isnstrm(isstr);
	
	ss << isnstrm.rdbuf();
	ss.flush();
	
	if ( !ss.str().length()  ) {
		return false;
	}

	dest = ss.str();
	return true;
}


/**
 * 
 * 验证是否是压缩
 * 
*/
bool SnappyCompress::IsValidCompressed(const std::string &src) 
{
	if ( memcmp (src.c_str(),"snappy\0",7) == 0 ) {
		return true;
	}
	
	return false;
}

