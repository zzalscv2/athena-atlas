/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MONTECARLOREACT_SHIFTFUNCTION_H
#define MONTECARLOREACT_SHIFTFUNCTION_H

#include "MonteCarloReactUtils/EffExceptions.h"
#include "MonteCarloReactUtils/Resolution.h"

#include <vector>
#include <string>
#include <iosfwd>

#include <TF2.h>
#include <TF1.h>
class TH1F;
class TAxis ;

namespace MonteCarloReact {

  class ShiftFunction : public Resolution {
    
  public:
    
    /* Different Mode for the treatment of underflow and overflow bins */
    enum Mode {Default, IgnoreUnderflow, IgnoreOverflow, IgnoreUnderflowOverflow} ;
    enum FindBinMode {IncludeLowBound, IncludeHighBound} ;

    ShiftFunction() : Resolution(), m_flowMode(Default) {}
    ShiftFunction( std::istream & ist, const ResInfo* request=0 );

    // initialise efficiency with Root TF1 or TF2 functions
    ShiftFunction( const TF1 * h);
    ShiftFunction( const TF2 * h);

    
    // add bin axis with non-uniform bin structure
    void defineBins( const std::vector<float>& edges) ;

    // add bin axis with uniform bins
    void defineBins( int nbins, float xlo, float xhi) ;

    // add bins from ROOT TAxis
    void addTAxis(const TAxis* axis) ;

    
    // number of bins of axis number n (starting from 0). 
    // Default value -1 corresponds to the total number of bins
    int getNbins(int n=-1) const ;

    
    void setVal( int ibin, const std::string& s) ;

    void setVal( const std::vector<int>& ibins, std::string s) ;

    void setVal( const std::vector<float>& axis, std::string s) ;

    

    // get internal bin index using  axis values.
    int getBinIndex( const std::vector<float>& value) const;

    int getBinIndex( const std::vector<double>& value) const;
    
    // return internal bin index using axis bin index. 
    // for example, for 3d index = z + nz*y + nz*ny*x 
    int getBinIndex( const std::vector<int>& value) const ;

    // get bin edges for axis with number axis_n
    float getLowBinEdge( int axis_n, int n) const ;
    float getHighBinEdge( int axis_n, int n) const ;

    // 1D only
    void getFunc(TF1& f1, float x) const;
    // 2D only
    void getFunc(TF1& f1, float x, float y) const;
    // any  number of dimensions
    void getFunc(TF1& f1,  const std::vector<float>&  value) const;
    // 1D only
    void getFunc(TF2& f2, float x) const;
    // 2D only
    void getFunc(TF2& f2, float x, float y) const;
    // any  number of dimensions
    void getFunc(TF2& f2,  const std::vector<float>&  value) const;

    
    // 1D only
    double getFuncVal(float x, float xval) const;
    // 2D only
    double getFuncVal(float x, float y, float xval) const;
    // any  number of dimensions
    double getFuncVal( const std::vector<float>&  value, float xval) const;
    // 2D function
    double getFuncVal(const std::vector<float>& value, 
                      float xval1 , float xval2) const;
    // any dimension with 2D function
    double getFuncVal(const std::vector<float>& value, 
                      const std::vector<float>& xval) const;

    double getFuncVal(const std::vector<double>& value, 
                      const std::vector<double>& xval) const;

    
    const TF1& getBinContent( int nx, int ny, int nz ) const;

    // remove all bin axis and bin contents information 
    void clear() ;
    
    // int find bin index (number) using axis value 
    int findBin( const std::vector< float >& loEdges, float x, FindBinMode mode = IncludeLowBound) const; 

    // int find bin index (number) using axis value and number 
    int findBin(int axis_n, float x, FindBinMode mode = IncludeLowBound) const {
      return findBin(m_axisEdges.at(axis_n), x, mode);} 
    
    
    /// if mode set to IGNORE_UNDERFLOW will substitute the underflow with the lowest bin value
    /// if mode set to  IGNORE_OVERFLOW will substitute the overrflow with the highest bin value 
    /// IGNORE_UNDEROVERFLOW will substitue both underflow and overflow
    void setExeptionMode(Mode mode) {m_flowMode = mode ;}

    // return historgram dimension (number of axis)
    int getDimension() const { return m_axisEdges.size();} 
    
    bool validateInput() const { return m_tf1.size() >0 ; }
    
  protected:
    bool parseInputLine( const std::string &key, const std::vector< std::string > & line);
    
  private:

    std::vector<std::vector< float > > m_axisEdges ; // The order of axes is x, y, z ...
    std::vector< TF1 > m_tf1;
    std::vector< TF2 > m_tf2;

    
    Mode m_flowMode ;

    // write efficiency information to the output stream
    void stream( std::ostream &) const;

    // initialize efficiencies
    void initRes() ;
  };
}
#endif
