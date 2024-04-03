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
    SUFFIX kvs1_lr
    USEION k READ ek WRITE ik
    RANGE ek_tmp, ik_tmp
    RANGE dv, ik_di, i_di
    RANGE i     : NONSPECIFIC_CURRENT i
    RANGE gbkvs1, minf, hinf, tm, th
    RANGE pure_i, didv
    RANGE w
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
    ek_tmp = -80. (mV)
    w = 1.
    dv = 1e-3 (mV)
}


ASSIGNED {
    v (mV)
    i (pA/um2)
    ek (mV)
    ik (pA/um2)
    ik_tmp (pA/um2)
    minf
    hinf
    tm (ms)
    th (ms)
    ik_di i_di
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
    ik_di = gbkvs1 * m_dm * h_dh * (v + dv - ek_tmp)
    i_di = w * ik_di

    ik_tmp = gbkvs1 * m * h * (v - ek_tmp)
    pure_i = ik_tmp
    i = w * pure_i

    ik = i
    
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
    tm = atm / (1 + exp((v - btm) / ctm)) + dtm
    th = ath / (1 + exp((v - bth) / cth)) + dth
} 