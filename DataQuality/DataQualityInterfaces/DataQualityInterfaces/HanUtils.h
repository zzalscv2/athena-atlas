/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TCollection.h"
#include "TNamed.h"

#include <mutex>

class TSeqCollection;
class TKey;
class TDirectory;

namespace dqi {
  TSeqCollection* newTList( const char *name, TObject *obj = 0 );
  TSeqCollection* newTObjArray( const char *name, TObject *obj = 0, Int_t size = TCollection::kInitCapacity);
  TKey* getObjKey( TDirectory* dir, const std::string& path );
  void dolsr(const TDirectory* dir, std::vector<std::string>& hists, const TDirectory* topdir = nullptr);

  class HanHistogramLink : public TNamed {
  public:
    HanHistogramLink(TDirectory* dir, const std::string& path);
    TObject* getObject();
  private:
    TDirectory* m_dir;
    std::string m_path;
  };


  class DisableMustClean {
  public:
    DisableMustClean();
    ~DisableMustClean();
  private:
    std::unique_lock<std::mutex> m_lock;
    bool m_useRecursiveDelete;
  };
}
