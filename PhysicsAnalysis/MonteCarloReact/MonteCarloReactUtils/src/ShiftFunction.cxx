/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MonteCarloReactUtils/ShiftFunction.h"
#include "MonteCarloReactUtils/EffExceptions.h"
#include "MonteCarloReactUtils/EffVal.h"
#include "MonteCarloReactUtils/BinnedEfficiency.h"

#include <TH1.h>
#include <TH2.h>
#include <TAxis.h>
#include <iostream>
#include <sstream>

using namespace MonteCarloReact;
using namespace std;

ShiftFunction::ShiftFunction( istream & ist,
                              const ResInfo* request) : Resolution(), m_flowMode(Default)
{
  makeResolution( ist, request );
}

ShiftFunction::ShiftFunction( const TF1 * ) : Resolution(), m_flowMode(Default)
{
  //needs to be implememented
  cerr << "MonteCarloReactUtils::ShiftFunction(const TF1) Not yet implemented" << endl;
}

ShiftFunction::ShiftFunction( const TF2 * ) : Resolution(), m_flowMode(Default)
{
  //needs to be implememented
  cerr << "MonteCarloReactUtils::ShiftFunction(const TF2) Not yet implemented" << endl;
}



void ShiftFunction::defineBins(const vector< float > & edges)
{
  m_axisEdges.push_back(vector<float>(edges)) ;
  initRes() ;
}

void ShiftFunction::defineBins( int nbins, float xlo, float xhi)
{
  if( nbins <=0 ) 
    throw runtime_error("ShiftFunction::DefineBins ERROR: number of bins should be greater than 0") ;

  // make n bins of equal size between lo and hi
  float xs = (xhi-xlo)/nbins;

  m_axisEdges.push_back(vector<float>()) ;
  for( int i = 0; i <= nbins; ++i) {
    m_axisEdges.back().push_back(xlo + ( i * xs )) ;
  }
  initRes() ;
}

void ShiftFunction::addTAxis(const TAxis* axis) {
  m_axisEdges.push_back(vector<float>()) ;
  for( int i = 0; i < axis->GetNbins(); ++i) 
    m_axisEdges.back().push_back(axis->GetBinLowEdge( i+1)) ;  
  m_axisEdges.back().push_back(axis->GetBinUpEdge( axis->GetNbins())) ;  
  initRes() ;
}

int ShiftFunction::getNbins(int n) const {
  if (n >= (int) m_axisEdges.size()) {
    ostringstream st  ;
    st << "ShiftFunction::GetNbins ERROR: requested axis with number " << n
      << " is not existing. Only " << m_axisEdges.size() << " axes exist" ;
    throw runtime_error(st.str()) ;	
  }

  if (n>=0) return (m_axisEdges.begin()+n)->size() - 1;

  int nbins = 1 ; 
  for (vector<vector<float> >::const_iterator it = m_axisEdges.begin(); 
       it != m_axisEdges.end(); ++it) nbins *= it->size() - 1 ;
  return nbins ;  
}


/* *********************************************************************************** */
void ShiftFunction::setVal( int ibin,const std::string& s)
{
  if( ibin < 0) throw BinnedEffUnderflow() ;
  if (m_spec.getResNParas() == 1) {
    if(ibin >= (int)m_tf1.size() )  throw BinnedEffOverflow() ;
    else {
      std::ostringstream streamOut;
      streamOut << "FuncBin" << ibin;
      std::string funcname =  streamOut.str();
      //float lowerbound = _axesEdges
  
      TF1 function(funcname.c_str(), s.c_str(),-100,100);
      TF1 & tmptf1 = m_tf1.at( ibin );
      tmptf1 = function;
    }
  }
  else if (m_spec.getResNParas() == 2) {
    if(ibin >= (int)m_tf2.size() )  throw BinnedEffOverflow() ;
    else {
      std::ostringstream streamOut;
      streamOut << "FuncBin" << ibin;
      std::string funcname =  streamOut.str();
      //float lowerbound = _axesEdges
      
      TF2 function(funcname.c_str(), s.c_str(),-100,100);
      TF2 & tmptf2 = m_tf2.at( ibin );
      tmptf2 = function;
    }
  }
}

void ShiftFunction::setVal( const vector<int>& ibins, std::string s)
{
  setVal( getBinIndex(ibins), s);
}

void ShiftFunction::setVal(const vector<float>& axis, std::string s)
{
  setVal( getBinIndex(axis), s);
}

int ShiftFunction::getBinIndex( const vector<float>& value) const {
  vector<double> dvalue;
  dvalue.clear();
  for(size_t i = 0; i < value.size(); ++i) {
    dvalue.push_back((double)value[i]);
  } 
  return getBinIndex( dvalue);
}


int ShiftFunction::getBinIndex( const vector<double>& value) const {
  if (value.size() != m_axisEdges.size()) {
    ostringstream st  ;
    st << "ShiftFunction::BinIndex ERROR: size of the value " <<  value.size()
       << " is not equal to number of Axis " << m_axisEdges.size() ;
      throw runtime_error(st.str()) ;
  }
  vector<int> axis ;
  for (int i = 0; i < int(value.size()) ; ++i) {
    int bin = 0 ;
    try { 
      bin =  findBin( i, *(value.begin()+i)) ;
    }
    catch (BinnedEffUnderflow& e) {
      cerr << "ShiftFunction::BinIndex ERROR: value " << *(value.begin()+i) 
	   << " smaller than than the first bin edge " <<  (m_axisEdges.begin()+i)->front() 
	   << " of axis " << i << endl;
      throw BinnedEffUnderflow();
    }
   catch (BinnedEffOverflow& e) {
      cerr << "ShiftFunction::BinIndex ERROR: value " << *(value.begin()+i) 
	   << " larger than than the latest bin edge " <<  (m_axisEdges.begin()+i)->back() 
	   << " of axis " << i << endl;      
      throw BinnedEffOverflow();
    }
    axis.push_back(bin) ;
  }
  
  return getBinIndex(axis);
}

int ShiftFunction::getBinIndex( const vector<int>& value) const {
  if (value.size() != m_axisEdges.size())  {
    ostringstream st  ;
    st << "ShiftFunction::BinIndex ERROR: size of the value " <<  value.size()
      << " is not equal to number of Axis " << m_axisEdges.size() ;
      throw runtime_error(st.str()) ;
  }

  int index = 0 ;
  int index_size = 1 ;
  for (int i = value.size()-1; i >= 0 ; i--) {
    index += index_size*(*(value.begin()+i)) ;
    index_size *=  (m_axisEdges.begin()+i)->size() -1 ;
  }
  return index ;
}

float ShiftFunction::getLowBinEdge(int axis_n, int n) const {
  if (axis_n >= (int)m_axisEdges.size()) {
    ostringstream st  ;
    st << "ShiftFunction::LowBinEdge ERROR: request axis with number " << axis_n
       << " is not existing. Only " << m_axisEdges.size() << " axis exist" ;
    throw runtime_error(st.str()) ;
  }

  if( n < 0 ) {
    ostringstream st ;
    st << "for " << axis_n << " axis" ;
    throw BinnedEffUnderflow(st.str());
  } else if( n >= getNbins(axis_n)) {
    ostringstream st ;
    st << "for " << axis_n << " axis" ;
    throw BinnedEffOverflow(st.str());
  }
return (m_axisEdges.begin()+axis_n)->at(n) ;
}

float ShiftFunction::getHighBinEdge(int axis_n, int n) const {
  if (axis_n >= (int) m_axisEdges.size()) {
    ostringstream st  ;
    st << "ShiftFunction::HighBinEdge ERROR: request axis with number " << axis_n
      << " is not existing. Only " << m_axisEdges.size() << " axis exist" ;
      throw runtime_error( st.str()) ;	
  }

  if( n < 0 ) {
    ostringstream st ;
    st << "for " << axis_n << " axis" ;
    throw BinnedEffUnderflow(st.str());
  } else if( n >= getNbins(axis_n)) {
    ostringstream st ;
    st << "for " << axis_n << " axis" ;
    throw BinnedEffOverflow(st.str());
  }
  return (m_axisEdges.begin()+axis_n)->at(n+1) ;
}


void ShiftFunction::getFunc(TF1& f1, float x) const
{
  vector<float> bins ;
  bins.push_back(x) ;
  getFunc(f1, bins) ;
}

void ShiftFunction::getFunc(TF1& f1, float x, float y) const
{
  vector<float> bins ;
  bins.push_back(x) ;
  bins.push_back(y) ;
  getFunc(f1, bins) ;
}

void ShiftFunction::getFunc(TF1& f1, const std::vector<float>& value) const
{
  int dim = m_spec.getResNVars();
  if( dim != getDimension()) throw BinnedBadDimension();
  int index = getBinIndex(value);
  f1 = m_tf1.at( index );
}

void ShiftFunction::getFunc(TF2& f2, float x) const
{
  vector<float> bins ;
  bins.push_back(x) ;
  getFunc(f2, bins) ;
}

void ShiftFunction::getFunc(TF2& f2, float x, float y) const
{
  vector<float> bins ;
  bins.push_back(x) ;
  bins.push_back(y) ;
  getFunc(f2, bins) ;
}

void ShiftFunction::getFunc(TF2& f2, const std::vector<float>& value) const
{
  int dim = m_spec.getResNVars();
  if( dim != getDimension()) throw BinnedBadDimension();
  int index = getBinIndex(value);
  f2 = m_tf2.at( index );
}


double ShiftFunction::getFuncVal(float x, float xval) const
{
  vector<float> bins ;
  bins.push_back(x) ;
  return getFuncVal(bins, xval) ;
}

double ShiftFunction::getFuncVal(float x, float y, float xval) const
{
  vector<float> bins ;
  bins.push_back(x) ;
  bins.push_back(y) ;
  return getFuncVal(bins, xval) ;
}

double ShiftFunction::getFuncVal(const std::vector<float>& value, float xval) const
{
  vector<float> paras ;
  paras.push_back(xval) ;
  return getFuncVal(value, paras) ;
}

double ShiftFunction::getFuncVal(const std::vector<float>& value, 
                                 float xval1 , float xval2) const
{
  vector<float> paras ;
  paras.push_back(xval1) ;
  paras.push_back(xval2) ;
  return getFuncVal(value, paras) ;
}


double ShiftFunction::getFuncVal(const std::vector<double>& value, 
                                 const std::vector<double>& xval) const
{
  int dim = m_spec.getResNVars();
  int paras = m_spec.getResNParas();
  if( dim != getDimension()) throw BinnedBadDimension();
  if( xval.size() > 2) throw BinnedBadDimension();
  int index = getBinIndex(value);
  if(paras == 1) return m_tf1.at( index ).Eval(xval[0]);
  else if(paras == 2) return m_tf2.at( index ).Eval(xval[0], xval[1]);
  return -999;
}

double ShiftFunction::getFuncVal(const std::vector<float>& value, 
                                 const std::vector<float>& xval) const
{
  vector<double> dvalue;
  dvalue.clear();
  for(size_t i = 0; i < value.size(); ++i) {
    dvalue.push_back((double)value[i]);
  } 
  vector<double> dxval;
  dxval.clear();
  for(size_t i = 0; i < xval.size(); ++i) {
    dxval.push_back((double)xval[i]);
  } 
  return getFuncVal( dvalue, dxval);
}

const TF1& ShiftFunction::getBinContent( int nx, int ny, int nz ) const {
  int dim = m_spec.getResNVars();

  if( dim == 1 && (ny > 0 || nz > 0)) 
    throw BinnedBadDimension();
  else if( dim == 2 && nz > 0 ) 
    throw BinnedBadDimension();
  else if( dim >3 ) 
    throw BinnedBadDimension(); 

  if( nx < 0 ) throw BinnedEffUnderflow(" for x axis");
  else if( nx >= getNbins(0)) throw BinnedEffOverflow(" for x axis");

  vector<int> bins ;
  bins.push_back(nx) ;
  if (dim==1) return m_tf1.at(getBinIndex(bins));

  if( ny < 0 ) throw BinnedEffUnderflow(" for y axis");
  else if( ny >= getNbins(1)) throw BinnedEffOverflow(" for y axis");

  bins.push_back(ny) ;
  if (dim==2) return m_tf1.at(getBinIndex(bins));

  if( nz < 0 ) throw BinnedEffUnderflow(" for z axis");
  else if( nz >= getNbins(2)) throw BinnedEffOverflow(" for z axis");

  bins.push_back(nz) ;
  return m_tf1.at(getBinIndex(bins));
}


void ShiftFunction::clear() {
  m_tf1.clear() ;
  m_tf2.clear() ;
  m_axisEdges.clear() ;
  Resolution::clear() ;
}


int ShiftFunction::findBin( const vector< float >& loEdges, float x, FindBinMode mode) const
{
  if (loEdges.size() <= 1)
    throw runtime_error("ShiftFunction::FindBin ERROR: number of edges should be greater than 1") ;

  if( x < loEdges.front()) {
    if (m_flowMode != IgnoreUnderflowOverflow && 
	m_flowMode != IgnoreUnderflow )
      throw BinnedEffUnderflow();
    else return 0 ;
  }

  if (mode == IncludeLowBound && x == loEdges.front()) return 0 ;

  vector< float >::const_iterator it = lower_bound(loEdges.begin(), loEdges.end(), x)  ;

  if ( it == loEdges.end() ) {
    if (m_flowMode != IgnoreUnderflowOverflow && 
	m_flowMode != IgnoreOverflow ) 
      throw BinnedEffOverflow();
    else return loEdges.size() - 1 ;
  }

  if (mode == IncludeLowBound && x == *it) return  it - loEdges.begin() ;
  if (mode == IncludeHighBound && x == *it) return  it - loEdges.begin() - 1 ;
  
  return  it == loEdges.begin() ? 0 : it - loEdges.begin()  -1 ;
}



bool ShiftFunction::parseInputLine( const string & key, const vector< string> & line)
{
  if( key.find("BinEdges") != string::npos ) {
    int npar = line.size();
    m_axisEdges.push_back(vector<float>()) ;
    for( int i =0; i < npar; ++i) { 
      m_axisEdges.back().push_back(atof( (line[i]).c_str())) ;
    }
    initRes() ;
    return true ;
  }
  
  if( key =="BinVal" ) {
    int npar = line.size();
    
    int dim = getDimension() ;
    
    if( dim!= npar-1 ) 
      throw runtime_error("ShiftFunction::ParseInputLine ERROR: number of fields in the line does not corresponds to the efficiency dimension") ;
    if( m_tf1.size() == 0) 
      throw runtime_error((string) "ShiftFunction::ParseInputLine ERROR: number of efficiency bin is 0. "+
			  "Check mcr file structure and the bin  edges information.") ;    
    vector<int> bins ;
    for (int i=0; i< dim; ++i) {
      bins.push_back(atoi( (line[i]).c_str()));
    }
    
    string function =  line[dim];
    
    setVal( bins, function);
        
    return true ;
  }
  return false;
}

void ShiftFunction::stream( ostream & os) const
{
  // first do bin edges
  for (vector<vector<float> >::const_iterator it =  m_axisEdges.begin();
       it != m_axisEdges.end() ; ++it) {
    os << "BinEdges" << it - m_axisEdges.begin() +1 << " : " ;
    for (vector<float>::const_iterator jt = it->begin(); jt != it->end() ; ++jt) 
      os << *jt << " " ;
    os << endl;
  }
  
  // now do data itself
  for(vector< TF1 >::const_iterator it = m_tf1.begin(); 
      it != m_tf1.end() ; ++it) {
    // save bin number
    os << "BinVal : " ;
    int index = it - m_tf1.begin() ;
    for (vector<vector<float> >::const_iterator jt =  m_axisEdges.begin();
	 jt != m_axisEdges.end() ; ++jt) {          
      int n = 1 ;
      for (vector<vector<float> >::const_iterator nt = jt+1 ; nt != m_axisEdges.end() ; ++nt) 
	n *= nt->size() - 1 ; 
      int x =  index/n  ;
      index = index%n ;
      os << x << " " ;
  }
    // save efficiency value and errors
    os << "    " << it->GetName() << " " << it->GetExpFormula() << " " ;
    os << endl;
  }
}






void ShiftFunction::initRes() {
  m_tf1.resize(getNbins() ) ;  
}

