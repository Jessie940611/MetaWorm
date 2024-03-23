#include <stdio.h>
#include "hocdec.h"
extern int nrnmpi_myid;
extern int nrn_nobanner_;
#if defined(__cplusplus)
extern "C" {
#endif

extern void _cainternm_reg(void);
extern void _cca1_reg(void);
extern void _egl19_reg(void);
extern void _egl2_reg(void);
extern void _egl36_reg(void);
extern void _exc_syn_advance_reg(void);
extern void _gapjunction_advance_reg(void);
extern void _gapjunction_reg(void);
extern void _inh_syn_advance_reg(void);
extern void _irk_reg(void);
extern void _kcnl_reg(void);
extern void _kqt3_reg(void);
extern void _kvs1_reg(void);
extern void _nca_reg(void);
extern void _neuron_to_neuron_exc_syn_reg(void);
extern void _neuron_to_neuron_inh_syn_reg(void);
extern void _shk1_reg(void);
extern void _shl1_reg(void);
extern void _slo1_egl19_reg(void);
extern void _slo1_unc2_reg(void);
extern void _slo2_egl19_reg(void);
extern void _slo2_unc2_reg(void);
extern void _unc2_reg(void);

void modl_reg(){
  if (!nrn_nobanner_) if (nrnmpi_myid < 1) {
    fprintf(stderr, "Additional mechanisms from files\n");

    fprintf(stderr," \"modfile/cainternm.mod\"");
    fprintf(stderr," \"modfile/cca1.mod\"");
    fprintf(stderr," \"modfile/egl19.mod\"");
    fprintf(stderr," \"modfile/egl2.mod\"");
    fprintf(stderr," \"modfile/egl36.mod\"");
    fprintf(stderr," \"modfile/exc_syn_advance.mod\"");
    fprintf(stderr," \"modfile/gapjunction_advance.mod\"");
    fprintf(stderr," \"modfile/gapjunction.mod\"");
    fprintf(stderr," \"modfile/inh_syn_advance.mod\"");
    fprintf(stderr," \"modfile/irk.mod\"");
    fprintf(stderr," \"modfile/kcnl.mod\"");
    fprintf(stderr," \"modfile/kqt3.mod\"");
    fprintf(stderr," \"modfile/kvs1.mod\"");
    fprintf(stderr," \"modfile/nca.mod\"");
    fprintf(stderr," \"modfile/neuron_to_neuron_exc_syn.mod\"");
    fprintf(stderr," \"modfile/neuron_to_neuron_inh_syn.mod\"");
    fprintf(stderr," \"modfile/shk1.mod\"");
    fprintf(stderr," \"modfile/shl1.mod\"");
    fprintf(stderr," \"modfile/slo1_egl19.mod\"");
    fprintf(stderr," \"modfile/slo1_unc2.mod\"");
    fprintf(stderr," \"modfile/slo2_egl19.mod\"");
    fprintf(stderr," \"modfile/slo2_unc2.mod\"");
    fprintf(stderr," \"modfile/unc2.mod\"");
    fprintf(stderr, "\n");
  }
  _cainternm_reg();
  _cca1_reg();
  _egl19_reg();
  _egl2_reg();
  _egl36_reg();
  _exc_syn_advance_reg();
  _gapjunction_advance_reg();
  _gapjunction_reg();
  _inh_syn_advance_reg();
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

#if defined(__cplusplus)
}
#endif
