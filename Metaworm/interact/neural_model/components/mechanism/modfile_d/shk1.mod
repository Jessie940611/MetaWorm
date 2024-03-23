TITLE shk1 current for C. elegans neuron model


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
    SUFFIX shk1
    USEION k READ ek WRITE ik
    RANGE gbshk1
}


PARAMETER {
    gbshk1 = 1 (nS/um2)
    vhm = 20.4 (mV)
    ka = 7.7 (mV)
    vhh = -7.0 (mV)
    ki = 5.8 (mV)
    atm = 26.6 (ms)
    btm = -33.7 (mV)
    ctm = 15.8 (mV)
    dtm = -33.7 (mV)
    etm = 15.4 (mV)
    ftm = 2.0 (ms)
    ath = 1400 (ms)
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
    :ik = gbshk1 * m * h * (v - ek)
    ik = gbshk1 * m * h * (v + 80) : ek=-80
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
    tm = atm / (exp(- (v - btm) / ctm) + exp((v - dtm) / etm)) + ftm
    th = ath
} 