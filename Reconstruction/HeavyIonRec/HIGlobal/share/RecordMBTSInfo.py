if "StreamESD" in dir():
    StreamESD.ItemList += [ "xAOD::TrigT2MbtsBitsContainer#MBTSBits", "xAOD::TrigT2MbtsBitsAuxContainer#MBTSBitsAux." ]


if "StreamAOD" in dir():
    StreamAOD.ItemList += [ "xAOD::TrigT2MbtsBitsContainer#MBTSBits", "xAOD::TrigT2MbtsBitsAuxContainer#MBTSBitsAux." ]

if "StreamESD" in dir() or  ("StreamAOD" in dir() and not rec.readESD):
    from HIGlobal.HIGlobalConf import MBTSInfoCopier
    copier = MBTSInfoCopier()
    topSequence += copier