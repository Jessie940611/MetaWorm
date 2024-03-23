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
    SUFFIX egl2
    USEION k READ ek WRITE ik
    RANGE gbegl2, minf, tm, m
}


PARAMETER {
    gbegl2 = 1 (nS/um2)
    vhm = -6.9 (mV)
    ka = 14.9 (mV)
    atm = 1845.8 (ms)
    btm = -122.6 (mV)
    ctm = 13.8 (mV)
    dtm = 1517.74 (ms)
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
    :ik = gbegl2 * m * (v - ek)
    ik = gbegl2 * m * (v + 80) : ek = -80
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
    minf = 1 / (1 + exp(-(v - vhm) / ka))
    tm = atm / (1 + exp((v - btm) / ctm)) + dtm
}