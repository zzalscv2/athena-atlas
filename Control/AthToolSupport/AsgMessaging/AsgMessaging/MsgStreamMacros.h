/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASGMESSAGING_MSGSTREAMMACROS_H
#define ASGMESSAGING_MSGSTREAMMACROS_H

// Local include(s):
#include "AsgMessaging/MsgLevel.h"

// Pull in the definition from Athena:
#ifndef XAOD_STANDALONE
#   include "AthenaBaseComps/AthMsgStreamMacros.h"
#else // not XAOD_STANDALONE

#include "CxxUtils/normalizeFunctionName.h"

// Not sure why this is needed...
#undef ERROR

// This is a GCC extension for getting the name of the current function.
#if defined( __GNUC__ )
#   define MSGSTREAM_FNAME CxxUtils::normalizeFunctionName(__PRETTY_FUNCTION__)
#else
#   define MSGSTREAM_FNAME ""
#endif

/// Common prefix for the non-usual messages
///
/// The idea is that a regular user usually only wants to see DEBUG, INFO
/// and some WARNING messages. So those should be reasonably short. On the other
/// hand serious warnings (ERROR, FATAL) messages should be as precise
/// as possible to make debugging the issue easier.
///
#define MSGSTREAM_REPORT_PREFIX \
   __FILE__ << ":" << __LINE__ << " (" << MSGSTREAM_FNAME << "): "

/// Macro used to print "serious" messages
#define ATH_MSG_LVL_SERIOUS( lvl, xmsg )                    \
   msg( lvl ) << MSGSTREAM_REPORT_PREFIX << xmsg << endmsg

/// Macro used to print "regular" messages
#define ATH_MSG_LVL_NOCHK( lvl, xmsg )          \
   msg( lvl ) << xmsg << endmsg

/// Macro used to print "protected" messages
#define ATH_MSG_LVL( lvl, xmsg )                \
   do {                                         \
      if( msg().msgLevel( lvl ) ) {             \
         ATH_MSG_LVL_NOCHK( lvl, xmsg );        \
      }                                         \
   } while( 0 )

/// Macro printing verbose messages
#define ATH_MSG_VERBOSE( xmsg )  ATH_MSG_LVL( MSG::VERBOSE, xmsg )
/// Macro printing debug messages
#define ATH_MSG_DEBUG( xmsg )    ATH_MSG_LVL( MSG::DEBUG, xmsg )
/// Macro printing info messages
#define ATH_MSG_INFO( xmsg )     ATH_MSG_LVL_NOCHK( MSG::INFO,  xmsg )
/// Macro printing warning messages
#define ATH_MSG_WARNING( xmsg )  ATH_MSG_LVL_NOCHK( MSG::WARNING, xmsg )
/// Macro printing error messages
#define ATH_MSG_ERROR( xmsg )    ATH_MSG_LVL_SERIOUS( MSG::ERROR, xmsg )
/// Macro printing fatal messages
#define ATH_MSG_FATAL( xmsg )    ATH_MSG_LVL_SERIOUS( MSG::FATAL, xmsg )
/// Macro printing messages that should always appear
#define ATH_MSG_ALWAYS( xmsg )   ATH_MSG_LVL_NOCHK( MSG::ALWAYS, xmsg )

/// can be used like so: ATH_MSG(INFO) << "hello" << endmsg;
#define ATH_MSG(lvl) \
  if( msg().msgLevel( MSG::lvl ) ) msg(MSG::lvl)

#endif // not XAOD_STANDALONE
#endif // ASGMESSAGING_MSGSTREAMMACROS_H
