TITLE nca current for C. elegans neuron model


COMMENT
channel type: sodium passive currents

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
    SUFFIX nca
    USEION na READ ena WRITE ina
    RANGE gbnca
}


PARAMETER {
    gbnca = 1 (nS/um2)
}


ASSIGNED {
    v (mV)
    ena (mV)
    ina (pA/um2)
}


BREAKPOINT {
    ina = gbnca * (v - 30)  :ena=30mV
}