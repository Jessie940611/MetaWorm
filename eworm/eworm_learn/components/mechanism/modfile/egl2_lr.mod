TITLE egl2 current for C. elegans neuron model


COMMENT
channel type: voltage-gated potassium channel

patch clamp data from:
Weinshenker, D. , et al. "Block of an ether-a-go-go-like K(+) channel by imipramine rescues egl-2 excitation defects in Caenorhabditis elegans. " Journal of Neuroscience the Official Journal of the Society for Neuroscience 19.22(1999):9831.
Figure 3a upper left

ENDCOMMENT


UNITS {
    (nS) = (nanosiemens)
    (mV) = (millivolt)
    (pA) = (picoamp)
    (um) = (micron)
}


NEURON {
    SUFFIX egl2_lr
    USEION k READ ek WRITE ik
    RANGE ek_tmp, ik_tmp
    RANGE dv, ik_di, i_di
    RANGE i
    RANGE gbegl2, minf, tm
    RANGE pure_i, didv
    RANGE w
}


PARAMETER {
    gbegl2 = 1 (nS/um2)
    vhm = -6.9 (mV)
    ka = 14.9 (mV)
    atm = 1845.8 (ms)
    btm = -122.6 (mV)
    ctm = 13.8 (mV)
    dtm = 1517.74 (ms)
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
    ik_di = gbegl2 * m_dm * (v + dv - ek_tmp)
    i_di = w * ik_di

    ik_tmp = gbegl2 * m * (v - ek_tmp)
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
    minf = 1 / (1 + exp(-(v - vhm) / ka))
    tm = atm / (1 + exp((v - btm) / ctm)) + dtm
}