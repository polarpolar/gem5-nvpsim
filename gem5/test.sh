rm m5out/batch_res.csv

echo "========================================================="
cap=5
energy=1
echo "cap: $cap; entergy: $energy"
#build/ARM/gem5.debug configs/example/test_engy_vdev.py $cap $energy
build/ARM/gem5.debug --debug-flag=VirtualDevice,EnergyMgmt --debug-file=virtual_device.o configs/example/test_engy_vdev.py $cap $energy -j2

exit

for energy in {2..2}
do
	for cap in {1..10}
	do
		echo "========================================================="
		echo "cap: $cap; energy: $energy"
	 	build/ARM/gem5.debug configs/example/test_engy_vdev.py $cap $energy
	done
done

