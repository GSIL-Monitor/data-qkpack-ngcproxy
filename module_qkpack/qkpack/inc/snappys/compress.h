#ifndef __COMPRESS_H__
#define __COMPRESS_H__

#include "common.h"

namespace com { 
namespace youku { 
namespace data {
namespace qkpack {
namespace snappys {


class ICompress 
{
public:
	ICompress(){};
	virtual ~ICompress(){};

	virtual int Compress(const std::string &src,std::string &dest){
        (void)src;
		(void)dest;
		return 0;
    }
		
	virtual bool Uncompress(const std::string &src,std::string &dest){
        (void)src;
		(void)dest;
		return 0;
    }
	
	virtual bool IsValidCompressed(const std::string &src){
		(void)src;
		return 0;
	}
	
};

} /* namespace snappys */
} /* namespace qkpack */
} /* namespace data */
} /* namespace youku */
} /* namespace com */

#endif //__COMPRESS_H__
