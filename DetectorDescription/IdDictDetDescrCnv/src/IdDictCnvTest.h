/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 IdDict converter package
 -----------------------------------------
 Copyright (C) 2003 by ATLAS Collaboration
 ***************************************************************************/

//<doc><file>	$Id: IdDictCnvTest.h,v 1.2 2004-10-07 17:19:39 schaffer Exp $
//<version>	$Name: not supported by cvs2svn $

#ifndef SRC_IDDICTCNVTEST_H
# define SRC_IDDICTCNVTEST_H

//<<<<<< INCLUDES                                                       >>>>>>

#include "GaudiKernel/Algorithm.h"

//<<<<<< PUBLIC DEFINES                                                 >>>>>>
//<<<<<< PUBLIC CONSTANTS                                               >>>>>>
//<<<<<< PUBLIC TYPES                                                   >>>>>>
//<<<<<< PUBLIC VARIABLES                                               >>>>>>
//<<<<<< PUBLIC FUNCTIONS                                               >>>>>>
//<<<<<< CLASS DECLARATIONS                                             >>>>>>

/********************************************************************

Algorithm for testing the loading of the Identifier dictionaries

********************************************************************/

class IdDictCnvTest : public Algorithm
{

public:

    IdDictCnvTest(const std::string& name, ISvcLocator* pSvcLocator);
    ~IdDictCnvTest();

    StatusCode initialize();
    StatusCode execute();
    StatusCode finalize();

private:

    void        tab(size_t level) const;
};

//<<<<<< INLINE PUBLIC FUNCTIONS                                        >>>>>>
//<<<<<< INLINE MEMBER FUNCTIONS                                        >>>>>>

#endif // SRC_IDDICTCNVTEST_H
