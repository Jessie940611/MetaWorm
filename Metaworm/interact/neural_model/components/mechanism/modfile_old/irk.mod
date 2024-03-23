TITLE irk current for C. elegans neuron model


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
    SUFFIX irk
    USEION k READ ek WRITE ik
    RANGE gbirk, minf, tm, m
}


PARAMETER {
    gbirk = 1 (nS/um2)
    vhm = -82 (mV)
    ka = 13 (mV)
    atm = 17.1 (ms)
    btm = -17.8 (mV)
    ctm = 20.3 (mV)
    dtm = -43.4 (mV)
    etm = 11.2 (mV)
    ftm = 3.8 (ms)
}


ASSIGNED {
    v (mV)
    ek (mV)
    ik (pA/um2)
    minf
    tm (ms)
}


STATE {
    m
}


BREAKPOINT {
    SOLVE states METHOD cnexp
    :ik = gbirk * m * (v - ek)
    ik = gbirk * m * (v + 80) : ek = -80
}


INITIAL {
    setparames(v)
    m = minf
}


DERIVATIVE states {
    setparames(v)
    m' = (minf - m) / tm
}


PROCEDURE setparames(v (mV)) {
    minf = 1 / (1 + exp((v - vhm) / ka))
    tm = atm / (exp(-(v - btm) / ctm) + exp((v - dtm) / etm)) + ftm
} 