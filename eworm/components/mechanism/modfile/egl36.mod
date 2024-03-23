TITLE egl36 current for C. elegans neuron model


COMMENT
Channel type: voltage-gated potassium channel.

Model, parameters and patch clamp data from:
Johnstone, D. B. , et al. "Behavioral Defects in C. elegans egl-36 Mutants Result from Potassium Channels Shifted in Voltage-Dependence of Activation." Neuron 19.1(1997):151-64.

patch clamp data in Figure 2A left upper
https://www.cell.com/action/showPdf?pii=S0896-6273%2800%2980355-4


ENDCOMMENT


UNITS {
    (nS) = (nanosiemens)
    (mV) = (millivolt)
    (pA) = (picoamp)
    (um) = (micron)
}


NEURON {
    SUFFIX egl36
    USEION k READ ek WRITE ik
    RANGE gbegl36, mfinf, mminf, msinf, tmf, tmm, tms, mf, mm, ms
}


PARAMETER {
    gbegl36 = 1 (nS/um2)
    vhm = 63.0 (mV)
    ka = 28.5 (mV)
    atms = 355.0 (ms)
    atmm = 63.0 (ms)
    atmf = 13.0 (ms)
}


ASSIGNED {
    v (mV)
    ek (mV)
    ik (pA/um2)
    mfinf
    mminf
    msinf
    tmf (ms)
    tmm (ms)
    tms (ms)
}


STATE {
    mf
    mm
    ms
}


BREAKPOINT {
    SOLVE states METHOD cnexp
    :ik = gbegl36 * (0.33 * mf + 0.36 * mm + 0.39 * ms) * (v - ek)
    ik = gbegl36 * (0.33 * mf + 0.36 * mm + 0.39 * ms) * (v + 80) : ek = -80
}


INITIAL {
    setparames(v)
    mf = mfinf
    mm = mminf
    ms = msinf
}


DERIVATIVE states {
    setparames(v)
    mf' = (mfinf - mf) / tmf
    mm' = (mminf - mm) / tmm
    ms' = (msinf - ms) / tms
}


PROCEDURE setparames(v (mV)) {
    mfinf = 1 / (1 + exp(-(v - vhm) / ka))
    mminf = 1 / (1 + exp(-(v - vhm) / ka))
    msinf = 1 / (1 + exp(-(v - vhm) / ka))
    tmf = atmf
    tmm = atmm
    tms = atms
} 