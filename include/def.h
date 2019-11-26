#pragma once

#if defined(unix) || defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
#define OS_UNIX
#elif defined(_WIN32)
#define OS_WIN
#endif

#ifdef OS_UNIX

#define ALWAYS_INLINE inline __attribute__((__always_inline__))

#elif OS_WIN

#define ALWAYS_INLINE __forceinline

#else

#define ALWAYS_INLINE

#endif
