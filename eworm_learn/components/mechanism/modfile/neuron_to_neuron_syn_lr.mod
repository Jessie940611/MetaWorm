COMMENT
Based on NMODL code for neuron_to_neuron_exc_syn

Code and comments have been normalized--that is,
they are now of the usual form employed in 
NMODL-specified synaptic mechanisms.

Oddities that save neither storage nor time, 
or preclude use with adaptive integration, 
or deliberately introduce errors in an attempt
to save a few CPU cycles, have been removed.

Unused variables and parameters have also been removed.

vpre is now a POINTER.

In the INITIAL block, the completely unnecessary repeat
execution of procedure rates() has been removed, 
and the state variable s is now set to the correct 
initial value.

The value of the current i is now calculated in the
BREAKPOINT block, instead of being calculated as a 
side-effect of calling rates().
ENDCOMMENT

TITLE excitatory to excitatory synapse

NEURON {
    POINT_PROCESS neuron_to_neuron_syn_lr
    POINTER vpre
    NONSPECIFIC_CURRENT i
    RANGE w, g
    RANGE pure_i, didv, dsdvpre, didvpre, dvpre
    RANGE delta, k, Vth, erev
}

UNITS {
    (nA) = (nanoamp)
    (uA) = (microamp)
    (mA) = (milliamp)
    (A) = (amp)
    (mV) = (millivolt)
    (mS) = (millisiemens)
    (uS) = (microsiemens)
    (molar) = (1/liter)
    (kHz) = (kilohertz)
    (mM) = (millimolar)
    (um) = (micrometer)
    (umol) = (micromole)
    (S) = (siemens)
}

PARAMETER {
    w = 1.
    g = 4.9 (uS)
    delta = 5 (mV)
    k = 0.5 (kHz)
    Vth = -20 (mV) :0
    erev = 30 (mV) : -10
    dvpre = 1e-3 (mV)
}

ASSIGNED {
    v (mV)
    i (nA)
    vpre (mV)
    inf
    tau (ms)
    pure_i
    didv
    dsdvpre
    didvpre
}

STATE {
    s
    s_ds
}

INITIAL {
    rates(vpre)
    s_ds = inf
    s = inf : NTC original code did not properly initialize s
    if (w > 0) {
        g = 4.9
        k = 0.5
        erev = 30
    }
    else {
        g = 2.
        k = 0.015000001
        erev = -70
    }
}

BREAKPOINT {
    SOLVE states METHOD cnexp
    pure_i = g * s * (v - erev)   : in diff equations should be sign reversed
    i = fabs(w) * pure_i
    didv = -fabs(w) * g * s
    dsdvpre = (s_ds - s) / dvpre
    didvpre = fabs(w) * g * (erev - v) * dsdvpre
}

DERIVATIVE states {
    s_ds = s
    rates(vpre + dvpre)
    s_ds' = (inf - s_ds)/tau
    rates(vpre)
    s' = (inf - s)/tau
}

PROCEDURE rates(v (mV)) {
    inf = 1/(1 + exp((Vth - v)/delta))
    tau = (1 - inf)/k
}
