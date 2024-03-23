#include <stdio.h>
#include "hocdec.h"
#define IMPORT extern __declspec(dllimport)
IMPORT int nrnmpi_myid, nrn_nobanner_;

extern void _cainternm_reg();
extern void _cainternn_reg();
extern void _cca1_reg();
extern void _egl19_reg();
extern void _egl2_reg();
extern void _egl36_reg();
extern void _gapjunction_reg();
extern void _irk_reg();
extern void _kcnl_reg();
extern void _kqt3_reg();
extern void _kvs1_reg();
extern void _nca_reg();
extern void _neuron_to_neuron_exc_syn_reg();
extern void _neuron_to_neuron_inh_syn_reg();
extern void _shk1_reg();
extern void _shl1_reg();
extern void _slo1_egl19_reg();
extern void _slo1_unc2_reg();
extern void _slo2_egl19_reg();
extern void _slo2_unc2_reg();
extern void _unc2_reg();

void modl_reg(){
	//nrn_mswindll_stdio(stdin, stdout, stderr);
    if (!nrn_nobanner_) if (nrnmpi_myid < 1) {
	fprintf(stderr, "Additional mechanisms from files\n");

fprintf(stderr," cainternm.mod");
fprintf(stderr," cainternn.mod");
fprintf(stderr," cca1.mod");
fprintf(stderr," egl19.mod");
fprintf(stderr," egl2.mod");
fprintf(stderr," egl36.mod");
fprintf(stderr," gapjunction.mod");
fprintf(stderr," irk.mod");
fprintf(stderr," kcnl.mod");
fprintf(stderr," kqt3.mod");
fprintf(stderr," kvs1.mod");
fprintf(stderr," nca.mod");
fprintf(stderr," neuron_to_neuron_exc_syn.mod");
fprintf(stderr," neuron_to_neuron_inh_syn.mod");
fprintf(stderr," shk1.mod");
fprintf(stderr," shl1.mod");
fprintf(stderr," slo1_egl19.mod");
fprintf(stderr," slo1_unc2.mod");
fprintf(stderr," slo2_egl19.mod");
fprintf(stderr," slo2_unc2.mod");
fprintf(stderr," unc2.mod");
fprintf(stderr, "\n");
    }
_cainternm_reg();
_cainternn_reg();
_cca1_reg();
_egl19_reg();
_egl2_reg();
_egl36_reg();
_gapjunction_reg();
_irk_reg();
_kcnl_reg();
_kqt3_reg();
_kvs1_reg();
_nca_reg();
_neuron_to_neuron_exc_syn_reg();
_neuron_to_neuron_inh_syn_reg();
_shk1_reg();
_shl1_reg();
_slo1_egl19_reg();
_slo1_unc2_reg();
_slo2_egl19_reg();
_slo2_unc2_reg();
_unc2_reg();
}
