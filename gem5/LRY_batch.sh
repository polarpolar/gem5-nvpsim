#rm m5out/batch_res.csv

for energy in {20..20}
do
	for cap in {101..1000}
	do
		echo "========================================================="
		echo "cap: $cap; energy: $energy"
	 	build/ARM/gem5.debug configs/example/LiuRuoyang_engy_vdev_batch.py $cap $energy
	done
done

