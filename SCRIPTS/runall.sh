
############################################################################
suite="spec2k17"
fw=8
simopt="-inst_limit 100000000 -ratemode 8 -dramrefpolicy 2  -drammappolicy 3 -memclosepage"
###########################################################################

echo "Compiling simulator ... this may take a few seconds"

cd ../src ; make; cd ../SCRIPTS

sleep 10

echo "Running sims ... this may take about 2 hours on an 8-core machine"

./runall.pl --w $suite --f $fw  --d "../RESULTS/A.BASE" --o  "$simopt ";

./runall.pl --w $suite --f $fw  --d "../RESULTS/A.RFMSB.T16" --o  "$simopt -dramrfmpolicy 1 -rfmth 16 -rfmrefth 16 -rfmraammt 48";
./runall.pl --w $suite --f $fw  --d "../RESULTS/A.RFMSB.T32" --o  "$simopt -dramrfmpolicy 1 -rfmth 32 -rfmrefth 32 -rfmraammt 96 ";

echo "Sleeping for 10 minutes to ensure ALL sims finish"

sleep 600

echo "Perf sims most likely done ... Please do top and check if any sim.bin is still running"

sleep 30


echo "Plotting graph (takes 15 seconds. if fails, most likely sims are running ... try below line again)"

echo "./getdata -i -g -printmask "0-1-1"   -n 0 -w spec2k17 -d ../RESULTS/A.BASE/ ../RESULTS/A.RFMSB.T16 ../RESULTS/A.RFMSB32.T32 > rfmsb.txt ; python rfmsb.py"

./getdata -i -g -printmask "0-1-1"   -n 0 -w spec2k17 -d ../RESULTS/A.BASE/ ../RESULTS/A.RFMSB.T16 ../RESULTS/A.RFMSB32.T32 > rfmsb.txt ; python rfmsb.py



# uncomment the below if you want to run the DRFM results (it may take 2 hours to run)

# ./runall.pl --w $suite --f $fw  --d "../RESULTS/A.DRFMSB.T16.L240" --o  "$simopt -dramrfmpolicy 1 -rfmth 16 -rfmrefth 0 -rfmraammt 16 -tRFM 960";
# ./runall.pl --w $suite --f $fw  --d "../RESULTS/A.DRFMSB.T32.L240" --o  "$simopt -dramrfmpolicy 1 -rfmth 32 -rfmrefth 0 -rfmraammt 32 -tRFM 960";
# ./runall.pl --w $suite --f $fw  --d "../RESULTS/A.DRFMSB.T72.L240" --o  "$simopt -dramrfmpolicy 1 -rfmth 72 -rfmrefth 0 -rfmraammt 72 -tRFM 960";

#./getdata -i -g -printmask "0-1-1"   -n 0 -w spec2k17 -d ../RESULTS/A.BASE/ ../RESULTS/A.DRFMSB* > drfm.txt 
