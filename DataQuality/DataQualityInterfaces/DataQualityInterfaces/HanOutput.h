/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef dqiHanOutput_h
#define dqiHanOutput_h

#include <TFile.h>
#include <TTree.h>

#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <set>
#include <string>

#include "dqm_core/Output.h"
#include "CxxUtils/checker_macros.h"

class TDirectory;

namespace dqm_core
{
  class Region;
}

namespace dqi
{

  // Converts sequense, containing TDirectories and strings to JSON file
  /**
   * @brief Converts sequense, containing TDirectories and strings to JSON file
   */
  nlohmann::ordered_json to_JSON(TSeqCollection* tseq);

  class HanConfig;

  class ATLAS_NOT_THREAD_SAFE HanOutput : public dqm_core::Output
  {
  public:
    enum Version { V1 = 1, V2 = 2 };

    Version HanOutput_FileVersion = V2;

    typedef std::map<std::string, TSeqCollection*> DQOutputMap_t;

    HanOutput(const std::string& rootFileName, DQOutputMap_t* outputMap, TSeqCollection* outputList);

    virtual ~HanOutput();

    virtual void addListener(const std::string& name, dqm_core::OutputListener* listener);

    virtual void addListener(const dqm_core::Parameter& parameter, dqm_core::OutputListener* listener);

    virtual void publishResult(const std::string& name, const dqm_core::Result& result);

    virtual void flushResults();

    virtual void activate();

    virtual void deactivate();

    virtual void setConfig(HanConfig* config);

    virtual void publishMissingDQPars();

    virtual void setInput(TDirectory* input);

  protected:
    class Result
    {
    public:
      Result(TDirectory* dir);
      Result(const Result&) = delete;
      Result& operator=(const Result&) = delete;
      virtual ~Result();
      virtual void fill(const dqm_core::Result& result);
      virtual void write();

    protected:
      void copyString(char* to, const std::string& from);
      std::unique_ptr<TTree> m_result;
      char* m_status;
      static const int s_charArrSize;
    };

    class RegionNameComp
    {
    public:
      bool operator()(const dqm_core::Region* a, const dqm_core::Region* b) const;
    };

    typedef std::map<std::string, dqm_core::Region*> DQParMap_t;
    typedef std::multimap<dqm_core::Region*, std::string, RegionNameComp> DQRegMap_t;
    typedef std::map<dqm_core::Region*, int, RegionNameComp> DQRegCount_t;
    typedef std::map<std::string, dqm_core::Result*> DQResultMap_t;
    typedef std::set<std::string> DQParSet_t;

    std::string m_fileName;
    std::unique_ptr<TFile> m_file;
    bool m_retainUnpubData;

    DQParSet_t m_unpublishedDQPars;
    DQParMap_t m_dqPars;
    DQRegMap_t m_dqRegs;
    DQRegCount_t m_dqRegCounts;
    DQResultMap_t m_dqResults;
    DQOutputMap_t* m_outputMap;

    TSeqCollection* m_outputList;
    HanConfig* m_config;
    DQParSet_t m_regexlist;
    TDirectory* m_input;

  private:
    HanOutput();
  };

}  // namespace dqi

#endif
