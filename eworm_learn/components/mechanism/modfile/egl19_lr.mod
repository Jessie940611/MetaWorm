TITLE egl19 current for C. elegans neuron model


COMMENT
voltage-gated calcium channel

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
    SUFFIX egl19_lr
    USEION ca READ eca WRITE ica
    RANGE eca_tmp, ica_tmp
    RANGE dv, ica_di, i_di
    RANGE i
    RANGE gbegl19, minf, hinf, tm, th
    RANGE pure_i, didv
    RANGE w
}


PARAMETER {
    gbegl19 = 1 (nS/um2)
    vhm = 5.6 (mV)
    ka = 7.5 (mV)
    vhh = 24.9 (mV)
    ki = 12.0 (mV)
    vhhb = -20.5 (mV)
    kib = 8.1 (mV)
    ahinf = 1.43
    bhinf = 0.14
    chinf = 5.96
    dhinf = 0.60
    atm = 2.9 (ms)
    btm = 5.2 (mV)
    ctm = 6.0 (mV)
    dtm = 1.9 (ms)
    etm = 1.4 (mV)
    ftm = 30.0 (mV)
    gtm = 2.3 (ms)
    ath = 0.4
    bth = 44.6 (ms)
    cth = -23.0 (mV)
    dth = 5.0 (mV)
    eth = 36.4 (ms)
    fth = 28.7 (mV)
    gth = 3.7 (mV)
    hth = 43.1 (ms)
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
    ica_di = gbegl19 * m_dm * h_dh * (v + dv - eca_tmp)
    i_di = w * ica_di

    ica_tmp = gbegl19 * m * h * (v - eca_tmp)
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
    hinf = (ahinf / (1 + exp(-(v - vhh) / ki)) + bhinf) * (chinf / (1 + exp((v - vhhb) / kib)) + dhinf)
    tm = atm * exp(-((v - btm) / ctm) ^ 2) + dtm * exp(-((v - etm) / ftm) ^ 2) + gtm
    th = ath * ((bth / (1 + exp((v - cth) / dth))) + (eth / (1 + exp((v - fth) / gth))) + hth)
}