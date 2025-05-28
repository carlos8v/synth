import librosa
from oscillators.triangle import TriangleOscillator
from oscillators.modulated_oscilator import ModulatedOscillator
from modulators.panner import Panner
from modulators.volume import ModulatedVolume
from modulators.adsr_envelope import ADSREnvelope
from wave_adder import WaveAdder
from chain import Chain
from file import wave_to_file

SR = 44_100


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


def get_val(osc, count=SR, it=False):
    if it:
        osc = iter(osc)
    # returns 1 sec of samples of given osc.
    return [next(osc) for i in range(count)]


def make_chord_key(key, osc=TriangleOscillator):
    chord = get_chord(key)
    gen = WaveAdder(
        Chain(
            ModulatedOscillator(osc(librosa.note_to_hz(chord[0]))),
            ModulatedVolume(ADSREnvelope(0.5, 0.2, 0.7, 0.5)),
            Panner(0.3),
        ),
        Chain(
            ModulatedOscillator(osc(librosa.note_to_hz(chord[1]))),
            Panner(0.5),
            ModulatedVolume(ADSREnvelope(0.5, 0.2, 0.7, 0.5)),
        ),
        Chain(
            ModulatedOscillator(osc(librosa.note_to_hz(chord[2]))),
            Panner(0.7),
            ModulatedVolume(ADSREnvelope(0.5, 0.2, 0.7, 0.5)),
        ),
        stereo=True,
    )

    return gen


def make_sound(keys, osc=TriangleOscillator, note_lens=[1]):
    notes = []
    for key, note_len in zip(keys, note_lens):
        notes += gettrig(make_chord_key(key, osc=osc), note_len)

    return notes


def gettrig(gen, downtime, sample_rate=SR):
    gen = iter(gen)
    down = int(downtime * sample_rate)
    vals = get_val(gen, down)
    gen.trigger_release()
    while not gen.ended:
        vals.append(next(gen))
    return vals


# --------------------------

notes = [2, 0, 1, 3, 5, 4, 0]
note_lens = [0.25, 0.25, 0.30, 0.40, 1, 1.25, 1]

wav = make_sound(notes, note_lens=note_lens)
wave_to_file(wav, None, fname="./sounds/test.wav")
