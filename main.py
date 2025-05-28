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


def get_chord(key):
    chords = [
        ["C4", "E4", "G4"],
        ["D4", "F4", "A4"],
        ["E4", "G4", "B4"],
        ["F4", "A4", "C5"],
        ["G4", "B4", "D5"],
        ["A4", "C5", "E5"],
        ["B4", "D5", "F5"],
    ]
    return chords[key]


def make_song(keys, osc=TriangleOscillator(), note_lens=[1]):
    s1 = []
    s2 = []
    s3 = []

    for key in keys:
        chord = get_chord(key)
        s1.append(chord[0])
        s2.append(chord[1])
        s3.append(chord[2])

    return (
        np.array(get_seq(osc, notes=s1, note_lens=note_lens))
        + np.array(get_seq(osc, notes=s2, note_lens=note_lens))
        + np.array(get_seq(osc, notes=s3, note_lens=note_lens))
    )


notes = [2, 0, 1, 3, 5, 4, 0]
note_lens = [0.25, 0.25, 0.30, 0.40, 1, 1.25, 1]

wav = make_song(
    notes, osc=TriangleOscillator(amp=0.5), note_lens=note_lens
) + make_song(notes, osc=SineOscillator(), note_lens=note_lens)

wave_to_file(wav, None, fname="./sounds/test.wav")
