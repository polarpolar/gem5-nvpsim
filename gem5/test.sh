#rm m5out/batch_res.csv

echo "========================================================="
energy=0.2
cap=5
echo "cap: $cap; entergy: $energy"
build/ARM/gem5.debug --debug-flag=EnergyMgmt,VirtualDevice --debug-file=virtual_device.o configs/example/test_engy_vdev.py $cap $energy -j2


#for energy in {100..100}
#do
#	for cap in {100..100}
#	do
#		echo "========================================================="
#		echo "cap: $cap; energy: $energy"
#	 	build/ARM/gem5.debug configs/example/LiuRuoyang_engy_vdev_batch.py $cap $energy
#	done
#done

