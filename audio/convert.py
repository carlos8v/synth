import soundfile as sf
import sys

def toHeaderFile(name):
    data, fs = sf.read(''+name+'.wav')
    with open('./'+name+'.h', 'w') as f:
        print('processing', name)
        print('#pragma once',file=f)
        print(file=f)
        print('const static float '+name+'_vector[] = {', file=f)
        for idx, v in enumerate(data[0:-1]):
            print(v, end='', file=f)
            print(',', end='', file=f)
            if idx%100==0:
                print(file=f)
        print(data[-1],'};',file=f)

toHeaderFile('click')
print('END')
