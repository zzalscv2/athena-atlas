/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef __TRIG_VERTEX_FITTER__
#define __TRIG_VERTEX_FITTER__

#include "TrigInDetToolInterfaces/ITrigVertexFitter.h"
#include "AthenaBaseComps/AthAlgTool.h"

class TrigTimer;

class matrixRow
{
  double v[2];
 public:
  matrixRow()
    {
      for(int k=0;k<2;k++) v[k]=0.0;
    }
  ~matrixRow(){};
  double & operator[] (int i)
    {
      return v[i];
    }
};

class bdTrack
{
public:
  bdTrack(const TrigInDetTrack* pT, double L[6], double C[2][2], double A[3][3],
	  double eta[2], double par[3])
    {
      m_pTrack=pT;
      for(int i=0;i<6;i++) m_L[i]=L[i];
      for(int i=0;i<2;i++)
	for(int j=0;j<2;j++) m_C[i][j]=C[i][j];
      for(int i=0;i<3;i++)
	for(int j=0;j<3;j++) m_V[i][j]=A[i][j];
      for(int i=0;i<3;i++) m_par[i]=par[i];
      m_eta[0]=eta[0];m_eta[1]=eta[1];
    }
  ~bdTrack(){};
  double m_measCov(int i, int j) { return m_C[i][j];}
  double m_paramCov(int i, int j) { return m_V[i][j];}
  double m_measurements(int i) {return m_eta[i];}
  double m_params(int i) {return m_par[i];}
  double m_lambda(int i) {return m_L[i];}
  const TrigInDetTrack* m_getTrack() {return m_pTrack;}
 private:
  double m_C[2][2];
  double m_V[3][3];
  double m_L[6];
  double m_eta[2];
  double m_par[3];
  const TrigInDetTrack* m_pTrack;
};

class TrigVertexFitter: public AthAlgTool, virtual public ITrigVertexFitter
{
 public:
  TrigVertexFitter( const std::string&, const std::string&, const IInterface* );
  virtual ~TrigVertexFitter();
  virtual StatusCode initialize();
  virtual StatusCode finalize();

  virtual TrigVertex* fit(TrigVertex*);
  virtual TrigVertex* fit(TrackInVertexList*);
  virtual TrigVertex* fit(TrackInVertexList*,std::vector<TrigVtx::TrigParticleName>);

private:
  bdTrack* m_biDiagonalization(const TrigInDetTrack*);
  void m_calculateInvariantMass(TrigVertex*, std::vector<TrigVtx::TrigParticleName>&);
  void m_showCov(int);
  void m_showVec(int);
  bool m_checkTracks(TrackInVertexList*);
  bool m_initFit(TrackInVertexList*);
  int m_numIter;
  double* m_Rk;
  double* m_Gk;
  double* m_dChi2;
  std::vector<bdTrack*> m_vpTracks;

#define TRIGVERTEXFIT_NTIMERS 5
  TrigTimer* m_timer[TRIGVERTEXFIT_NTIMERS];
  bool m_timers;

};

#endif
