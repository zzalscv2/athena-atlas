/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 InDetRawData package
 -----------------------------------------
 ***************************************************************************/

//<doc><file>	$Id: TRT_RDO_Container.h,v 1.4 2006-09-27 06:31:46 andr Exp $
//<version>	$Name: not supported by cvs2svn $

#ifndef INDETRAWDATA_TRT_RDO_CONTAINER_H
# define INDETRAWDATA_TRT_RDO_CONTAINER_H

#include "AthenaKernel/CLASS_DEF.h"
#include "AthContainers/ConstDataVector.h"
#include "InDetRawData/InDetRawDataContainer.h"
#include "InDetRawData/InDetRawDataCollection.h"
#include "InDetRawData/TRT_RDORawData.h"
#include "InDetRawData/TRT_RDO_Collection.h"
#include "AthLinks/DeclareIndexingPolicy.h"
#include "EventContainers/IIdentifiableCont.h"

typedef InDetRawDataContainer<InDetRawDataCollection<TRT_RDORawData> >
        TRT_RDO_Container;


typedef EventContainers::IdentifiableCache< InDetRawDataCollection<TRT_RDORawData> > TRT_RDO_Cache;

CLASS_DEF(TRT_RDO_Container,2542,1)
CONTAINER_IS_IDENTCONT(TRT_RDO_Container)

CLASS_DEF( TRT_RDO_Cache , 183588116, 1 )

// new persistent class
typedef  ConstDataVector<DataVector<InDetRawDataCollection<TRT_RDORawData> > > TRT_RDO_colvector;
CLASS_DEF( TRT_RDO_colvector , 140807381 , 1 )

#endif // INDETRAWDATA_TRT_RDO_CONTAINER_H
