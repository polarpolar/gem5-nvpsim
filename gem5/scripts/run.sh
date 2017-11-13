#!/bin/sh

build/ARM/gem5.debug --debug-flags=EnergyMgmt --debug-file=a.out configs/example/se_engy.py -c tests/test-progs/hello/bin/arm/linux/hello --cpu-clock='1MHz' --energy-modules='cpu' --thres-low=5000 --thres-high=10000 --energy-prof=./profile/solar_10us_30s.txt --energy-time-unit=10000000
