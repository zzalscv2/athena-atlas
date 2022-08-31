/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef __CONDITIONSSINGLETON_H
#define __CONDITIONSSINGLETON_H
// sami
#include <string>
#include <vector>
//#include <utility>
#include <map>
class TMap;

namespace dqi{
  class ConditionsSingleton
  {
  private:
    ConditionsSingleton() = default;
    ~ConditionsSingleton() = default;
    ConditionsSingleton(const ConditionsSingleton &) = delete;
    ConditionsSingleton & operator=(const ConditionsSingleton &) = delete;
    int m_numRefHisto{0};
    std::string m_currentConditions;
    std::map<std::string,std::string> m_referenceMap;
    const TMap* m_refsourcedata{nullptr};
  public:
    static ConditionsSingleton &getInstance();
    void makeConditionMap(std::map<std::string, std::string>& cmap,
			  const std::string& condition);
    bool conditionsMatch(std::map<std::string, std::string>& refConds,
			 std::map<std::string, std::string>& currentConds) const;
    std::string conditionalSelect(std::string inp,const std::string& condition);
    void setCondition(const std::string& c);
    const std::string& getCondition() const;
    int getNumReferenceHistos() const;
    std::string getNewRefHistoName();
    std::vector<std::string> getAllReferenceNames(std::string inp) const;
    std::vector<std::pair<std::string,std::string> > getConditionReferencePairs(std::string inp) const;
    void setNewReferenceName(const std::string&,const std::string&);
    std::string getNewReferenceName(const std::string&,bool quiet=false) const;
    void setRefSourceMapping(const TMap* refsourcedata);
    std::string getRefSourceData(const std::string& rawref) const;
  };
}

#endif
