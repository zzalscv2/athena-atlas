/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#ifndef ASG_DATA_HANDLES_READ_DECOR_HANDLE_KEY_ARRAY_H
#define ASG_DATA_HANDLES_READ_DECOR_HANDLE_KEY_ARRAY_H

#ifndef XAOD_STANDALONE
#include <StoreGate/ReadDecorHandleKeyArray.h>
#else

#include "AsgDataHandles/DecorHandleKeyArray.h"

#include "AsgDataHandles/ReadDecorHandleKey.h"
#include "AsgDataHandles/ReadDecorHandle.h"

namespace SG {

  /**
   * @class SG::ReadDecorHandleKeyArray<T>
   * @brief class to hold an array of ReadDecorHandleKeys
   *
   * See StoreGate/ReadDecorHandleKeyArray for details.
   *
   */
  template <class T, class S = float>
  using ReadDecorHandleKeyArray = DecorHandleKeyArray<ReadDecorHandle<T, S>,ReadDecorHandleKey<T>/*, Gaudi::DataHandle::Writer*/ >;

} // namespace SG

#endif

#endif
