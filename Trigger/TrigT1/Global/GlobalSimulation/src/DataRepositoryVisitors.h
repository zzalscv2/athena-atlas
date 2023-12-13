//  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#ifndef GLOBSIM_DATAREPOSITORYVISITORS_H
#define GLOBSIM_DATAREPOSITORYVISITORS_H

#include "DataRepository.h"
#include "IDataRepositoryVisitor.h"
#include "GlobalData.h"
#include "GlobalData.tpl"
#include <ostream>

/*
 * This file contains implentations of IDataRepositoryVisitor
 *
 * Such visitors are used for type-safe and type-complete
 * examination of the contents of DataRepository.
 *
 */
namespace GlobalSim {

  // A visitor useful for debugging.
  class SimpleRepositoryDumper: public IDataRepositoryVisitor {

  public:
  SimpleRepositoryDumper(std::ostream& os) : m_os{os} {}
  virtual ~SimpleRepositoryDumper() = default;

    virtual void process(const std::vector<GSInputTOBArray>& v) override {
      m_os << "No of InputTOB Arrays " << v.size() << '\n';
      for (const auto& e : v) {
	m_os << e.sn() << " size " << e.data()->size() << '\n';
      }
      m_os << '\n';
      m_os.flush();
    }

    virtual void process(const std::vector<GSTOBArray>& v) override {
      m_os << "No of TOBArrays " << v.size() << '\n';
      for (const auto& e : v) {
	m_os << e.sn() << " size " << e.data().size() << '\n';
      }
      m_os << '\n';
      m_os.flush();

    }

    virtual void process(const std::vector<GSTOBArrayPtrVec>& v) override {
      m_os << "No of vectors of TOBArrays " << v.size() << '\n';
      for (const auto& e : v) {
	m_os << e.sn() << " size " << (e.data()).size() << '\n';
      }
      m_os << '\n';
      m_os.flush();  
    }

    virtual void process(const std::vector<GSCount>& v)  override {
      m_os << "No of Count objects " << v.size() << '\n';
      for (const auto& e : v) {
	m_os << e.sn() << " Count\n";
      }
      m_os << '\n';
      m_os.flush();
    }
    
    virtual void process(const std::vector<GSDecision>& v)  override {
      m_os << "No of Decision objects " << v.size() << '\n';
      for (const auto& e : v) {
	m_os << e.sn() << " Decision\n";
      }
      m_os << '\n';
      m_os.flush();
    }
    
  private:
    std::ostream& m_os;
    
  };

}

#endif
