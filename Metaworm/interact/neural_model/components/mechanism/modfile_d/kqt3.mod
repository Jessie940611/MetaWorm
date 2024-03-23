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
    RANGE gbkqt3
}


PARAMETER {
    gbkqt3 = 1 (nS/um2)
    vhm = -12.8 (mV)
    ka = 15.8 (mV)
    vhw = -1.1 (mV)
    kiw = 28.8 (mV)
    aw = 0.5
    bw = 0.5
    vhs = -45.3 (mV)
    kis = 12.3 (mV)
    as = 0.3
    bs =  0.7
    atmf = 395.3 (ms)
    btmf = 38.1 (mV)
    ctmf = 33.6 (mV)
    atms = 5503.0 (ms)
    btms = -5345.4 (ms)
    ctms = 0.0283 (1/mV)
    dtms = -23.9 (mV)
    etms = -4590 (ms)
    ftms = 0.0357 (1/mV)
    gtms = 14.2 (mV)
    atw = 0.5 (ms)
    btw = 2.9 (ms)
    ctw = -48.1 (mV)
    dtw = 48.8 (mV)
    ats = 500 (ms)
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
    :ik = gbkqt3 * (0.3 * mf + 0.7 * ms) * w * s * (v - ek)
    ik = gbkqt3 * (0.3 * mf + 0.7 * ms) * (v + 80) : ek = -80
}


INITIAL {
    setparames(v)
    mf = minf
    ms = minf
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