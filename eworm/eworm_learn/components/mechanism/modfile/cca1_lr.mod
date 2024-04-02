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
    SUFFIX cca1_lr
    USEION ca READ eca WRITE ica
    RANGE eca_tmp, ica_tmp
    RANGE dv, ica_di, i_di
    RANGE i
    RANGE gbcca1, minf, hinf, tm, th
    RANGE pure_i, didv
    RANGE w
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
    ica_di = gbcca1 * m_dm * m_dm * h_dh * (v + dv - eca_tmp)
    i_di = w * ica_di

    ica_tmp = gbcca1 * m * m * h * (v - eca_tmp)
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
    tm = atm / (1 + exp(-(v - btm) / ctm)) + dtm
    th = ath / (1 + exp((v - bth) / cth)) + dth
} 