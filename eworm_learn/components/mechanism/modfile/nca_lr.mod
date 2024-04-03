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
    SUFFIX nca_lr
    USEION na READ ena WRITE ina
    RANGE ena_tmp, ina_tmp
    RANGE i     : NONSPECIFIC_CURRENT i
    RANGE gbnca
    RANGE pure_i, didv
    RANGE w
}


PARAMETER {
    gbnca = 1 (nS/um2)
    ena_tmp = 30. (mV)
    w = 1.
}


ASSIGNED {
    v (mV)
    i (pA/um2)
    ena (mV)
    ina (pA/um2)
    ina_tmp (pA/um2)
    pure_i
    didv
}


BREAKPOINT {
    ina_tmp = gbnca * (v - ena_tmp)
    pure_i = ina_tmp
    i = w * pure_i

    ina = i
    
    didv = - w * gbnca
}