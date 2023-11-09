/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file CxxUtils/SealDebug.h
 * @author Lassi Tuura (original author)
 * @author Wim Lavrijsen <WLavrijsen@lbl.gov> (responsible in ATLAS)
 * @date Oct, 2008
 * @brief This are the SEAL debug aids, adapted to build in Atlas,
 *        after the drop of that project.
 *
 *        Search for `wlav' to find changes from the SEAL version. I
 *        also dropped all ASSERT macro's in favor of assert. Removed
 *        logstream references.
 *
 *        sss: Add stacktraceLine.
 *        fwinkl: Add enableCoreFiles, disableCoreFiles.
 */

#ifndef CXXUTILS_SEAL_DEBUG_H // wlav SEAL_BASE_DEBUG_H
#define CXXUTILS_SEAL_DEBUG_H // wlav SEAL_BASE_DEBUG_H

#include "CxxUtils/checker_macros.h"
#include "CxxUtils/SealCommon.h"  // sss -- needed for IOFD
# include <cstddef>
# include <atomic>

// wlav copied from SealBase/sysapi/DebugAids.h
// Windows doesn't have this, so fake a suitable substitute
# ifdef _WIN32
#  define STDERR_HANDLE GetStdHandle (STD_ERROR_HANDLE)
# else
#  define STDERR_HANDLE STDERR_FILENO
# endif

// Define a suitable wrapper to write to system file descriptors.
// This is needed because on Windows we are using HANDLEs, not the
// compiler's crippled posixy interface.
# ifdef _WIN32
#  define MYWRITE(fd,data,n)    do { DWORD written; WriteFile(fd,data,n,\
                                        &written,0); } while (0)
# else
#  define MYWRITE(fd,data,n)    write(fd,data,n)
# endif


namespace Athena {                             // wlav


// wlav copied from SealBase/BitTraits.h
/** Describe the bit features of an integral type @c T. */
template <class T>
struct BitTraits
{
    /// Number of bits in @c T.
    enum { Bits		= sizeof (T) * CHAR_BIT };

    /// Number of 8-bit bytes in @c T.
    enum { Bytes	= Bits / 8 + ((Bits % 8) > 0) };

    /// Number of base-10 digits in @c T (without leading sign).
    enum { Digits	= (Bits * 30103) / 100000 + 1 };
    // 30103 =~ M_LN2 / M_LN10 * 100000

    /// Number of base-16 digits in @c T (without leading sign).
    enum { HexDigits	= Bits / 4 + ((Bits % 4) > 0) };
};


/** Utilities for debugging support.  */
class DebugAids
{
public:
    // Miscellaneous functions
    static IOFD			stacktraceFd (IOFD fd = IOFD_INVALID);
    static void			stacktrace ATLAS_NOT_THREAD_SAFE (IOFD fd = IOFD_INVALID);
    static void			coredump (int sig, ...);
    // sss
    static void                 stacktraceLine ATLAS_NOT_THREAD_SAFE (IOFD fd,
                                                                      unsigned long addr);
    static void                 setStackTraceAddr2Line ATLAS_NOT_THREAD_SAFE (const char* path);

    // fwinkl
    static unsigned long        enableCoreFiles();
    static void                 disableCoreFiles();
private:
    static std::atomic<IOFD>	 s_stackTraceFd;
};


} // namespace Athena                             wlav
#endif // CXXUTILS_SEAL_DEBUG_H wlav SEAL_BASE_DEBUG_H
