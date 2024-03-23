TITLE kvs1 current for C. elegans neuron model


COMMENT
channel type: voltage-gated potassium channel

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
    SUFFIX kvs1
    USEION k READ ek WRITE ik
    RANGE gbkvs1, minf, hinf, tm, th, m, h
}


PARAMETER {
    gbkvs1 = 1 (nS/um2)
    vhm = 57.1 (mV)
    ka = 25.0 (mV)
    vhh = 47.3 (mV)
    ki = 11.1 (mV)
    atm = 30.0 (ms)
    btm = 18.1 (mV)
    ctm = 20.0 (mV)
    dtm = 1.0 (ms)
    ath = 88.5 (ms)
    bth = 50.0 (mV)
    cth = 15.0 (mV)
    dth = 53.4 (ms)
}


ASSIGNED {
    v (mV)
    ek (mV)
    ik (pA/um2)
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
    :ik = gbkvs1 * m * h * (v - ek)
    ik = gbkvs1 * m * h * (v + 80) : ek = -80
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
    tm = atm / (1 + exp((v - btm) / ctm)) + dtm
    th = ath / (1 + exp((v - bth) / cth)) + dth
} 