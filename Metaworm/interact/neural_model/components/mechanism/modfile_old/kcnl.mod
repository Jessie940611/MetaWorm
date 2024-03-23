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
    SUFFIX kcnl
    USEION k READ ek WRITE ik
    USEION ca READ cai
    RANGE gbkcnl
}


PARAMETER {
    : kcnl
    gbkcnl = 1 (nS/um2)
    kca = 0.33 (uM/um2)
    a = 6.3 (ms)
}


ASSIGNED {
    v (mV)
    ek (mV)
    ik (pA/um2)
    cai (uM/um2)
    minf
    tm (ms)
}


STATE {
    m
}


BREAKPOINT {
    SOLVE states METHOD cnexp
    ik = gbkcnl * m * (v + 80) : ek=-80
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
    minf = cai / (kca + cai)
    tm = a
}