TITLE cca1 current for C. elegans neuron model


COMMENT
channel type: voltage-gated calcium channel

model and parameters from:
Nicoletti, M., et al. (2019). "Biophysical modeling of C. elegans neurons: Single ion currents and whole-cell dynamics of AWCon and RMD." PLoS One 14(7): e0218738.
ENDCOMMENT


UNITS {
    (nS) = (nanosiemens)
    (mV) = (millivolt)
    (pA) = (picoamp)
    (um) = (micron)
}


NEURON {
    SUFFIX cca1
    USEION ca READ eca WRITE ica
    RANGE gbcca1, minf, hinf, tm, th, m, h
}


PARAMETER {
    gbcca1 = 1 (nS/um2)
    vhm = -43.32 (mV)
    ka = 7.6 (mV)
    vhh = -58.0 (mV)
    ki = 7.0 (mV)
    atm = 40.0 (ms)
    btm = -62.5 (mV)
    ctm = -12.6 (mV)
    dtm = 0.7 (ms)
    ath = 280 (ms)
    bth = -60.7 (mV)
    cth = 8.5 (mV)
    dth = 19.8 (ms)
}


ASSIGNED {
    v (mV)
    eca (mV)
    ica (pA/um2)
    minf
    hinf
    tm (ms)
    th (ms)
}


STATE {
    m
    h
}


BREAKPOINT {
    SOLVE states METHOD cnexp
    :ica = gbcca1 * m * m * h * (v - eca)
    ica = gbcca1 * m * m * h * (v - 60) : eca=60 mV
}


INITIAL {
    setparames(v)
    m = minf
    h = hinf
}


DERIVATIVE states {
    setparames(v)
    m' = (minf - m) / tm
    h' = (hinf - h) / th
}


PROCEDURE setparames(v (mV)) {
    minf = 1 / (1 + exp(-(v - vhm) / ka))
    hinf = 1 / (1 + exp((v - vhh) / ki))
    tm = atm / (1 + exp(-(v - btm) / ctm)) + dtm
    th = ath / (1 + exp((v - bth) / cth)) + dth
} 