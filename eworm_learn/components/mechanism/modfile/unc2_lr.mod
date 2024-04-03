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
    SUFFIX unc2_lr
    USEION ca READ eca WRITE ica
    RANGE eca_tmp, ica_tmp
    RANGE dv, ica_di, i_di
    RANGE i
    RANGE gbunc2, minf, hinf, tm, th
    RANGE pure_i, didv
    RANGE w
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
    eca_tmp = 60. (mV)
    w = 1.
    dv = 1e-3 (mV)
}


ASSIGNED {
    v (mV)
    i (pA/um2)
    eca (mV)
    ica (pA/um2)
    ica_tmp (pA/um2)
    minf
    hinf
    tm (ms)
    th (ms)
    ica_di i_di
    pure_i
    didv
}


STATE {
    m
    h
    m_dm
    h_dh
}


BREAKPOINT {
    SOLVE states METHOD cnexp
    ica_di = gbunc2 * m_dm * h_dh * (v + dv - eca_tmp)
    i_di = w * ica_di

    ica_tmp = gbunc2 * m * h * (v - eca_tmp)
    pure_i = ica_tmp
    i = w * pure_i

    ica = i
    
    didv = -(i_di - i) / dv
}


INITIAL {
    setparames(v)
    m_dm = minf
    h_dh = hinf
    m = minf
    h = hinf
}


DERIVATIVE states {
    m_dm = m
    h_dh = h
    setparames(v + dv)
    m_dm' = (minf - m_dm) / tm
    h_dh' = (hinf - h_dh) / th
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