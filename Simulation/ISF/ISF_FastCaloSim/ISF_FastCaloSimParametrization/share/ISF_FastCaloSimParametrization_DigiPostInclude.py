streamRDO.ItemList+=["ISF_FCS_Parametrization::FCS_StepInfoCollection#MergedEventSteps","LArHitContainer#*","TileHitVector#*"];
puAlg = job.StandardPileUpToolsAlg
puAlg.PileUpTools["LArPileUpTool"].CrossTalk=False
