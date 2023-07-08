#!/bin/sh
#
# art-description: run the RAWtoESD transform of plain q443 with different number of threads and compare the outputs
#
# art-type: grid
# art-include: main/Athena
# art-include: 22.0/Athena
# art-athena-mt: 8
# art-output: OUT_ESD.root
# art-output: OUT_ESD_1thread.root
# art-output: OUT_ESD_5thread.root
# art-output: OUT_ESD_8thread.root
# art-output: diff_1_vs_serial.txt
# art-output: diff_5_vs_1.txt
# art-output: diff_8_vs_1.txt
# art-output: log.RAWtoESD_serial
# art-output: log.RAWtoESD_1thread
# art-output: log.RAWtoESD_5thread
# art-output: log.RAWtoESD_8thread
# art-output: log.HITtoRDO_serial
# art-output: log.HITtoRDO_1thread
# art-output: log.HITtoRDO_5thread
# art-output: log.HITtoRDO_8thread
# art-output: log.RDOtoRDOTrigger_serial
# art-output: log.RDOtoRDOTrigger_1thread
# art-output: log.RDOtoRDOTrigger_5thread
# art-output: log.RDOtoRDOTrigger_8thread

#####################################################################
Reco_tf.py --AMI q443 \
           --imf False \
           --outputESDFile OUT_ESD.root
exit_code=$?
echo  "art-result: ${exit_code} Reco_tf.py"
if [ ${exit_code} -ne 0 ]
then
    exit ${exit_code}
fi
mv log.HITtoRDO log.HITtoRDO_serial
mv log.RDOtoRDOTrigger log.RDOtoRDOTrigger_serial
mv log.RAWtoESD log.RAWtoESD_serial
#####################################################################

#####################################################################
# now run reconstruction with AthenaMT with 1 thread
Reco_tf.py --AMI q443 \
           --imf False \
           --athenaopts="HITStoRDO:--threads=1;RAWtoALL:--threads=1" \
           --outputESDFile OUT_ESD_1thread.root
exit_code=$?
echo  "art-result: ${exit_code} Reco_tf_1thread.py"
if [ ${exit_code} -ne 0 ]
then
    exit ${exit_code}
fi
mv log.HITtoRDO log.HITtoRDO_1thread
mv log.RDOtoRDOTrigger log.RDOtoRDOTrigger_1thread
mv log.RAWtoESD log.RAWtoESD_1thread
#####################################################################

#####################################################################
# now run reconstruction with AthenaMT with 5 threads
Reco_tf.py --AMI q443 \
           --imf False \
           --athenaopts="HITStoRDO:--threads=5;RAWtoALL:--threads=5" \
           --outputESDFile OUT_ESD_5thread.root
exit_code=$?
echo  "art-result: ${exit_code} Reco_tf_5thread.py"
if [ ${exit_code} -ne 0 ]
then
    exit ${exit_code}
fi
mv log.HITtoRDO log.HITtoRDO_5thread
mv log.RDOtoRDOTrigger log.RDOtoRDOTrigger_5thread
mv log.RAWtoESD log.RAWtoESD_5thread
#####################################################################

#####################################################################
# now run reconstruction with AthenaMT with 8 threads
Reco_tf.py --AMI q443 \
           --imf False \
           --athenaopts="HITStoRDO:--threads=8;RAWtoALL:--threads=8" \
           --outputESDFile OUT_ESD_8thread.root
exit_code=$?
echo  "art-result: ${exit_code} Reco_tf_8thread.py"
if [ ${exit_code} -ne 0 ]
then
    exit ${exit_code}
fi
mv log.HITtoRDO log.HITtoRDO_8thread
mv log.RDOtoRDOTrigger log.RDOtoRDOTrigger_8thread
mv log.RAWtoESD log.RAWtoESD_8thread
#####################################################################

#####################################################################
# now run diff-root to compare the ESDs made with serial and 1thread
acmd.py diff-root  --nan-equal \
                   --ignore-leaves InDet::PixelClusterContainer_p3_PixelClusters \
                                  HLT::HLTResult_p1_HLTResult_HLT.m_navigationResult  \
                                  xAOD::BTaggingAuxContainer_v1_BTagging_AntiKt4EMTopoAuxDyn \
                                  xAOD::TrigDecisionAuxInfo_v1_xTrigDecisionAux \
                                  xAOD::TrigPassBitsAuxContainer_v1_HLT_xAOD__TrigPassBitsContainer_passbitsAux \
                                  xAOD::TrigNavigationAuxInfo_v1_TrigNavigationAux \
                                  InDet::SCT_ClusterContainer_p3_SCT_Clusters \
                                  xAOD::BTaggingAuxContainer_v1_HLT_BTaggingAuxDyn \
                                  xAOD::JetAuxContainer_v1_AntiKt4TruthJetsAuxDyn \
                                  xAOD::BTaggingAuxContainer_v1_HLT_BTaggingAuxDyn \
                                  xAOD::JetAuxContainer_v1_AntiKt4EMTopoJetsAuxDyn \
                                  xAOD::JetAuxContainer_v1_AntiKt4EMPFlowJetsAuxDyn \
                                  RecoTimingObj_p1_HITStoRDO_timings \
                                  RecoTimingObj_p1_RAWtoESD_mems \
                                  RecoTimingObj_p1_RAWtoESD_timings \
                                  index_ref \
                  --order-trees OUT_ESD_1thread.root OUT_ESD.root &> diff_1_vs_serial.txt
exit_code=$?
echo  "art-result: ${exit_code} diff-root"
if [ ${exit_code} -ne 0 ]
then
    exit ${exit_code}
fi
#####################################################################
# now run diff-root to compare the ESDs made with 5threads and 1thread
acmd.py diff-root  --nan-equal \
                   --ignore-leaves InDet::PixelClusterContainer_p3_PixelClusters \
                                  HLT::HLTResult_p1_HLTResult_HLT.m_navigationResult  \
                                  xAOD::BTaggingAuxContainer_v1_BTagging_AntiKt4EMTopoAuxDyn \
                                  xAOD::TrigDecisionAuxInfo_v1_xTrigDecisionAux \
                                  xAOD::TrigPassBitsAuxContainer_v1_HLT_xAOD__TrigPassBitsContainer_passbitsAux \
                                  xAOD::TrigNavigationAuxInfo_v1_TrigNavigationAux \
                                  InDet::SCT_ClusterContainer_p3_SCT_Clusters \
                                  xAOD::BTaggingAuxContainer_v1_HLT_BTaggingAuxDyn \
                                  xAOD::JetAuxContainer_v1_AntiKt4TruthJetsAuxDyn \
                                  xAOD::BTaggingAuxContainer_v1_HLT_BTaggingAuxDyn \
                                  xAOD::JetAuxContainer_v1_AntiKt4EMTopoJetsAuxDyn \
                                  xAOD::JetAuxContainer_v1_AntiKt4EMPFlowJetsAuxDyn \
                                  RecoTimingObj_p1_HITStoRDO_timings \
                                  RecoTimingObj_p1_RAWtoESD_mems \
                                  RecoTimingObj_p1_RAWtoESD_timings \
                                  index_ref \
                    --order-trees OUT_ESD_5thread.root OUT_ESD_1thread.root &> diff_5_vs_1.txt
exit_code=$?
echo  "art-result: ${exit_code} diff-root_5thread"
if [ ${exit_code} -ne 0 ]
then
    exit ${exit_code}
fi
#####################################################################
# now run diff-root to compare the ESDs made with 8threads and 1thread
acmd.py diff-root  --nan-equal \
                   --ignore-leaves InDet::PixelClusterContainer_p3_PixelClusters \
                                  HLT::HLTResult_p1_HLTResult_HLT.m_navigationResult  \
                                  xAOD::BTaggingAuxContainer_v1_BTagging_AntiKt4EMTopoAuxDyn \
                                  xAOD::TrigDecisionAuxInfo_v1_xTrigDecisionAux \
                                  xAOD::TrigPassBitsAuxContainer_v1_HLT_xAOD__TrigPassBitsContainer_passbitsAux \
                                  xAOD::TrigNavigationAuxInfo_v1_TrigNavigationAux \
                                  InDet::SCT_ClusterContainer_p3_SCT_Clusters \
                                  xAOD::BTaggingAuxContainer_v1_HLT_BTaggingAuxDyn \
                                  xAOD::JetAuxContainer_v1_AntiKt4TruthJetsAuxDyn \
                                  xAOD::BTaggingAuxContainer_v1_HLT_BTaggingAuxDyn \
                                  xAOD::JetAuxContainer_v1_AntiKt4EMTopoJetsAuxDyn \
                                  xAOD::JetAuxContainer_v1_AntiKt4EMPFlowJetsAuxDyn \
                                  RecoTimingObj_p1_HITStoRDO_timings \
                                  RecoTimingObj_p1_RAWtoESD_mems \
                                  RecoTimingObj_p1_RAWtoESD_timings \
                                  index_ref \
                   --order-trees OUT_ESD_8thread.root OUT_ESD_1thread.root &> diff_8_vs_1.txt
exit_code=$?
echo  "art-result: ${exit_code} diff-root_8thread"
if [ ${exit_code} -ne 0 ]
then
    exit ${exit_code}
fi
#####################################################################

echo "art-result: $?"

