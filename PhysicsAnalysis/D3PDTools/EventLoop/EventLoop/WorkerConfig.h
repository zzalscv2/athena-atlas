/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef EVENT_LOOP__WORKER_CONFIG_H
#define EVENT_LOOP__WORKER_CONFIG_H

#include <EventLoop/Global.h>

#include <AsgTools/SgTEventMeta.h>
#include <TObject.h>

namespace EL
{
  namespace Detail
  {
    class ModuleData;
  }

  class PythonConfigBase;


  class WorkerConfig final : public TObject
  {
    /// Public Members
    /// ==============

  public:

    /// \brief access the meta store in the input file
    [[nodiscard]] const asg::SgTEventMeta *metaStore() const noexcept;


    /// \brief add the given component
    void add (const PythonConfigBase& config);



    /// Internal/Detail Members
    /// =======================

  public:

    WorkerConfig (Detail::ModuleData *val_data) noexcept;
    ~WorkerConfig () noexcept;



    /// Private Members
    /// ===============

  private:

    Detail::ModuleData *m_data = nullptr; //!
    asg::SgTEventMeta m_metaStore; //!

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Winconsistent-missing-override"
    ClassDef(WorkerConfig, 1);
#pragma GCC diagnostic pop
  };



  /// Inline/Template Functions
  /// =========================

  [[nodiscard]] inline const asg::SgTEventMeta *WorkerConfig ::
  metaStore() const noexcept
  {
    return &m_metaStore;
  }
}

#endif
