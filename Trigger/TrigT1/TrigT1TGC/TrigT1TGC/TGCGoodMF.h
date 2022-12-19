/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TGCGoodMF_h
#define TGCGoodMF_h

#include "AthenaBaseComps/AthMessaging.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadCondHandleKey.h"

#include <map>

namespace LVL1TGCTrigger {
class TGCArguments;
}

namespace LVL1TGC {

class TGCGoodMF : public AthMessaging {
 public:
  TGCGoodMF(LVL1TGCTrigger::TGCArguments*,const std::string& version);
  virtual ~TGCGoodMF() = default;

  TGCGoodMF(const TGCGoodMF& right) = default;
  const TGCGoodMF& operator=(const TGCGoodMF& right);

  bool test_GoodMF(int moduleId, int sscId, int subsector) const;
  bool readBadMFList();

  const LVL1TGCTrigger::TGCArguments* tgcArgs() const { return m_tgcArgs; }

 private:
  std::map<int, std::map<int,int> > m_mapisgoodMF; //Number of moduleId
  LVL1TGCTrigger::TGCArguments* m_tgcArgs;
  std::string m_verName;

};

}  // end of namespace

#endif // TGCGoodMF_h
