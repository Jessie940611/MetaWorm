TITLE slo2 current for C. elegans neuron model (suppose to interact with egl19)


COMMENT
channel type: calcium-related potassium channel

slo2-egl19 1:1 stoichiometry

model and parameters from:
Nicoletti, M., et al. (2019). "Biophysical modeling of C. elegans neurons: Single ion currents and whole-cell dynamics of AWCon and RMD." PLoS One 14(7): e0218738.
ENDCOMMENT


UNITS {
    (nS) = (nanosiemens)
    (mV) = (millivolt)
    (pA) = (picoamp)
    (um) = (micron)
    (molar) = (1/liter)  : moles do not appear in units
    (uM) = (micromolar)
}


NEURON {
    SUFFIX slo2_egl19_lr
    USEION k READ ek WRITE ik
    RANGE ek_tmp, ik_tmp
    RANGE dv, ik_di, i_di
    RANGE i
    RANGE gbslo2, minf, tm, mcavinf, tmcav, hcavinf, thcav
    RANGE pure_i, didv
    RANGE w
}


PARAMETER {
    : slo2
    gbslo2 = 1 (nS/um2)
    wyx = 0.019 (1/mV)
    wxy = -0.024 (1/mV)
    wom = 0.90 (1/ms)
    wop = 0.027 (1/ms)
    kxy = 93.45 (uM/um2)
    nxy = 1.84
    kyx = 3294.55 (uM/um2)
    nyx = 0.00001
    canci = 0.05 (uM/um2) : calcium concentration When the calcium channel is closed

    : egl19
    vhm = 5.6 (mV)
    ka = 7.5 (mV)
    vhh = 24.9 (mV)
    ki = 12.0 (mV)
    vhhb = -20.5 (mV)
    kib = 8.1 (mV)
    ahinf = 1.43
    bhinf = 0.14
    chinf = 5.96
    dhinf = 0.60
    atm = 2.9 (ms)
    btm = 5.2 (mV)
    ctm = 6.0 (mV)
    dtm = 1.9 (ms)
    etm = 1.4 (mV)
    ftm = 30.0 (mV)
    gtm = 2.3 (ms)
    ath = 0.4
    bth = 44.6 (ms)
    cth = -23.0 (mV)
    dth = 5.0 (mV)
    eth = 36.4 (ms)
    fth = 28.7 (mV)
    gth = 3.7 (mV)
    hth = 43.1 (ms)

    : cain
    FARADAY = 96485 (coul) : moles do not appear in units
	gsc = 0.04 (nS)   : single channel conductance, assumed equal to 40 pS for both L-type and P/Qtype calcium channels
    r = 0.013 (um)       : radius of the nanodomain
    dca = 250 (um2/s) : calcium diffusion coefficient
    kb = 500 (1/uM/s) : intracellular buffer rate constant
    btot = 30 (uM)    : total intracellular buffer concentration

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
    mcavinf
    tmcav (ms)
    hcavinf
    thcav (ms)
    ik_di i_di
    pure_i
    didv
}


STATE {
    m
    hcav
    mcav
    m_dm
    hcav_dhcav
    mcav_dmcav
}


BREAKPOINT {
    SOLVE states METHOD cnexp
    ik_di = gbslo2 * m_dm * hcav_dhcav * (v + dv - ek_tmp)
    i_di = w * ik_di

    ik_tmp = gbslo2 * m * hcav * (v - ek_tmp)
    pure_i = ik_tmp
    i = w * pure_i

    ik = i
    
    didv = -(i_di - i) / dv
}


INITIAL {
    setparames(v)
    hcav_dhcav = hcavinf
    mcav_dmcav = mcavinf
    m_dm = minf
    hcav = hcavinf
    mcav = mcavinf
    m = minf
}


DERIVATIVE states {
    mcav_dmcav = mcav
    hcav_dhcav = hcav
    m_dm = m
    setparames(v + dv)
    mcav_dmcav' = (mcavinf - mcav_dmcav) / tmcav
    hcav_dhcav' = (hcavinf - hcav_dhcav) / thcav
    m_dm' = (minf - m_dm) / tm
    setparames(v)
    mcav' = (mcavinf - mcav) / tmcav
    hcav' = (hcavinf - hcav) / thcav
    m' = (minf - m) / tm
}


PROCEDURE setparames(v (mV)) {
    LOCAL alpha, beta, wm, wp, fm, fp, kom, kop, kcm, cain
    : egl19 model
    mcavinf = 1 / (1 + exp(-(v - vhm) / ka))
    hcavinf = (ahinf / (1 + exp(-(v - vhh) / ki)) + bhinf) * (chinf / (1 + exp((v - vhhb) / kib)) + dhinf)
    tmcav = atm * exp(-((v - btm) / ctm) ^ 2) + dtm * exp(-((v - etm) / ftm) ^ 2) + gtm
    thcav = ath * ((bth / (1 + exp((v - cth) / dth))) + (eth / (1 + exp((v - fth) / gth))) + hth)
    : cain
    if (v < 60) {
        cain = -gsc * (v - 60) * 10^9 / (8 * 3.1415926 * r * dca * FARADAY) * exp (-r / sqrt(dca / (kb * btot)))
    } else {
        cain = 0.0001
    }
    : slo2-egl19 model
    alpha = mcavinf / tmcav
    beta = 1 / tmcav - alpha
    wm = wom * exp(-wyx * v)
    wp = wop * exp(-wxy * v)
    fm = 1 / (1 + (cain / kyx) ^ nyx)
    fp = 1 / (1 + (kxy / cain) ^ nxy)
    kom = wm * fm
    kop = wp * fp
    kcm = wm * 1 / (1 + (canci / kyx) ^ nyx)
    minf = mcav * kop * (alpha + beta + kcm) / ((kop + kom) * (kcm + alpha) + beta * kcm)
    tm = (alpha + beta + kcm) / ((kop + kom) * (kcm + alpha) + beta * kcm)
}