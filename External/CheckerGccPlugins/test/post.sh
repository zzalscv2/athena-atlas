#!/bin/sh
#/** @file post.sh
# @brief sh script that check the return code of an executable and compares
# its output with a reference (if available).
# @param test_name
#
# @author Scott Snyder <snyder@fnal.gov> - ATLAS Collaboration.
# @author Paolo Calafiura <pcalafiura@lbl.gov> - ATLAS Collaboration.
# $Id: post.sh,v 1.23 2007-11-10 00:00:07 calaf Exp $
# **/
test=$1
extrapatterns="$2"
#verbose="1"
if [ "$POST_SH_NOCOLOR" = "" ]; then
 GREEN="[92;1m"
 YELLOW="[93;1m"
 RED="[97;101;1m"
 RESET="[m"
else
 GREEN=""
 YELLOW=""
 RED=""
 RESET=""
fi

# consider these name pairs identical in the diff
read -d '' II <<EOF
s/.enabled by default.//
s!^/.*/test/!../test/!
s! /.*/test/! ../test/!
s/:[0-9]*: note:/:: note:/
s/:[0-9]*: warning:/:: warning:/
EOF

# ignore diff annotations
PP='^---|^[[:digit:]]+[acd,][[:digit:]]+'

# ignore hex addresses
PP="$PP"'|0x\w{4,}'



if [ "$extrapatterns" != "" ]; then
 PP="$PP""|$extrapatterns"
fi

if [ -z "$testStatus" ]
   then
   echo "$YELLOW post.sh> Warning: athena exit status is not available $RESET"
else
   # check exit status
   joblog=${test}.log
   if [ "$testStatus" = 0 ]
       then
       if [ "$verbose" != "" ]; then
         echo "$GREEN post.sh> OK: ${test} exited normally. Output is in $joblog $RESET"
       fi
       reflog=../share/${test}.ref
       if [ -r $reflog ]
           then
	   jobrep=${joblog}-rep
	   sed "$II" $joblog > $jobrep
	   refrep=`basename ${reflog}`-rep
	   sed "$II" $reflog > $refrep
           jobdiff=${joblog}-todiff
           refdiff=`basename ${reflog}`-todiff
           egrep -a -v "$PP" < $jobrep > $jobdiff
           egrep -a -v "$PP" < $refrep > $refdiff
           diff -a -b -E -B -u $jobdiff $refdiff
           diffStatus=$?
           if [ $diffStatus != 0 ] ; then
               echo "$RED post.sh> ERROR: $joblog and $reflog differ $RESET"
           else
               if [ "$verbose" != "" ]; then
                 echo "$GREEN post.sh> OK: $joblog and $reflog identical $RESET"
               fi
           fi
       else
           tail $joblog
           echo "$YELLOW post.sh> WARNING: reference output $reflog not available $RESET"
           echo  " post.sh> Please check ${PWD}/$joblog"
       fi
   else
       tail $joblog
       echo  "$RED post.sh> ERROR: Athena exited abnormally! Exit code: $testStatus $RESET"
       echo  " post.sh> Please check ${PWD}/$joblog"
   fi
fi
exit $testStatus
