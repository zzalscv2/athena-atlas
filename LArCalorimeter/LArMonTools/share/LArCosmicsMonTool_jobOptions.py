# --- for athena online running ---
if 'EventBlockSize' not in dir():
    EventBlockSize=0

###### Cosmic Tool Configuration ###############
from LArMonTools.LArMonToolsConf import LArCosmicsMonTool
theLArCosmicsMonTool = LArCosmicsMonTool(name="LArCosmicsMonTool",
                                         ProcessNEvents             = EventBlockSize,
                                         LArDigitContainerKey       = LArMonFlags.LArDigitKey(),
                                         muonADCthreshold_EM_barrel = 30,         # Sampling 2
                                         muonADCthreshold_EM_endcap = 40,         # Sampling 2
                                         muonADCthreshold_HEC       = 30,         # Sampling 1
                                         muonADCthreshold_FCAL      = 30,         # Sampling 1
                                         ProblemsToMaks             = ProblemsToMask
                         )

theLArCosmicsMonTool.LArPedestalKey='LArPedestal'
   
    
#ToolSvc += theLArCosmicsMonTool
LArMon.AthenaMonTools+=[ theLArCosmicsMonTool ]

                                    
