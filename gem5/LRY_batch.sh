#rm m5out/batch_res.csv

energy = 10
cap = 10
echo "cap: $cap; energy: $energy"
build/ARM/gem5.debug --debug-flag=VirtualDevice --debug-file=virtual_device.o configs/example/LiuRuoyang_engy_vdev.py -j2 $cap $energy


#for energy in {100..100}
#do
#	for cap in {100..100}
#	do
#		echo "========================================================="
#		echo "cap: $cap; energy: $energy"
#	 	build/ARM/gem5.debug configs/example/LiuRuoyang_engy_vdev_batch.py $cap $energy
#	done
#done

