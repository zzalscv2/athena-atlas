/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHEXHELLOWORLD_HELLOTOOL_H
#define ATHEXHELLOWORLD_HELLOTOOL_H

#include <AthExHelloWorld/IHelloTool.h>
#include <AthenaBaseComps/AthAlgTool.h>

#include <string>

class HelloTool : public extends<AthAlgTool, IHelloTool> {
 public:
  HelloTool(const std::string &type, const std::string &name, const IInterface *parent);

  // the magic method this tool provides
  virtual StatusCode saySomething() override;

 private:
  Gaudi::Property<std::string> m_myMessage{this, "MyMessage",
                                           "Default message set in HelloTool.h",
                                           "something to say"};
};

#endif
