# This sets up the trigger config for a BStoESD job
# to use the HLT output XML file generated by a previous BStoBS step
# and a fixed LVL1 file from the release which should be what was used to rerun the trigger over BS
# It is intended for private trigger reprocessing only.
# Contact:  Clemencia Mora or  trigger configuration experts

##preInclude for all steps but enable only for RAWtoESD
##don't set this in ESDtoAOD, it works with HLTonline since DS folders are stored in ESD metadata

from RecExConfig.RecFlags import rec
if rec.readRDO and rec.doESD:
    from TriggerJobOpts.TriggerFlags import TriggerFlags as tf
    tf.inputHLTconfigFile.set_Value_and_Lock("outputHLTconfig.xml")
    tf.inputLVL1configFile.set_Value_and_Lock("TriggerMenuXML/LVL1config_InitialBeam_v3.xml")
    tf.configForStartup.set_Value_and_Lock("HLToffline")
    tf.configurationSourceList.set_Value_and_Lock(['xml'])
    tf.readHLTconfigFromXML.set_Value_and_Lock(True)
    tf.readLVL1configFromXML.set_Value_and_Lock(True)
