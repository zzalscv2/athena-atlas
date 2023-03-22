#!/bin/bash

if [[ $# < 3 ]];
then
  echo "Syntax: $0 [-append] [-offline] [-onerun] <Run> <LBb> [<LBe>] File1 [Folder] ..."
  exit
fi

inputTextFile="mf_input.txt"
outputTextFile="mf_output.txt"
outputSqlite="MissingFEBs.db"
outputSqliteOnl="MissingFEBsOnl.db"
oldTextFile="mf_previous.txt"
diffTextFile="mf_diff.txt"
BaseTagName="LARBadChannelsOflMissingFEBs-RUN2-UPD3-01"

if [ $1 == "-append" ]
then
    echo "Appending to previous bad-FEB list"
    catfiles=$oldTextFile
    shift
else
    catfiles=""
fi

if [ $1 == "-offline" ]
then
    echo "Creating only offline tags"
    online=0
    shift
else
    online=1 
fi

if [ $1 == "-onerun" ]
then
    echo "Creating closed IOV, for one run only"
    onerun=1
    shift
else
    onerun=0 
fi

if echo $1 | grep -q "^[0-9]\{5,\}$";
then
    runnumber=$1
    shift
else
    echo "ERROR: Expected a run-number, got $1"
    exit
fi


if echo $1 | grep -q "^[0-9]*$";
then
    lbnumber=$1
    shift
else
    echo "ERROR: Expected a lumi-block-number, got $1"
    exit
fi

#if [ $onerun == 1 ]
#then
  if echo $1 | grep -q "^[0-9]*$";
  then
    lbnumbere=$1
    shift
  else  
    lbnumbere=-1
  fi  
#  else
#    echo "ERROR: Expected a lumi-block-number, got $1"
#    exit
#  fi
#fi

if ! which AtlCoolCopy 1>/dev/null 2>&1
then
    echo "No offline setup found!"
    exit
fi

if [ -f $inputTextFile ];
then
  echo "Temporary file $inputTextFile exists already. Please remove!"
  exit
fi

if [ -f $outputTextFile ];
then
  echo "Temporary file $outputTextFile exists already. Please remove!"
  exit
fi

if [ -f $outputSqlite ];
then
  echo "Output file $outputSqlite exists already. Please remove!"
  exit
fi

if [ -f $outputSqliteOnl ];
then
  echo "Output file $outputSqliteOnl exists already. Please remove!"
  exit
fi

if [ -f $oldTextFile ];
then
  echo "Output file $oldTextFile exists already. Please remove!"
  exit
fi


if [ -f ${outputSqlite}.tmp ];
then
    rm -f ${outputSqlite}.tmp
fi

echo "Left parameters: " $# $1

if [ ! -f $1 ];
    then
    echo "ERROR File $1 not found!"
    exit
fi
echo "Adding $1"
catfiles="${catfiles} ${1%%:}"
shift

echo "Left parameters: " $# $1
if [[ $# > 0 ]]
then
   Folder=$1
else   
   Folder="/LAR/BadChannelsOfl/MissingFEBs"
fi

#Get UPD4-nn tag connected to 'current':
echo "Resolving current folder-level tag suffix for ${Folder} ...."
fulltag=`getCurrentFolderTag.py "COOLOFL_LAR/CONDBR2" $Folder` 
upd4TagName=`echo $fulltag | grep -o "RUN2-UPD4-[0-9][0-9]"` 
echo "Found $upd4TagName"
fulltages=`getCurrentFolderTag.py "COOLOFL_LAR/CONDBR2" $Folder True` 
gtages=`echo $fulltages | grep ES1PA | awk '{print $2}'`
upd1TagName=`echo $fulltages | grep -o "RUN2-UPD1-[0-9][0-9]"` 
echo "Found $upd1TagName"

#create a tag string from folder
IFS='/' 
read -r -a array <<< "$Folder"
fldtag=""
for i in ${array[@]}
do
  fldtag+=$i
done
IFS=' ' 



echo "Running athena to read current database content..."
athena.py -c "OutputFile=\"${oldTextFile}\";RunNumber=${runnumber};LBNumber=${lbnumber};Folder=\"${Folder}\";GlobalTag=\"${gtages}\"" LArBadChannelTool/LArMissingFebs2Ascii.py > oracle2ascii.log 2>&1
if [ $? -ne 0 ];  then
    echo "Athena reported an error reading back sqlite file ! Please check oracle2ascii.log!"
    exit
fi

echo "cat the files:"$catfiles":to " $inputTextFile
if [ ! -f "mf_previous.txt" ];
    then
    echo "ERROR File mf_previous.txt not found!"
    exit
fi
if [ ! -f "new.txt" ];
    then
    echo "ERROR File new.txt not found!"
    exit
fi
#cat $catfiles > $inputTextFile
#cat mf_previous.txt new.txt > $inputTextFile
cp mf_previous.txt $inputTextFile
cat new.txt >> $inputTextFile
if [ $? -ne 0 ];  then
    echo "Failed to concatinate input files!"
    exit
fi

if [ $onerun -eq 1 ]; then
 if [ $lbnumbere -ge 0 ]; then
   endlb=$[ $lbnumbere + 1]
   prefix="IOVEndRun=${runnumber};IOVEndLB=$endlb;"
 else
   prefix=$[ $runnumber + 1]
   prefix="IOVEndRun=${prefix};IOVEndLB=0;"
 fi  
else 
 prefix=""
fi

echo "TagSuffix: " $upd4TagName
echo "Running athena to build sqlite database file ..."
prefix="${prefix}IOVBeginRun=${runnumber};IOVBeginLB=${lbnumber};sqlite=\"${outputSqlite}.tmp\";Folder=\"${Folder}\";GlobalTag=\"${gtages}\";"
echo "prefix: ${prefix}"
athena.py -c $prefix LArBadChannelTool/LArMissingFebDbAlg.py > ascii2sqlite.log 2>&1

if [ $? -ne 0 ];  then
    echo "Athena reported an error! Please check ascii2sqlite.log!"
    exit
fi

 
if grep -q ERROR ascii2sqlite.log
then
    echo "An error occured during ascii2sqlite job! Please check ascii2sqlite.log!"
    exit
fi

if grep -q "REJECTED" ascii2sqlite.log
then
    echo "ERROR: At least one line in the input text file could not be read. Syntax Error?"
fi


cp ${outputSqlite}.tmp ${outputSqlite}

if [ $onerun -eq 1 ] || [ $lbnumbere -ge 0 ]; then
   pref="RunNumber=${runnumber};LBNumber=${lbnumber};"
else   
   pref=""
fi
pref="${pref}sqlite=\"${outputSqlite}\";OutputFile=\"${outputTextFile}\";Folder=\"${Folder}\";GlobalTag=\"${gtages}\";tag=\"${fldtag}-RUN2-UPD3-01\""
echo "Running athena to test readback of sqlite database file"
athena.py  -c ${pref} LArBadChannelTool/LArMissingFebs2Ascii.py > sqlite2ascii.log 2>&1

if [ $? -ne 0 ];  then
    echo "Athena reported an error reading back sqlite file ! Please check sqlite2ascii.log!"
    exit
fi


if grep -q ERROR sqlite2ascii.log
then
    echo "An error occured during reading back sqlite file ! Please check sqlite2ascii.log!"
    exit
fi

if [ $online -eq 1 ]; then
   echo "Copying UPD3 to UPD1 tag..."
   AtlCoolCopy "sqlite://;schema=${outputSqlite}.tmp;dbname=CONDBR2" "sqlite://;schema=${outputSqlite};dbname=CONDBR2" -f ${Folder} -t ${fldtag}-RUN2-UPD3-01 -ot ${fldtag}-${upd1TagName} -r 2147483647 -a  > AtlCoolCopy.upd3.log 2>&1

   if [ $? -ne 0 ];  then
       echo "AtlCoolCopy reported an error! Please check AtlCoolCopy.upd3.log!"
       exit
   fi
fi

echo "Copying UPD3 to UPD4 tag..."
AtlCoolCopy "sqlite://;schema=${outputSqlite}.tmp;dbname=CONDBR2" "sqlite://;schema=${outputSqlite};dbname=CONDBR2" -f ${Folder} -t ${fldtag}-RUN2-UPD3-01 -ot ${fldtag}-$upd4TagName  > AtlCoolCopy.upd4.log 2>&1

if [ $? -ne 0 ];  then
    echo "AtlCoolCopy reported an error! Please check AtlCoolCopy.upd4.log!"
    exit
fi

if ! grep -q ${runnumber} /afs/cern.ch/user/a/atlcond/scratch0/nemo/prod/web/calibruns.txt
    then
    echo " *** WARNING *** Run ${runnumber} is not on the NEMO watchlist! Outside of CalibLoop? Not a stable-beam run?" 
fi


if [ $online -eq 1 ]; then
   echo "Copying to for online database..."
   #create a online folder name
   IFS='/' 
   read -r -a array1 <<< "$Folder"
   onlfld=""
   for i in ${array1[@]}
   do
     if [[ $i == "BadChannelsOfl" ]] 
     then 
         onlfld+="/BadChannels"
         continue
     fi    
     onlfld+="/"$i
   done
   onlfld=${onlfld##/}
   #create the online tag
   read -r -a array2 <<< "$onlfld"
   onlfldtag=""
   for i in ${array2[@]}
   do
     onlfldtag+=$i
   done
   IFS=' ' 

   echo "Copying to the: "${onlfld} " with tag " ${onlfldtag}-${upd1TagName}
   AtlCoolCopy "sqlite://;schema=${outputSqlite}.tmp;dbname=CONDBR2" "sqlite://;schema=${outputSqliteOnl};dbname=CONDBR2" -f ${Folder} -t ${fldtag}-RUN2-UPD3-01 -of ${onlfld} -ot ${onlfldtag}-${upd1TagName} -r 2147483647 -a -c > AtlCoolCopy.onl.log 2>&1


   if [ $? -ne 0 ];  then
      echo "AtlCoolCopy reported an error! Please check AtlCoolCopy.onl.log!"
      exit
   fi
fi

if [ -f $diffTextFile ]
then 
    rm -rf $diffTextFile
fi

diff $oldTextFile $outputTextFile > $diffTextFile

nNew=`grep -c ">" $diffTextFile`
nGone=`grep -c "<" $diffTextFile`
nTotal=`wc -l $outputTextFile | cut -f 1 -d " "`

echo "Added $nNew missing FEBs and removed $nGone"
echo "Total number of FEBs in the new list: $nTotal" 


echo "Output files:"
echo "$outputSqlite: Containing UPD3 version of the missing FEB list for the OFFLINE DB. UPD3 valid as of run $runnumber"
if [ $online -eq 1 ]; then
   echo "            and UPD1 for the OFFLINE DB"
   echo "$outputSqliteOnl: Containing UPD1 version of the missing FEB list for the ONLINE DB"
fi   
echo "$outputTextFile: Text version of the new bad channel list (read back from sqlite)"

echo "" 
echo "Upload to OFFLINE oracle server using"
echo "/afs/cern.ch/user/a/atlcond/utilsflask/AtlCoolMerge.py --flask ${outputSqlite} CONDBR2 ATONR_COOLOFL_GPN ATLAS_COOLOFL_LAR_W <password>"
echo ""
if [ $online -eq 1 ]; then
  echo "Upload to ONLINE oracle server using"
  echo "/afs/cern.ch/user/a/atlcond/utils22/AtlCoolMerge.py --online ${outputSqliteOnl} CONDBR2 ATONR_COOL ATLAS_COOLONL_LAR_W <password>"
fi


