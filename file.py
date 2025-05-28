import numpy as np
from scipy.io import wavfile

SR = 44_100


def to_16(wav, amp):
    return np.int16(wav * amp * (2**15 - 1))


def wave_to_file(wav, wav2=None, fname="temp.wav", amp=0.1, sample_rate=SR):
    wav = np.array(wav)
    wav = to_16(wav, amp)
    if wav2 is not None:
        wav2 = np.array(wav2)
        wav2 = to_16(wav2, amp)
        wav = np.stack([wav, wav2]).T

    wavfile.write(fname, SR, wav)
