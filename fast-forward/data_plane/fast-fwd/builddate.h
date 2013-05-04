#ifndef _BUILDDATE_H
#define _BUILDDATE_H

#define FASTFWD_VERSION_STR "Fast-fwd: V1.1.0"

#ifdef OCTEON_DEBUG_LEVEL
#define FASTFWD_BUILD_NUMBER_STR "(Development build)"
#else
#define FASTFWD_BUILD_NUMBER_STR "(Released build)"
#endif

#endif
