#ifndef __SNAPPY_COMPRESS_H__
#define __SNAPPY_COMPRESS_H__

#include "snappys/compress.h"
#include "common.h"

namespace com { 
namespace youku { 
namespace data {
namespace qkpack {
namespace snappys {


class SnappyCompress : public ICompress
{
public:
	SnappyCompress(){};
	virtual ~SnappyCompress(){};

	int Compress(const std::string &src,std::string &dest);
		
	bool Uncompress(const std::string &src,std::string &dest);
	
	bool IsValidCompressed(const std::string &src);
	
};

} /* namespace snappys */
} /* namespace qkpack */
} /* namespace data */
} /* namespace youku */
} /* namespace com */

#endif //__SNAPPY_COMPRESS_H__
