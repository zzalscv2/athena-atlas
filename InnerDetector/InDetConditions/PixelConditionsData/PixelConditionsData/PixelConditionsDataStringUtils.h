#ifndef PixelConditionsData_StringUtils
#define PixelConditionsData_StringUtils

#include <vector>
#include <string>
#include <cctype> //for std::tolower


namespace PixelConditionsData{

  //for PixelConfigCondAlg
  template <class T = std::string>
  T 
  fromString(const std::string & s){
    return s;
  }
  
  template<>
  inline float 
  fromString(const std::string &s){
    return std::stof(s);
  }
  
  template <>
  inline double
  fromString(const std::string & s){
    return std::stod(s);
  }
  
  template <>
  inline int
  fromString(const std::string & s){
    return std::stoi(s);
  }
  
  template<>
  inline bool 
  fromString(const std::string & s){
    std::string v=s;
    for (char & c: v) c = std::tolower(c);
    if (v.find("false")!=std::string::npos) return false;
    if (v.find("true")!=std::string::npos) return true;
    throw("bad conversion");
  }
  
  std::vector<std::string> 
  getParameterString(const std::string& varName, const std::vector<std::string>& buffer);
  
  template <class T>
  std::vector<T>
  getParameter(const std::string& varName, const std::vector<std::string>& buffer){
    const std::vector<std::string> & varString = getParameterString(varName, buffer);
    std::vector<T> result;
    for (const auto & var : varString) {
      result.emplace_back(fromString<T>(var));
    }
    return result;
  }
}
#endif
