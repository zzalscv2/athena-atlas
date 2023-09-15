/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef EVENT_LOOP__I_INPUT_MODULE_ACTIONS_H
#define EVENT_LOOP__I_INPUT_MODULE_ACTIONS_H

#include <Rtypes.h>
#include <optional>
#include <string>

class StatusCode;

namespace EL
{
  struct EventRange;

  namespace Detail
  {
    struct ModuleData;


    /// @brief the actions that @ref Module::processInputs can perform
    ///
    /// The main reason to have this interface is that it is much easier to
    /// write an input module if it can call functions to perform actions
    /// directly (as opposed to e.g. returning which file to open next, etc.).
    ///
    /// There is a canonical implementation of this interface that is used in
    /// the worker, but I'm using an abstract interface to decouple the input
    /// modules from the actual worker implementation.  In addition, this makes
    /// it easier to write tests for input modules, or to employ a decorator
    /// pattern, should the need arise.
    ///
    /// This interface is not frozen, but reflects the need of the input modules
    /// I have defined.

    class IInputModuleActions
    {
    public:

      /// @brief standard virtual destructor
      virtual ~IInputModuleActions () noexcept = default;


      /// \brief process the given event range
      ///
      /// This will update `eventRange` if the end is set to eof
      ///
      /// \par Guarantee
      ///   basic
      /// \par Failures
      ///   file can't be opened\n
      ///   event range exceeds length of file\n
      ///   processing failures
      virtual ::StatusCode processEvents (EventRange& eventRange) = 0;


      /// \brief open the given input file without processing it
      ///
      /// This is mostly to allow the driver to query the number of
      /// events in the input file without processing it, usually to
      /// determine the range of events to process.
      ///
      /// \par Guarantee
      ///   basic
      /// \par Failures
      ///   file can't be opened
      virtual ::StatusCode openInputFile (const std::string& inputFileUrl) = 0;


      /// \brief the number of events in the input file
      /// \pre inputFile() != 0
      [[nodiscard]] virtual Long64_t inputFileNumEntries () const = 0;
    };
  }
}

#endif
