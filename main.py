import librosa
import numpy as np
from scipy.io import wavfile
from oscillators.sine import SineOscillator
from oscillators.square import SquareOscillator
from oscillators.sawtooth import SawtoothOscillator
from oscillators.triangle import TriangleOscillator
from waveform import WaveAdder

SR = 44_100


def get_val(osc, sample_rate=SR):
    return [next(osc) for i in range(sample_rate)]


def get_seq(osc, notes=["C4", "E4", "G4"], note_lens=[0.5, 0.5, 0.5]):
    samples = []
    osc = iter(osc)
    for note, note_len in zip(notes, note_lens):
        osc.freq = librosa.note_to_hz(note)
        for _ in range(int(SR * note_len)):
            samples.append(next(osc))
    return samples


def to_16(wav, amp):
    return np.int16(wav * amp * (2**15 - 1))


def wave_to_file(wav, wav2=None, fname="temp.wav", amp=0.1):
    wav = np.array(wav)
    wav = to_16(wav, amp)
    if wav2 is not None:
        wav2 = np.array(wav2)
        wav2 = to_16(wav2, amp)
        wav = np.stack([wav, wav2]).T

    wavfile.write(fname, SR, wav)


# Testing generating sounds
# wav = get_seq(SineOscillator())
# wave_to_file(wav, fname="./sounds/c4_maj_sine.wav")

# wav = get_seq(SquareOscillator())
# wave_to_file(wav, fname="./sounds/c4_maj_square.wav")

# wav = get_seq(SawtoothOscillator())
# wave_to_file(wav, fname="./sounds/c4_maj_saw.wav")

# wav = get_seq(TriangleOscillator())
# wave_to_file(wav, fname="./sounds/c4_maj_triangle.wav")

dur = 4
s1 = ["C4", "E4", "G4", "B4"] * dur
l1 = [0.25, 0.25, 0.25, 0.25] * dur

s2 = ["C3", "E3", "G3"] * dur
l2 = [0.333334, 0.333334, 0.333334] * dur

s3 = ["C2", "G2"] * dur
l3 = [0.5, 0.5] * dur

wavl = (
    np.array(get_seq(TriangleOscillator(amp=0.8), notes=s1, note_lens=l1))
    + np.array(get_seq(SineOscillator(), notes=s2, note_lens=l2))
    + np.array(get_seq(TriangleOscillator(amp=0.4), notes=s3, note_lens=l3))
)

wavr = (
    np.array(get_seq(TriangleOscillator(amp=0.8), notes=s1[::-1], note_lens=l1))
    + np.array(get_seq(SineOscillator(), notes=s2[::-1], note_lens=l2))
    + np.array(get_seq(TriangleOscillator(amp=0.4), notes=s3[::-1], note_lens=l3))
)

wave_to_file(wavl, wavr, fname="./sounds/c_maj7.wav")
