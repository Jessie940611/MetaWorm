TITLE kcnl current for C. elegans neuron model


COMMENT
channel type: calcium-related potassium channel

regulated soly by calcium concentration in microdomain surrounding the channel

model and parameters from:
Nicoletti, M., et al. (2019). "Biophysical modeling of C. elegans neurons: Single ion currents and whole-cell dynamics of AWCon and RMD." PLoS One 14(7): e0218738.
ENDCOMMENT


UNITS {
    (nS) = (nanosiemens)
    (mV) = (millivolt)
    (pA) = (picoamp)
    (um) = (micron)
    (molar) = (1/liter)  : moles do not appear in units
    (uM)    = (micromolar)
}


NEURON {
    SUFFIX kcnl_lr
    USEION k READ ek WRITE ik
    USEION ca READ cai
    RANGE dv, ik_di, i_di
    RANGE i
    RANGE gbkcnl
    RANGE pure_i, didv
    RANGE w
}


PARAMETER {
    : kcnl
    gbkcnl = 1 (nS/um2)
    kca = 0.33 (uM/um2)
    a = 6.3 (ms)
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
    cai (uM/um2)
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
    ik_di = gbkcnl * m_dm * (v + dv - ek_tmp)
    i_di = w * ik_di

    ik_tmp = gbkcnl * m * (v - ek_tmp)
    pure_i = ik_tmp
    i = w * pure_i

    ik = i

    didv = -(i_di - i) / dv
}


INITIAL {
    setparames(v)
    m = minf
    m_dm = minf
}


DERIVATIVE states {
    m_dm = m
    setparames(v + dv)
    m_dm' = (minf - m_dm) / tm
    setparames(v)
    m' = (minf - m) / tm
}


PROCEDURE setparames(v (mV)) {
    minf = cai / (kca + cai)
    tm = a
}