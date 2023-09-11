include("Sherpa_i/Base_Fragment.py")
include("Sherpa_i/NNPDF30NNLO.py")

evgenConfig.description = "Sherpa 2.2.x example JO, Z+0,1-jet production."
evgenConfig.keywords = [ "2lepton" ]
evgenConfig.contact  = [ "atlas-generators-sherpa@cern.ch", "frank.siegert@cern.ch"]
evgenConfig.nEventsPerJob = 10000

genSeq.Sherpa_i.RunCard="""
(run){
  ME_SIGNAL_GENERATOR=Amegic
}(run)

(processes){
  Process 93 93 -> 11 -11 93{1}
  Order (*,2)
  CKKW sqr(20/E_CMS)
  End process;
}(processes)

(selector){
  Mass 11 -11 40 E_CMS
}(selector)
"""

genSeq.Sherpa_i.Parameters += []
genSeq.Sherpa_i.OpenLoopsLibs = []
genSeq.Sherpa_i.ExtraFiles = []
genSeq.Sherpa_i.NCores = 1
