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
    POINT_PROCESS gapjunction
    POINTER vpre
	  RANGE weight
    RANGE i
	  NONSPECIFIC_CURRENT i
}


PARAMETER { 
	  weight = 0.0 	   (uS)
} 


ASSIGNED {
    v (mV)
    vpre (mV)
    i (nA)
}


BREAKPOINT { 
	  i = weight * (v - vpre) 
} 
