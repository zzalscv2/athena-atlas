/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCMsg_h
#define ZDCMsg_h

#include <functional>
#include <memory>

namespace ZDCMsg {
    typedef std::function<bool(int, std::string)> MessageFunction;

    typedef std::shared_ptr<MessageFunction> MessageFunctionPtr;

    enum MSGLevels {

        Verbose = 1,
        Debug,
        Info,
        Warn,
        Error,
        Fatal
    };
}

#endif
