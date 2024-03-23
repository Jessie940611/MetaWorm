TITLE shl1 current for C. elegans neuron model


COMMENT
channel type: voltage-gated potassium channel

model and parameters from:
Nicoletti, M., et al. (2019). "Biophysical modeling of C. elegans neurons: Single ion currents and whole-cell dynamics of AWCon and RMD." PLoS One 14(7): e0218738.
ENDCOMMENT


UNITS {
    (nS) = (nanosiemens)
    (mV) = (millivolt)
    :(mA) = (milliamp)
    (pA) = (picoamp)
    (um) = (micron)
}


NEURON {
    SUFFIX shl1
    USEION k READ ek WRITE ik
    RANGE gbshl1, minf, hfinf, hsinf, tm, thf, ths, m, hf, hs
}


PARAMETER {
    gbshl1 = 1 (nS/um2)
    vhm = 11.2 (mV)
    ka = 14.1 (mV)
    vhh = -33.1 (mV)
    ki = 8.3 (mV)
    atm = 13.8 (ms)
    btm = -17.5 (mV)
    ctm = 12.9 (mV)
    dtm = -3.7 (mV)
    etm = 6.5 (mV)
    ftm = 1.9 (ms)
    athf = 539.2 (ms)
    bthf = -28.2 (mV)
    cthf = 4.9 (mV)
    dthf = 27.3 (ms)
    aths = 8422.0 (ms)
    bths = -37.7 (mV)
    cths = 6.4 (mV)
    dths = 118.9 (ms)
}


ASSIGNED {
    v (mV)
    ek (mV)
    ik (pA/um2) :(mA/cm2)
    minf
    hfinf
    hsinf
    tm (ms)
    thf (ms)
    ths (ms)
}


STATE {
    m
    hf
    hs
}


BREAKPOINT {
    SOLVE states METHOD cnexp
    :ik = gbshl1 * m * m * m * (0.7 * hf + 0.3 * hs) * (v - ek)
    ik = gbshl1 * m * m * m * (0.7 * hf + 0.3 * hs) * (v + 80) : ek = -80
}


INITIAL {
    setparames(v)
    m = minf
    hf = hfinf
    hs = hsinf
}


DERIVATIVE states {
    setparames(v)
    m' = (minf - m) / (tm*0.4)
    hf' = (hfinf - hf) / (thf*0.08)
    hs' = (hsinf - hs) / (ths*0.3)
}


PROCEDURE setparames(v (mV)) {
    minf = 1 / (1 + exp(-(v - vhm) / ka))
    hfinf = 1 / (1 + exp((v - vhh) / ki))
    hsinf = 1 / (1 + exp((v - vhh) / ki))
    tm = atm / (exp(-(v - btm) / ctm) + exp((v - dtm) / etm)) + ftm
    thf = athf / (1 + exp((v - bthf) / cthf)) + dthf
    ths = aths / (1 + exp((v - bths) / cths)) + dths
} 