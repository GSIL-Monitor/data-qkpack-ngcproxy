#include "util/snprintf.h"

int safe_snprintf(char *buf, size_t size, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	int ret = safe_vsnprintf(buf, size, fmt, ap);  // µ÷ÓÃ acl::vsnprintf
	va_end(ap);
	return ret;
}

int safe_vsnprintf(char *buf, size_t size, const char *fmt, va_list ap)
{
	return ::vsnprintf(buf, size, fmt, ap);
}

