#!/bin/perl
#before running the script remember to do:
#setupATLAS
#localSetupRucioClients
#voms-proxy-init -voms atlas

$num_args = $#ARGV + 1;
if ($num_args<2) {
    print "\nUsage: makeDataRunList.pl <GRL> <stream> <year>\n";
    exit;
}

my $GRL=$ARGV[0];
my $year=$ARGV[1];


my @runlist=`./getRunListinGRL.sh $GRL`;

foreach my $run (@runlist) {
    chop($run);
    print "# Run $run\n";
    my $cmd = "rucio list-dids --filter type=container data23_13p6TeV.*$run.physics_Main.merge.AOD* | sort";
    #print $cmd;
    @dslist = `$cmd`;
    foreach my $ds (@dslist) {
        chop($ds);
        $dataset = substr ($ds, 17,-22); #cut "| CONTAINER"

        if ($dataset=~ /data/) {
            print ("$dataset\n");  
            # my $cmd2 = "./pathena_run3DQTestingDriver.sh $dataset";

        }
    }
}