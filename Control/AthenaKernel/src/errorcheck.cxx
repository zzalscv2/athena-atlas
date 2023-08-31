/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file  errorcheck.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2006
 * @brief Helpers for checking error return status codes and reporting errors.
 */

#include "AthenaKernel/errorcheck.h"
#include "AthenaKernel/getMessageSvc.h"
#include "CxxUtils/normalizeFunctionName.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/INamedInterface.h"
#include <ctype.h>


namespace errorcheck {


/// If true, hide the source file and line number in output messages.
std::atomic<bool> ReportMessage::s_hide_error_locus;


/// If true, hide the function names in output messages.
std::atomic<bool> ReportMessage::s_hide_function_names;


/**
 * @brief Shorten filename
 * @param file The full file path
 * @param pkg The package name (with optional version)
 *
 * @return Returns the file path starting from the package name
 */
std::string munge_filename (const std::string& file, const std::string& pkg)
{
  // Extract package name in case there is a version (MyPackage-00-00-00)
  const std::string p = pkg.substr(0, pkg.find('-'));
  if (!p.empty()) {
    // Find package name in path and remove any leading entries
    std::string::size_type ipos = file.find("/"+p+"/");
    if (ipos != std::string::npos) {
      return file.substr(ipos+1, std::string::npos);
    }
  }
  return file;
}


/**
 * @brief Constructor.
 * @param level The error logging level for the message.
 * @param line The source line from which the report is being made.
 * @param file The source file name from which the report is being made.
 * @param func The name of the function from which the report is being made.
 * @param pkg The name of the package from which the report is being made.
 * @param context The name of the context (algorithm/tool/service/etc.)
 *                from which the report is being made.
 * @param sc The @c StatusCode to include in the error message.
 */
ReportMessage::ReportMessage (MSG::Level level,
                              int line,
                              const char* file,
                              const char* func,
                              const char* pkg,
                              const std::string& context,
                              StatusCode sc)
  : MsgStream (Athena::getMessageSvc(), context)
{
  // The common part.
  format_common (level, line, file, func, pkg);

  // The status code.
  *this << ": code " << sc;

  // Remember the end of the header.
  m_pos = stream().str().size();
}


/**
 * @brief Constructor.
 * @param level The error logging level for the message.
 * @param line The source line from which the report is being made.
 * @param file The source file name from which the report is being made.
 * @param func The name of the function from which the report is being made.
 * @param pkg The name of the package from which the report is being made.
 * @param context The name of the context (algorithm/tool/service/etc.)
 *                from which the report is being made.
 */
ReportMessage::ReportMessage (MSG::Level level,
                              int line,
                              const char* file,
                              const char* func,
                              const char* pkg,
                              const std::string& context)
  : MsgStream (Athena::getMessageSvc(), context)
{
  // The common part.
  format_common (level, line, file, func, pkg);

  // Remember the end of the header.
  m_pos = stream().str().size();
}


/**
 * @brief Generate the common header for messages.
 * @param level The error logging level for the message.
 * @param line The source line from which the report is being made.
 * @param file The source file name from which the report is being made.
 * @param func The name of the function from which the report is being made.
 * @param pkg The name of the package from which the report is being made.
 */
void ReportMessage::format_common (MSG::Level level,
                                   int line,
                                   const char* file,
                                   const char* func,
                                   const char* pkg)
{
  // Logging level.
  *this << level;

  // Write the source file/line.
  if (s_hide_error_locus)
    *this << "FILE:LINE";
  else {
    if (pkg && pkg[0] != '\0')
      *this << munge_filename(file, pkg) << ":" << line;
    else
      *this << file << ":" << line;
  }

  // Include the function name if available.
  if (s_hide_function_names)
    *this << " (FUNC)";
  else {
    if (func && func[0] != '\0') {
      *this << " (" << CxxUtils::normalizeFunctionName(func) << ")";
    }
  }
}


/**
 * @brief Destructor.
 * This will cause the message to be emitted, if it hasn't already been.
 */
ReportMessage::~ReportMessage()
{
  // Don't do anything if the message has already been emitted
  // (due to using << endmsg, for example).
  if (!stream().str().empty())
    *this << endmsg;
}


/**
 * @brief Emit the message.
 * We override this method from @c MsgStream in order to fix up
 * the message punctuation.
 */
MsgStream& ReportMessage::doOutput()
{
  // The deal here is this.  We want
  //  REPORT_MESSAGE(MSG::INFO) << "foo";
  // to get a `: ' before the `foo'.
  // But we don't want
  //  REPORT_MESSAGE(MSG::INFO);
  // to have a trailing `: '.
  // So, after we generate the common header part, we remember
  // where we are in the string.  When the message is emitted, we look
  // to see if additional text has been added.  If not, then we don't
  // need to do anything.  But if so, we insert a `: ' after the header.
  if (m_pos != stream().str().size()) {
    std::string tmp1 = stream().str();
    std::string tmp2 = tmp1.substr(0, m_pos);
    tmp2 += ": ";
    tmp2.append( tmp1, m_pos);
    stream().str (tmp2);
  }
  return MsgStream::doOutput();
}


/**
 * @brief If set to true, hide the source file and line number
 *        in the output.
 *
 *        This is intended for use in regression tests, where
 *        it's undesirable to have the output change if source lines
 *        are added or deleted.
 */
void ReportMessage::hideErrorLocus (bool flag /*= true*/)
{
  s_hide_error_locus = flag;
}


/**
 * @brief If set to true, hide function names in the output.
 *        in the output.
 *
 *        This is intended for use in regression tests, where
 *        function names may be formatted differently on different
 *        platforms.
 */
void ReportMessage::hideFunctionNames (bool flag /*= true*/)
{
  s_hide_function_names = flag;
}


} // namespace errorcheck


namespace errorcheck {


/**
 * @brief Return the context name from a context (@c this) pointer.
 * @param context The context from which to get the name.
 * @return The name of this context.
 *
 * This is the specialization for an @c INamedInterface.
 */
std::string context_name (const INamedInterface* context)
{
  return context->name();
}


/**
 * @brief Return the context name from a context (@c this) pointer.
 * @param context The context from which to get the name.
 * @return The name of this context.
 *
 * This is the specialization for an unknown context type; it returns
 * a blank name.
 */
std::string context_name (const void* /*context*/)
{
  return "";
}


} // namespace errorcheck
