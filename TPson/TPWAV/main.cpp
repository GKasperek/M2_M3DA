#include <iostream>
#include "wavdata.h"
#include "fft.h"
#include <math.h>

#define DELAY 6000
#define AMPLITUDE 0.5

int main(int argc, char **argv)
{
	WavData w;
    WavData testw;
    w.load("COW.WAV");

	char *data = w.data();
	char *data2 = new char[w.datasize()*2];

    char *sonPur = new char[w.datasize()];

	int i;
	for(i=0;i<w.datasize();i++)
		data2[i]=data[i];
	for(;i<w.datasize()*2;i++)
		data2[i]=128;

    for(int i=DELAY;i<2*w.datasize();i++)
    {
        float value = (float)(unsigned char)data2[i-DELAY]-128.0;
        value = value * AMPLITUDE;
        int val = (unsigned int)value + (unsigned char)data2[i];
		if(val>255)val=255;
		if(val<0)val=0;

        data2[i]= (unsigned char)(unsigned int)val;
	}

    for(int j = 0; j < w.datasize(); j++){
        sonPur[j] = (unsigned char)(unsigned int)110;
    }


	w.clearData();
	w.setDatasize(w.datasize()*2);
	w.setData(data2);

    testw.setDatasize(w.datasize());
    testw.setData(sonPur);

    testw.save("TEST.WAV");
	w.save("COW_d.WAV");
}
