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
    SUFFIX irk_lr
    USEION k READ ek WRITE ik
    RANGE ek_tmp, ik_tmp
    RANGE dv, ik_di, i_di
    RANGE i     : NONSPECIFIC_CURRENT i
    RANGE gbirk, minf, tm
    RANGE pure_i, didv
    RANGE w
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
    tm (ms)
    ik_di i_di
    pure_i
    didv
}


STATE {
    m
    m_dm
}


BREAKPOINT {
    SOLVE states METHOD cnexp
    ik_di = gbirk * m_dm * (v + dv - ek_tmp)
    i_di = w * ik_di

    ik_tmp = gbirk * m * (v - ek_tmp)
    pure_i = ik_tmp
    i = w * pure_i

    ik = i
    
    didv = -(i_di - i) / dv
}


INITIAL {
    setparames(v)
    m_dm = minf
    m = minf
}


DERIVATIVE states {
    m_dm = m
    setparames(v + dv)
    m_dm' = (minf - m_dm) / tm
    setparames(v)
    m' = (minf - m) / tm
}


PROCEDURE setparames(v (mV)) {
    minf = 1 / (1 + exp((v - vhm) / ka))
    tm = atm / (exp(-(v - btm) / ctm) + exp((v - dtm) / etm)) + ftm
} 