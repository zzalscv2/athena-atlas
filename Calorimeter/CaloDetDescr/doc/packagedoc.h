/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/**


@page CaloDetDescr_page 

The main purpose of the CaloDetDescr package is to provide an unified 
interface to readout geometry description of the ATLAS Calorimeters -
LAr and Tile - for Reconstruction clients. 

The key classes of this package are:

1. CaloDetDescrManager object holds the information about calorimeter
cells and regions, represented by CaloDetDescrElement and CaloDetDescriptor
objects respectively. The manager is available for Reconstruction clients
in the Transient Detector Store.

2. CaloDetDescrElement contains geometrical information of one calorimeter cell.

3. CaloDetDescriptor objects represent regions in the calorimeter.

4. CaloDepthTool provides simplified and end-user defined quantities

--------------------------------
  REQUIREMENTS 
--------------------------------


*/
