# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#make the InDetTrigConfigFlags available as a global object to different modules
if 'InDetTrigConfigFlags' not in globals():
  InDetTrigConfigFlags = None
