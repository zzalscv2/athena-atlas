/*
Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FEATURE_H
#define FEATURE_H

#include <iostream>
#include <cmath>
#include <string>
#include <bitset>
#include <vector>
#include <map>
#include <queue>

namespace Trig {

  struct TrigBtagEmulationJet {
    double pt;
    double eta;
    double phi;
    std::map< std::string, double > weights;
  };

  bool sortEmulationJet_byPt(Trig::TrigBtagEmulationJet&,Trig::TrigBtagEmulationJet&);

  // ***

  class feature {
  public:
    feature(int type = feature::UNKNOWN,std::string name = "NONE");
    feature(const feature&);
    virtual ~feature();

    std::string name() const;
    virtual bool isPassed() = 0;
    virtual bool evaluateJet(struct TrigBtagEmulationJet*) = 0;
    virtual void clear() = 0;
    virtual void Print() = 0;
    virtual void setCuts(float,float,float) {}

    virtual float getCut() const = 0;

  public:
    int m_type;
    std::string m_name;

    static const int UNKNOWN    = -1;
    static const int THRESHOLD  =  0;
    static const int SELECTION  =  1;
  };

  // *** B-TAG

  class feature_btag : public feature {
  public:
    feature_btag(std::string name = "NONE",float weight = -1000);
    feature_btag(const feature_btag&);
    virtual ~feature_btag();

    virtual bool isPassed();
    virtual bool evaluateJet(struct TrigBtagEmulationJet*);
    virtual void clear();
    virtual void Print();

    virtual float getCut() const;

  public:
    double m_weight;

  public:
    static constexpr double BCOMBLOOSE  = 0.25 ;
    static constexpr double BCOMBMEDIUM = 1.25 ;
    static constexpr double BCOMBTIGHT  = 2.65 ;
    static constexpr double BLOOSE      = 0.25 ;
    static constexpr double BMEDIUM     = 1.25 ;
    static constexpr double BTIGHT      = 2.65 ;

    static constexpr double MV2C2040    = 0.75 ;
    static constexpr double MV2C2050    = 0.50 ;
    static constexpr double MV2C2060    = -0.0224729 ;
    static constexpr double MV2C2070    = -0.509032  ;
    static constexpr double MV2C2077    = -0.764668  ;
    static constexpr double MV2C2085    = -0.938441  ;

    // mv2c10 cuts: https://indico.cern.ch/event/642743/contributions/2612294/attachments/1468496/2271441/BJetTrigTuning_TGM31May2017.pdf
    static constexpr double MV2C1040    = 0.978 ;
    static constexpr double MV2C1050    = 0.948 ;
    static constexpr double MV2C1060    = 0.846 ;
    static constexpr double MV2C1070    = 0.58  ;
    static constexpr double MV2C1077    = 0.162 ;
    static constexpr double MV2C1085    = -0.494;
  };

  // *** ANTI-B-TAG

  class feature_antibtag : public feature_btag {
  public:
    feature_antibtag(std::string name = "NONE",float weight = 1000);
    feature_antibtag(const feature_antibtag&);
    virtual ~feature_antibtag();

    virtual bool evaluateJet(struct TrigBtagEmulationJet*);
    virtual void Print();
  };


  // *** HT
 
  class feature_ht : public feature {
  public:
    feature_ht(std::string triggerLevel = "L1",std::string name = "NONE", float ht = 0);
    feature_ht(const feature_ht&);
    virtual ~feature_ht();

    virtual bool isPassed();
    virtual bool evaluateJet(struct TrigBtagEmulationJet*);
    virtual void clear();
    virtual void Print();
    virtual void setCuts(float,float,float);
    virtual float getCut() const;

    bool satisfyCuts(struct TrigBtagEmulationJet*);

  protected:
    bool isPassed_L1();
    bool isPassed_HLT();

    virtual bool evaluateJet_L1(struct TrigBtagEmulationJet*);
    bool evaluateJet_HLT(struct TrigBtagEmulationJet*);

  public:
    std::string m_trigLevel;
    float m_min_ht;
    float m_count_ht;

  protected:
    float m_cut_pt;
    float m_cut_min_eta;
    float m_cut_max_eta;
    float m_cut_btag;
    std::string m_btagName;
  };

  // *** HT-TOP

  class feature_ht_top : public feature_ht {
  public:
    feature_ht_top(std::string triggerLevel = "L1",std::string name = "NONE", float ht = 0, unsigned int topEt = 0);
    feature_ht_top(const feature_ht_top&);
    virtual ~feature_ht_top();
    
    virtual void clear();
    virtual void Print();

  protected:
    virtual bool evaluateJet_L1(struct TrigBtagEmulationJet*);

  private:
    void calculateHT_L1();

  private:
    std::vector<struct TrigBtagEmulationJet> m_piorityQueue;
    unsigned int m_topEt;
  };

  // *** INVM
  class feature_invm : public feature {
  public:
    feature_invm(std::string triggerLevel = "L1",std::string name = "NONE", float min_invm = 0);
    feature_invm(const feature_invm&);
    virtual ~feature_invm();

    virtual bool isPassed();
    virtual bool evaluateJet(struct TrigBtagEmulationJet*);
    virtual void clear();
    virtual void Print();
    virtual void setCuts(float,float,float);
    virtual float getCut() const;

    bool satisfyCuts(struct TrigBtagEmulationJet*);

  protected:
    void initLUTs();
    double cosh_LUT(double value) const;
    double cos_LUT(double value) const;

  protected:
    virtual bool evaluateJet_L1(struct TrigBtagEmulationJet*);  
    bool evaluateJet_HLT(struct TrigBtagEmulationJet*);  

  protected:
    double calculateINVM(struct TrigBtagEmulationJet const&,struct TrigBtagEmulationJet const&) const;

  protected:
    std::string m_trigLevel;
    double m_min_invm;
    double m_count_invm;
    double m_cut_pt;
    double m_cut_min_eta;
    double m_cut_max_eta;

  protected:
    std::vector<struct TrigBtagEmulationJet> m_jetCollection_HLT;
    std::vector<struct TrigBtagEmulationJet> m_priorityQueue_20;
    std::vector<struct TrigBtagEmulationJet> m_priorityQueue_30;

  protected:
    std::map< int,double > m_LUTcos;
    std::map< int,double > m_LUTcosh;
  };

  // *** INVM CF
  class feature_invm_CF : public feature_invm {
  public:
    feature_invm_CF(std::string triggerLevel = "L1",std::string name = "NONE", float min_invm = 0);
    feature_invm_CF(const feature_invm_CF&);
    virtual ~feature_invm_CF();

    virtual void Print();

  private:
    virtual bool evaluateJet_L1(struct TrigBtagEmulationJet*);    
  };


} //namespace

#endif
