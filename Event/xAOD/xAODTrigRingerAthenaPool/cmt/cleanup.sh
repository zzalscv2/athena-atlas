# echo "cleanup xAODTrigRingerAthenaPool xAODTrigRingerAthenaPool-00-00-01 in /tmp/jodafons/testRinger/test2/Event/xAOD"

if test "${CMTROOT}" = ""; then
  CMTROOT=/cvmfs/atlas.cern.ch/repo/sw/software/x86_64-slc6-gcc47-opt/19.1.3/CMT/v1r25p20140131; export CMTROOT
fi
. ${CMTROOT}/mgr/setup.sh
cmtxAODTrigRingerAthenaPooltempfile=`${CMTROOT}/${CMTBIN}/cmt.exe -quiet build temporary_name`
if test ! $? = 0 ; then cmtxAODTrigRingerAthenaPooltempfile=/tmp/cmt.$$; fi
${CMTROOT}/${CMTBIN}/cmt.exe cleanup -sh -pack=xAODTrigRingerAthenaPool -version=xAODTrigRingerAthenaPool-00-00-01 -path=/tmp/jodafons/testRinger/test2/Event/xAOD  $* >${cmtxAODTrigRingerAthenaPooltempfile}
if test $? != 0 ; then
  echo >&2 "${CMTROOT}/${CMTBIN}/cmt.exe cleanup -sh -pack=xAODTrigRingerAthenaPool -version=xAODTrigRingerAthenaPool-00-00-01 -path=/tmp/jodafons/testRinger/test2/Event/xAOD  $* >${cmtxAODTrigRingerAthenaPooltempfile}"
  cmtcleanupstatus=2
  /bin/rm -f ${cmtxAODTrigRingerAthenaPooltempfile}
  unset cmtxAODTrigRingerAthenaPooltempfile
  return $cmtcleanupstatus
fi
cmtcleanupstatus=0
. ${cmtxAODTrigRingerAthenaPooltempfile}
if test $? != 0 ; then
  cmtcleanupstatus=2
fi
/bin/rm -f ${cmtxAODTrigRingerAthenaPooltempfile}
unset cmtxAODTrigRingerAthenaPooltempfile
return $cmtcleanupstatus

