main_trans: delay.o peripheral.o fourierf.o encode.o
	aarch64-linux-gnu-gcc-5 -o main_trans main_trans.c delay.o peripheral.o fourierf.o fftmisc.o encode.o -lm -static

delay.o:  delay.c delay.h
	aarch64-linux-gnu-gcc-5 -c delay.c

peripheral.o: peripheral.c peripheral.h reg51.h
	aarch64-linux-gnu-gcc-5 -c peripheral.c

fourierf.o: fourierf.c fourier.h ddcmath.h fftmisc.o reg51.h
	aarch64-linux-gnu-gcc-5 -c fourierf.c fftmisc.c -lm

fftmisc.o: fftmisc.c fourier.h
	aarch64-linux-gnu-gcc-5 -c fftmisc.c -lm

encode.o: encode.c encode.h reg51.h
	aarch64-linux-gnu-gcc-5 -c encode.c

clean:
	rm main_trans delay.o peripheral.o fourierf.o fftmisc.o encode.o