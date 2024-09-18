#!/usr/bin/perl -w

require ( "./bench_common.pl");

$trace_dir_1BLN = "../TRACES/";
#$trace_dir_4BLN = "../TRACES.4BLN/";

$trace_dir=$trace_dir_1BLN;

$filetype  = ".mtf.gz";

#####################################
######### USAGE OPTIONS      ########
#####################################

$wsuite   = "all";
$sim_exe  = "../src/sim";
$dest_dir  = ".";
$sim_opt   = "  ";
$debug     = 0;
$rerun     = 0;
$firewidth = 2; # num parallel jobs at a time
$singlebmk = 0;
$qsub      = 0; 
$sleep     = 2;

#####################################
######### USAGE OPTIONS      ########
#####################################

sub usage(){

$USAGE = "Usage:  '$0 <-option> '";
 
print(STDERR "$USAGE\n");
print(STDERR "\t--h                    : help -- print this menu. \n");
print(STDERR "\t--r                    : rerun -- re-execute killed sims. \n");
print(STDERR "\t--d <dest_dir>         : name of the destination dir. \n");
print(STDERR "\t--w <workload/suite>   : workload suite from bench_common \n");
print(STDERR "\t--b <benchmark>        : run a single benchmark \n");
print(STDERR "\t--s <sim_exe>          : simulator executable \n");
print(STDERR "\t--o <sim_options>      : simulator options \n");
print(STDERR "\t--dbg                  : debug \n");
print(STDERR "\t--f <val>              : firewidth, num of parallel jobs \n");
print(STDERR "\t--z <val>              : sleep for z seconds \n");
print(STDERR "\t--n                    : insert nohup \n");
print(STDERR "\n");

exit(1);
}

######################################
########## PARSE COMMAND LINE ########
######################################
 
while (@ARGV) {
    $option = shift;
 
    if ($option eq "--dbg") {
        $debug = 1;
    }elsif ($option eq "--w") {
        $wsuite = shift;
    }elsif ($option eq "--b") {
        $singlebmk = shift;
    }elsif ($option eq "--r") {
        $rerun = 1;
    }elsif ($option eq "--q") {
        $qsub = 1;
    }elsif ($option eq "--s") {
        $sim_exe = shift;
    }elsif ($option eq "--o") {
        $sim_opt = shift;
    }elsif ($option eq "--d") {
        $dest_dir = shift;
    }elsif ($option eq "--f") {
        $firewidth = shift;
    }elsif ($option eq "--z") {
        $sleep = shift;
    }else{
	usage();
        die "Incorrect option ... Quitting\n";
    }
}

##########################################################             
# create full path to mysim and dest dir
$mysim = "$dest_dir"."/"."sim.bin";
$myopt = "$dest_dir"."/"."sim.opt";

##########################################################
# get the suite names, num_w, and num_p from bench_common

if($rerun){
    get_rerun_expts();
}elsif($singlebmk){
    $w[0] = $singlebmk;
}else{
    die "No benchmark set '$wsuite' defined in bench_common.pl\n"
        unless $SUITES{$wsuite};
    @w = split(/\s+/, $SUITES{$wsuite});
}

$num_w = scalar @w;


##########################################################

if($debug==0 && $rerun==0){
    mkdir "$dest_dir";
    system ("ls $dest_dir");
    system ("cp $sim_exe $mysim");
    system ("chmod +x $mysim");
    system ("echo $sim_opt > $myopt");
}

##########################################################

for($ii=0; $ii< $num_w; $ii++){
    $wname = $w[$ii];
    $wname = $MPROG_PARAM{ $w[$ii] } if($MPROG_PARAM{ $w[$ii] });

    $infiles = " ";

    $auto_sim_opt = " ";

    if ( $SIMOPT_PARAM{$w[$ii]}){
	$auto_sim_opt = $SIMOPT_PARAM{ $w[$ii] } 
    }

    @bmks = split/-/,$wname;
    
    qsub_set_trace_dir() if($qsub);
    
    foreach (@bmks) {
	$bname   = $_;
	$infile  = $trace_dir.$bname.$filetype;
	$infiles = $infiles." ".$infile;
    }
    
    $outfile = $dest_dir. "/". $w[$ii] . ".res";
    
    #$exe = "nohup $mysim  $auto_sim_opt $sim_opt $infiles > $outfile ";
    $exe = "$mysim  $auto_sim_opt $sim_opt $infiles > $outfile ";

    #background all jobs except the last one (acts as barrier)
    $exe .= " &" unless($ii%$firewidth==($firewidth-1));
    print "$exe\n";
    system("$exe") unless($debug);

}
