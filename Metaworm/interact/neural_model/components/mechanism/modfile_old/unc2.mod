TITLE unc2 current for C. elegans neuron model


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
    SUFFIX unc2
    USEION ca READ eca WRITE ica
    RANGE gbunc2, minf, hinf, tm, th, m, h
}


PARAMETER {
    gbunc2 = 1 (nS/um2)
    vhm = -12.2 (mV)
    ka = 4.0 (mV)
    vhh = -52.5 (mV)
    ki = 5.6 (mV)
    atm = 1.5 (ms)
    btm = -8.2 (mV)
    ctm = 9.1 (mV)
    dtm = 15.4 (mV)
    etm = 0.1 (ms)
    ath = 83.8 (ms)
    bth = 52.9 (mV)
    cth = -3.5 (mV)
    dth = 72.1 (ms)
    eth = 23.9 (mV)
    fth = -3.6 (mV)
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
    :ica = gbunc2 * m * h * (v - eca)
    ica = gbunc2 * m * h * (v - 60) :eca=60 mV
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
    tm = atm / (exp(-(v - btm) / ctm) + exp((v - btm) / dtm)) + etm
    th = ath / (1 + exp(-(v - bth) / cth)) + dth / (1 + exp((v - eth) / fth))
}