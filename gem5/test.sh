rm m5out/batch_res.csv

echo "========================================================="
cap=6
energy=3
echo "cap: $cap; entergy: $energy"
build/ARM/gem5.debug --debug-flag=EnergyMgmt --debug-file=virtual_device.o configs/example/test_engy_vdev.py $cap $energy -j2

exit

for energy in {100..100}
do
	for cap in {1..10}
	do
		echo "========================================================="
		echo "cap: $cap; energy: $energy"
	 	build/ARM/gem5.debug configs/example/test_engy_vdev.py $cap $energy
	done
done

