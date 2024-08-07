TITLE Gap junction current for parallel computation

COMMENT
Implemented by Erin Munro, 
copied largely from:
The NEURON Book by Nicholas T. Carnevale, Michael L. Hines
online at Google books
ENDCOMMENT


UNITS {
    (mV) = (millivolt)
    (nA) = (nanoamp)
    (uS) = (microsiemens)
} 


NEURON {
    POINT_PROCESS gapjunction_lr
    POINTER vpre
    RANGE w, g
    RANGE pure_i, didv, didvpre
    NONSPECIFIC_CURRENT i
}


PARAMETER {
    g = 1. (uS)
    w = 0.
} 


ASSIGNED {
    v (mV)
    i (nA)
    vpre (mV)
    pure_i
    didv
    didvpre
}


BREAKPOINT {
    pure_i = g * (v - vpre)   : in diff equations should be sign reversed
    i = w * pure_i
    didv = -w * g
    didvpre = w * g
} 
