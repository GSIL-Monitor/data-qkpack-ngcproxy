#ifndef __SNSTRM_QRRCONFIG_H__
#define __SNSTRM_QRRCONFIG_H__


#if defined(SNSTRM_SHARED) && (defined(_WIN32) || defined(WIN32))
#	if defined()
#		define SNSTRM_API __declspec(dllexport)
#	else
#		define SNSTRM_API __declspec(dllimport)
#	endif
#else
#	define SNSTRM_API
#endif
#define SNSTRM_VERSION_MAJOR 0
#define SNSTRM_VERSION_MINOR 2
#define SNSTRM_VERSION_PATCH 1
/* #undef SNSTRM_WITH_BOOST_IOSTREAMS */


namespace com { 
namespace youku { 
namespace data {
namespace qkpack {
namespace snappys {


class Config{
	public:
		static const char magic[];
		static const int magic_sz;
		static const unsigned defaultChunkSize = 1 << 15;
};


} /* namespace snappys */
} /* namespace qkpack */
} /* namespace data */
} /* namespace youku */
} /* namespace com */


#endif // __SNSTRM_QRRCONFIG_H__

