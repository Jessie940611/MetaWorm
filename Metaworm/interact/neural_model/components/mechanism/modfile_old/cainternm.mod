TITLE the microscale internal calcium concentration

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
	SUFFIX cainternm
	USEION ca READ ica, cai WRITE cai
	RANGE vcell
}


CONSTANT {
	FARADAY = 96489		(coul)		: moles do not appear in units
}


PARAMETER {
	vcell = 1 (um3)
	f = 0.001         : free intracellular calcium fraction
	tca = 50 (ms)     : calcium removal time constant
	caeq = 0.05 (uM/um2)  : the baseline calcium concentration
}


ASSIGNED {
	v  (mV)
	ica (pA/um2)
	alpha
}


STATE {
    cai  (uM/um2)
}


INITIAL {
    setparames(v)
}


BREAKPOINT {
	SOLVE states METHOD derivimplicit
}


DERIVATIVE states {
    setparames(v)
    cai' = alpha
}


PROCEDURE setparames(v (mV)) {
	if (v <= 60.) { : eca=60 mV
		alpha = (-f * ica / (2 * FARADAY * vcell * (10^(-6)))) - ((cai - caeq) / tca)
	}
	else {
		alpha = -(cai - caeq) / tca
	}
}