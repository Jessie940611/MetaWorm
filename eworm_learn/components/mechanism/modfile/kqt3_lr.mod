TITLE kqt3 current for C. elegans neuron model


COMMENT
voltage-gated potassium channel
ENDCOMMENT


UNITS {
    (nS) = (nanosiemens)
    (mV) = (millivolt)
    (pA) = (picoamp)
    (um) = (micron)
}


NEURON {
    SUFFIX kqt3_lr
    USEION k READ ek WRITE ik
    RANGE ek_tmp, ik_tmp
    RANGE dv, ik_di, i_di
    RANGE i     : NONSPECIFIC_CURRENT i
    RANGE gbkqt3, minf, tmf, tms, winf, sinf, tw, ts
    RANGE pure_i, didv
    RANGE w
}


PARAMETER {
    gbkqt3 = 1 (nS/um2)
    vhm = -12.6726 (mV)
    ka = 15.8008 (mV)
    vhw = -1.084 (mV)
    kiw = 28.78 (mV)
    aw = 0.49
    bw = 0.51
    vhs = -45.3 (mV)
    kis = 12.3 (mV)
    as = 0.34
    bs = 0.66
    atmf = 395.3 (ms)
    btmf = 38.1 (mV)
    ctmf = 33.59 (mV)
    atms = 5503.0 (ms)
    btms = -5345.4 (ms)
    ctms = 0.02827 (1/mV)
    dtms = -23.9 (mV)
    etms = -4590.6 (ms)
    ftms = 0.0357 (1/mV)
    gtms = 14.15 (mV)
    atw = 0.544 (ms)
    btw = 29.2 (ms)
    ctw = -48.09 (mV)
    dtw = 48.83 (mV)
    ats = 500e3 (ms)
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
    tmf (ms)
    tms (ms)
    winf
    sinf
    tw (ms)
    ts (ms)
    ik_di i_di
    pure_i
    didv
}


STATE {
    mf
    ms
    ww
    s
    mf_dmf
    ms_dms
    ww_dww
    s_ds
}


BREAKPOINT {
    SOLVE states METHOD cnexp
    ik_di = gbkqt3 * (0.3 * mf_dmf + 0.7 * ms_dms) * ww_dww * s_ds * (v + dv - ek)
    i_di = w * ik_di

    ik_tmp = gbkqt3 * (0.3 * mf + 0.7 * ms) * ww * s * (v - ek)
    pure_i = ik_tmp
    i = w * pure_i

    ik = i
    
    didv = -(i_di - i) / dv
}


INITIAL {
    setparames(v)
    mf_dmf = 0
    ms_dms = 0
    ww_dww = winf
    s_ds = sinf
    mf = 0
    ms = 0
    ww = winf
    s = sinf
}


DERIVATIVE states {
    mf_dmf = mf
    ms_dms = ms
    ww_dww = ww
    s_ds = s
    setparames(v + dv)
    mf_dmf' = (minf - mf_dmf) / tmf
    ms_dms' = (minf - ms_dms) / tms
    ww_dww' = (winf - ww_dww) / tw
    s_ds' = (sinf - s_ds) / ts
    setparames(v)
    mf' = (minf - mf) / tmf
    ms' = (minf - ms) / tms
    ww' = (winf - ww) / tw
    s' = (sinf - s) / ts
}


PROCEDURE setparames(v (mV)) {
    minf = 1 / (1 + exp(-(v - vhm) / ka))
    tmf = atmf / (1 + ((v + btmf) / ctmf) ^ 2)
    tms = atms + btms / (1 + 10 ^ (-ctms * (dtms - v))) + etms / (1 + 10 ^ (-ftms * (gtms + v)))
    winf = aw + bw / (1 + exp((v - vhw) / kiw))
    sinf = as + bs / (1 + exp((v - vhs) / kis))
    tw = (atw + btw / (1 + ((v - ctw) / dtw) ^ 2))
    ts = ats
}