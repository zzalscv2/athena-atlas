/*
 * Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration.
 */


#ifndef ASG_DATA_HANDLES_SHALLOWCOPYDECORDEPS_H
#define ASG_DATA_HANDLES_SHALLOWCOPYDECORDEPS_H

#ifndef XAOD_STANDALONE
#include "StoreGate/ShallowCopyDecorDeps.h"
#else

#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/WriteHandleKey.h"
#include "AsgTools/CurrentContext.h"


namespace SG {


/**
 * @brief standalone (dummy) implementation of `ShallowCopyDecorDeps`
 *
 * This doesn't do anything in AnalysisBase and is just provided to
 * allow components to use it in AthenaMT while staying dual-use.
 */
template <class T>
class ShallowCopyDecorDeps
{
public:
  /**
   * @brief Auto-declaring Property constructor.
   * @param owner Owning component.
   * @param name Name of the Property.
   * @param l Default list of decorations to propagate.
   * @param doc Documentation string.
   */
  template <class OWNER>
  ShallowCopyDecorDeps (OWNER* owner,
                        const std::string& name,
                        std::initializer_list<std::string> l,
                        const std::string& doc = "");


  /**
   * @brief Initialize this property.  Call this from initialize().
   * @param origKey Key for the source of the shallow copy.
   * @param copyKey Key for the result of the shallow copy.
   * @param used If false, then this handle is not to be used.
   *             Instead of normal initialization, the keys will be cleared.
   */
  StatusCode initialize (const SG::ReadHandleKey<T>& origKey,
                         const SG::WriteHandleKey<T>& copyKey,
                         bool used = true);


  /**
   * @brief Create alias for the decorations, linked to the shallow copy.
   * @param origKey Key for the source of the shallow copy.
   * @param ctx The current EventContext.
   *
   * Call this after the shallow copy has been recorded in SG.
   */
  StatusCode linkDecors (const SG::ReadHandleKey<T>& origKey,
                         const EventContext& ctx = Gaudi::Hive::currentContext()) const;
};


template<class T> template <class OWNER> ShallowCopyDecorDeps<T> ::
ShallowCopyDecorDeps (OWNER* /*owner*/,
                      const std::string& /*name*/,
                      std::initializer_list<std::string> /*l*/,
                      const std::string& /*doc*/)
{}


template<class T> StatusCode ShallowCopyDecorDeps<T> ::
initialize (const SG::ReadHandleKey<T>& /*origKey*/,
            const SG::WriteHandleKey<T>& /*copyKey*/,
            bool /*used*/)
{
  return StatusCode::SUCCESS;
}


template<class T> StatusCode ShallowCopyDecorDeps<T> ::
linkDecors (const SG::ReadHandleKey<T>& /*origKey*/,
            const EventContext& /*ctx*/) const
{
  return StatusCode::SUCCESS;
}

} // namespace SG

#endif

#endif
