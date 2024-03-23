TITLE the nanoscale internal calcium concentration

COMMENT
Internal calcium concentration calculated in response to a calcium current.

model and parameters from:
Nicoletti, M., et al. (2019). "Biophysical modeling of C. elegans neurons: Single ion currents and whole-cell dynamics of AWCon and RMD." PLoS One 14(7): e0218738.
ENDCOMMENT


UNITS {
	(molar) = (1/liter)			: moles do not appear in units
    (uM)    = (micromolar)
	(um)	= (micron)
	(mA)	= (milliamp)
    (pS)    = (picosiemens)
}


NEURON {
	SUFFIX cainternn
	USEION ca READ ica, cai WRITE cai
	RANGE vcell, gbcav
}


CONSTANT {
	FARADAY = 96489		(coul)		: moles do not appear in units
}


PARAMETER {
	vcell = 1 (um3)
	gbcav = 1 (nS/um2)
	:gsc = 40 (pS)     : single channel conductance, assumed equal to 40 pS for both L-type and P/Qtype calcium channels
    r = 13 (nm)       : radius of the nanodomain
    dca = 250 (um2/s) : calcium diffusion coefficient 
    kb = 500 (1/uM/s) : intracellular buffer rate constant
    btot = 30 (uM)    : total intracellular buffer concentration
}


ASSIGNED {
	v  (mV)
	ica (pA/um2)
	cai (uM/um2)
}


BREAKPOINT {
	ica = gbcav * (v - 60): ena=60 mV
	cai = -ica * 10^12 / (8 * 3.1415926 * r * dca * FARADAY) * exp (-r * 10^(-3) / sqrt(dca / (kb * btot)))
}