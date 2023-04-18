from xAODTruthCnv.xAODTruthCnvConf import xAODMaker__xAODTruthCnvAlg
if not hasattr(prefiltSeq, 'xAODCnv'):
    prefiltSeq += xAODMaker__xAODTruthCnvAlg('xAODCnv',WriteTruthMetaData=False)
prefiltSeq.xAODCnv.AODContainerName = 'GEN_EVENT'

#CreateTruthJets(prefiltSeq,filtSeq,runArgs.ecmEnergy,0.6)

# Turn off ghost association algorithms
#include("GeneratorFilters/JetFilter_JZX.py")
#from JetFilter_JZX import JZSlice

#JZSlice(0,prefiltSeq,filtSeq,runArgs.ecmEnergy,0.6)

# Min and max momenta for the slices
minDict = {0:-1,1:20,2:60,3:160,4:400,5:800,6:1300,7:1800,8:2500,9:3200,10:3900,11:4600,12:5300}
maxDict = {0:20,1:60,2:160,3:400,4:800,5:1300,6:1800,7:2500,8:3200,9:3900,10:4600,11:5300,12:7000}

def CreateJets(prefiltSeq, jetR, mods=""):    
    # for compatibility with the rest of GEN config, we just re-map the
    # standard def. Best would be to change all clients in GEN to call us passing a full JetDefinition object
    if jetR < 0.65 :
      from JetRecConfig.StandardSmallRJets import  AntiKt4TruthGEN,AntiKt4TruthGENWZ,AntiKt6TruthGEN,AntiKt6TruthGENWZ
      jetdef = {
          (0.4,"") : AntiKt4TruthGEN,
          (0.4,"WZ") : AntiKt4TruthGENWZ,
          (0.6,"") : AntiKt6TruthGEN,
          (0.6,"WZ") : AntiKt6TruthGENWZ,
      }[ (jetR,mods) ]
    else :
      from JetRecConfig.StandardLargeRJets import  AntiKt10TruthGEN,AntiKt10TruthGENWZ
      jetdef = {
          (1.0,"") : AntiKt10TruthGEN,
          (1.0,"WZ") : AntiKt10TruthGENWZ,
      }[ (jetR,mods) ]

    
    # run2-config compatibility. (run3 style would use a ComponentAccumulator).
    from JetRecConfig.JetRecConfig import getJetAlgs, reOrderAlgs
    # Initialize ConfigFlags for use by CA-based code below
    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    from AthenaConfiguration.Enums import BeamType           
    ConfigFlags.Input.isMC = True
    ConfigFlags.Beam.Type = BeamType.Collisions

    # Get the algs needed by the JetDefinition and schedule them with runII style
    algs, jetdef_i = getJetAlgs(ConfigFlags, jetdef, True)
    algs, ca = reOrderAlgs( [a for a in algs if a is not None])
    # ignore dangling CA instance in legacy config
    ca.wasMerged()
    for a in algs:
        prefiltSeq += a

        
def AddJetsFilter(filtSeq,ecmEnergy, jetR, mods=""):       
       include("GeneratorFilters/QCDJetFilter.py")
#    from QCDJetFilter import AddJetFilter
       AddJetFilter(filtSeq,ecmEnergy)
       jetcollname = 'AntiKt{0}Truth{1}Jets'.format(int(jetR*10),mods)
       filtSeq.QCDTruthJetFilter.TruthJetContainer = jetcollname

def JZSlice(x,filtSeq):
    from AthenaCommon.SystemOfUnits import GeV
    filtSeq.QCDTruthJetFilter.MinPt = minDict[x]*GeV
    filtSeq.QCDTruthJetFilter.MaxPt = maxDict[x]*GeV
