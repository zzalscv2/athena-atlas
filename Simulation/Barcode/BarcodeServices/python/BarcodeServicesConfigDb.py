# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

"""
Configuration database for BarcodeServices
Elmar Ritsch, 23/10/2014
"""

from AthenaCommon.CfgGetter import addService

# Common tools, services and algorithms used by jobs:
addService("BarcodeServices.BarcodeServicesConfigLegacy.getMC12BarcodeSvc"          ,  "Barcode_MC12BarcodeSvc"          )
addService("BarcodeServices.BarcodeServicesConfigLegacy.getMC12LLPBarcodeSvc"       ,  "Barcode_MC12LLPBarcodeSvc"       )
addService("BarcodeServices.BarcodeServicesConfigLegacy.getMC12PlusBarcodeSvc"      ,  "Barcode_MC12PlusBarcodeSvc"      )
addService("BarcodeServices.BarcodeServicesConfigLegacy.getMC15aBarcodeSvc"         ,  "Barcode_MC15aBarcodeSvc"         )
addService("BarcodeServices.BarcodeServicesConfigLegacy.getMC15aPlusBarcodeSvc"     ,  "Barcode_MC15aPlusBarcodeSvc"     )
addService("BarcodeServices.BarcodeServicesConfigLegacy.getMC15aPlusLLPBarcodeSvc"  ,  "Barcode_MC15aPlusLLPBarcodeSvc"  )
addService("BarcodeServices.BarcodeServicesConfigLegacy.getMC15BarcodeSvc"          ,  "Barcode_MC15BarcodeSvc"          )
addService("BarcodeServices.BarcodeServicesConfigLegacy.getValidationBarcodeSvc"    ,  "Barcode_ValidationBarcodeSvc"    )
addService("BarcodeServices.BarcodeServicesConfigLegacy.getBarcodeSvc"    ,  "BarcodeSvc"    )
