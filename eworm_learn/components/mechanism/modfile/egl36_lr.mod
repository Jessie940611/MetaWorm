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
    SUFFIX egl36_lr
    USEION k READ ek WRITE ik
    RANGE ek_tmp, ik_tmp
    RANGE dv, ik_di, i_di
    RANGE i
    RANGE gbegl36, mfinf, mminf, msinf, tmf, tmm, tms
    RANGE pure_i, didv
    RANGE w
}


PARAMETER {
    gbegl36 = 1 (nS/um2)
    vhm = 63.0 (mV)
    ka = 28.5 (mV)
    atms = 355.0 (ms)
    atmm = 63.0 (ms)
    atmf = 13.0 (ms)
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
    mfinf
    mminf
    msinf
    tmf (ms)
    tmm (ms)
    tms (ms)
    ik_di i_di
    pure_i
    didv
}


STATE {
    mf
    mm
    ms
    mf_dmf
    mm_dmm
    ms_dms
}


BREAKPOINT {
    SOLVE states METHOD cnexp
    ik_di = gbegl36 * (0.33 * mf_dmf + 0.36 * mm_dmm + 0.39 * ms_dms) * (v + dv - ek_tmp)
    i_di = w * ik_di

    ik_tmp = gbegl36 * (0.33 * mf + 0.36 * mm + 0.39 * ms) * (v - ek_tmp)
    pure_i = ik_tmp
    i = w * pure_i

    ik = i
    
    didv = -(i_di - i) / dv
}


INITIAL {
    setparames(v)
    mf_dmf = mfinf
    mm_dmm = mminf
    ms_dms = msinf
    mf = mfinf
    mm = mminf
    ms = msinf
}


DERIVATIVE states {
    mf_dmf = mf
    mm_dmm = mm
    ms_dms = ms
    setparames(v + dv)
    mf_dmf' = (mfinf - mf_dmf) / tmf
    mm_dmm' = (mminf - mm_dmm) / tmm
    ms_dms' = (msinf - ms_dms) / tms
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