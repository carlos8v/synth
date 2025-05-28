SR = 44_100


def amp_mod(init_amp, env):
    return env * init_amp


def freq_mod(init_freq, env, mod_amt=0.01, sustain_level=0.7):
    return init_freq + ((env - sustain_level) * init_freq * mod_amt)


def getdownlen(env, suslen, sample_rate=SR):
    n = sum(env.attack_duration, env.release_duration, suslen)
    return int(n * sample_rate)


# def getadsr(a, d, sl, sd, r, Osc=SquareOscillator(55), mod=None):
#     if mod is None:
#         mod = ModulatedOscillator(Osc, ADSREnvelope(a, d, sl, r), amp_mod=amp_mod)
#     downtime = a + d + sd
#     return gettrig(mod, downtime)
