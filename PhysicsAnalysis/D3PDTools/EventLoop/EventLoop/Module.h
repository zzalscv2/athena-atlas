/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef EVENT_LOOP__MODULE_H
#define EVENT_LOOP__MODULE_H

#include <EventLoop/Global.h>

#include <AsgMessaging/StatusCode.h>

namespace EL
{
  namespace Detail
  {
    class IInputModuleActions;


    /// \brief the base class for EventLoop instrumentation module
    ///
    /// These are **internal** modules for EventLoop that allow to
    /// factor out non-core functionalities into separate modules.
    /// The goal here is a separation of concern, allowing to
    /// implement these modules to be defined without having to worry
    /// about the internals of EventLoop, and conversely to allow
    /// implementing the core functionality of EventLoop without
    /// having the code interwoven with a lot auxilliary code.
    ///
    /// \warn The module interface is not considered stable and it can
    /// change without warning from one release to the next.  This
    /// interface is really meant to be completely internal and users
    /// are not meant to mess with it.
    ///
    /// \warn While I currently do not stream modules, the possibility
    /// is anticipated and all modules ought to be written as if they
    /// were to be streamed one day.

    class Module
    {
      /// \brief standard (virtual) destructor
    public:
      virtual ~Module () noexcept = default;


      /// \brief action at the the very beginning of the worker job
      ///
      /// This gets called as early as possible in the worker
      /// initialization.  Essentially all that should happen before
      /// this is to load all the modules.  The main purpose is to
      /// start any benchmarks that are meant to capture the
      /// initialization process as well.
    public:
      virtual ::StatusCode firstInitialize (ModuleData& data);


      /// \brief action before opening the first file in the worker
      /// job
      ///
      /// This is mostly meant to allow loading the dictionaries
      /// before any files and associated information is loaded.
    public:
      virtual ::StatusCode preFileInitialize (ModuleData& data);


      /// \brief action just before algorithms are initialized
      ///
      /// This is typically used for any setup that this module needs
      /// to do.
    public:
      virtual ::StatusCode onInitialize (ModuleData& data);


      /// \brief process all input files
      ///
      /// This deviates slightly from the usual pattern for module functions in
      /// that I pass in the possible actions as an argument.  See @ref
      /// IInputModuleActions for details.
    public:
      virtual StatusCode processInputs (ModuleData& data, IInputModuleActions& actions);


      /// \brief action after processing first event
      ///
      /// This is mostly meant to set up benchmarks that record
      /// per-event performance.  While a lot of initialization
      /// happens during `initialize()` there is a fair amount of
      /// initialization that happens on the first event, so when
      /// recording per-event performance the first event is sort of
      /// "special" and may need to be omitted.
    public:
      virtual ::StatusCode postFirstEvent (ModuleData& data);


      /// \brief actions after opening a new input file
    public:
      virtual ::StatusCode onNewInputFile (ModuleData& data);


      /// \brief actions before closing an input file
    public:
      virtual ::StatusCode onCloseInputFile (ModuleData& data);


      /// \brief actions after `CloseInputFile` is called on the
      /// algorithms
      ///
      /// Right now that is only used to disconnect the `TEvent`
      /// object from the input file.
    public:
      virtual ::StatusCode postCloseInputFile (ModuleData& data);


      /// \brief actions just before `fileExecute` is called on
      /// algorithms
    public:
      virtual ::StatusCode onFileExecute (ModuleData& data);


      /// \brief actions just before `execute` is called on
      /// algorithms
      ///
      /// For now that is mostly used to point input modules to the
      /// right event
    public:
      virtual ::StatusCode onExecute (ModuleData& data);


      /// \brief actions just before algorithms are finalized
    public:
      virtual ::StatusCode onFinalize (ModuleData& data);


      /// \brief actions after algorithms have been finalized
      ///
      /// This is usually used to calculate some job summary
      /// information that is to be stored it in the histogram output
      /// file.
    public:
      virtual ::StatusCode postFinalize (ModuleData& data);


      /// \brief action at the end of the worker job
      ///
      /// This is mostly meant/used to print job summary statements at
      /// the very end of worker job, and actually have them show up
      /// at or at least near the end of any log file.
    public:
      virtual ::StatusCode onWorkerEnd (ModuleData& data);

    /// \brief action at of the worker job
    ///
    /// Executed just right before worker exit successfully
    /// at this stage all outputs have been created 
    public: 
      virtual ::StatusCode postFileClose(ModuleData& data);


      /// \brief report that we failed to open our input file
    public:
      virtual void reportInputFailure (ModuleData& data);
    };
  }
}

#endif
