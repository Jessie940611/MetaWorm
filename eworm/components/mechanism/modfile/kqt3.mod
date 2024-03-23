TITLE kqt3 current for C. elegans neuron model


COMMENT
voltage-gated potassium channel
ENDCOMMENT


UNITS {
    (nS) = (nanosiemens)
    (mV) = (millivolt)
    (pA) = (picoamp)
    (um) = (micron)
}


NEURON {
    SUFFIX kqt3
    USEION k READ ek WRITE ik
    RANGE gbkqt3, minf, tmf, tms, winf, sinf, tw, ts, mf, ms, w, s
}


PARAMETER {
    gbkqt3 = 1 (nS/um2)
    vhm = -12.6726 (mV)
    ka = 15.8008 (mV)
    vhw = -1.084 (mV)
    kiw = 28.78 (mV)
    aw = 0.49
    bw = 0.51
    vhs = -45.3 (mV)
    kis = 12.3 (mV)
    as = 0.34
    bs =  0.66
    atmf = 395.3 (ms)
    btmf = 38.1 (mV)
    ctmf = 33.59 (mV)
    atms = 5503.0 (ms)
    btms = -5345.4 (ms)
    ctms = 0.02827 (1/mV)
    dtms = -23.9 (mV)
    etms = -4590.6 (ms)
    ftms = 0.0357 (1/mV)
    gtms = 14.15 (mV)
    atw = 0.544 (ms)
    btw = 29.2 (ms)
    ctw = -48.09 (mV)
    dtw = 48.83 (mV)
    ats = 500e3 (ms)
}


ASSIGNED {
    v (mV)
    ek (mV)
    ik (pA/um2)
    minf
    tmf (ms)
    tms (ms)
    winf
    sinf
    tw (ms)
    ts (ms)
}


STATE {
    mf
    ms
    w
    s
}


BREAKPOINT {
    SOLVE states METHOD cnexp
    ik = gbkqt3 * (0.3 * mf + 0.7 * ms) * w * s * (v - ek)
    :ik = gbkqt3 * (0.3 * mf + 0.7 * ms) * (v + 80)
    :ek = -80
}


INITIAL {
    setparames(v)
    mf = 0
    ms = 0
    w = winf
    s = sinf
}


DERIVATIVE states {
    setparames(v)
    mf' = (minf - mf) / tmf
    ms' = (minf - ms) / tms
    w' = (winf - w) / tw
    s' = (sinf - s) / ts
}


PROCEDURE setparames(v (mV)) {
    minf = 1 / (1 + exp(-(v - vhm) / ka))
    tmf = atmf / (1 + ((v + btmf) / ctmf) ^ 2)
    tms = atms + btms / (1 + 10 ^ (-ctms * (dtms - v))) + etms / (1 + 10 ^ (-ftms * (gtms + v)))
    winf = aw + bw / (1 + exp((v - vhw) / kiw))
    sinf = as + bs / (1 + exp((v - vhs) / kis))
    tw = (atw + btw / (1 + ((v - ctw) / dtw) ^ 2))
    ts = ats
}