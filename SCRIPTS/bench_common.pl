#!/usr/bin/perl -w

#******************************************************************************
# Benchmark Sets
# ************************************************************

%SUITES = ();



$SUITES{'gap6'}      = 
	      'cc
	       pr
	       tc
	       bfs
               bc
               sssp';


$SUITES{'stream4'}      = 
	      'add
	       triad
	       copy
               scale';



$SUITES{'spec2k17'}      = 
'blender_17
bwaves_17
cactuBSSN_17
cam4_17
deepsjeng_17
fotonik3d_17
gcc_17
imagick_17
lbm_17
leela_17
mcf_17
nab_17
namd_17
omnetpp_17
parest_17
perlbench_17
povray_17
roms_17
wrf_17
x264_17
xalancbmk_17
xz_17';


$SUITES{'mint32'}      = 
    "$SUITES{'spec2k17'}"."\n".
    "$SUITES{'gap6'}"."\n".
    "$SUITES{'stream4'}";

