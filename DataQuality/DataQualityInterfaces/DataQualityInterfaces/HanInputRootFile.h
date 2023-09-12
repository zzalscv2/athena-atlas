/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef dqiHanInputRootFile_h
#define dqiHanInputRootFile_h

#include <TDirectory.h>
#include <TKey.h>
#include <TObject.h>

#include <string>
#include <vector>

#include "dqm_core/InputRootFile.h"

namespace dqi {

class HanInputRootFile: public dqm_core::InputRootFile {
public:

  HanInputRootFile( const std::string& rootFileName, const std::string& path = "" );
  ~HanInputRootFile();

  virtual void addListener( const boost::regex& regex, dqm_core::InputListener* listener ) override;

  virtual void addListener( const std::vector<std::string>& names, dqm_core::InputListener* listener ) override;

  virtual void addListener( const std::string& name, dqm_core::InputListener *listener ) override;

  TFile* file() const { return m_file.get(); }

  const TDirectory* getBasedir() const { return m_basedir; }
  TDirectory* getBasedir() { return m_basedir; }

protected:

  TDirectory*           m_basedir;

  std::vector<std::string> m_histNames;
  bool                     m_histNamesBuilt;

};

} // namespace dqi

#endif
