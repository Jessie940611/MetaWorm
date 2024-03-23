/* Created by Language version: 7.7.0 */
/* VECTORIZED */
#define NRN_VECTORIZED 1
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "scoplib.h"
#undef PI
#define nil 0
#include "md1redef.h"
#include "section.h"
#include "nrniv_mf.h"
#include "md2redef.h"
 
#if METHOD3
extern int _method3;
#endif

#if !NRNGPU
#undef exp
#define exp hoc_Exp
extern double hoc_Exp(double);
#endif
 
#define nrn_init _nrn_init__shl1
#define _nrn_initial _nrn_initial__shl1
#define nrn_cur _nrn_cur__shl1
#define _nrn_current _nrn_current__shl1
#define nrn_jacob _nrn_jacob__shl1
#define nrn_state _nrn_state__shl1
#define _net_receive _net_receive__shl1 
#define setparames setparames__shl1 
#define states states__shl1 
 
#define _threadargscomma_ _p, _ppvar, _thread, _nt,
#define _threadargsprotocomma_ double* _p, Datum* _ppvar, Datum* _thread, NrnThread* _nt,
#define _threadargs_ _p, _ppvar, _thread, _nt
#define _threadargsproto_ double* _p, Datum* _ppvar, Datum* _thread, NrnThread* _nt
 	/*SUPPRESS 761*/
	/*SUPPRESS 762*/
	/*SUPPRESS 763*/
	/*SUPPRESS 765*/
	 extern double *getarg();
 /* Thread safe. No static _p or _ppvar. */
 
#define t _nt->_t
#define dt _nt->_dt
#define gbshl1 _p[0]
#define minf _p[1]
#define hfinf _p[2]
#define hsinf _p[3]
#define tm _p[4]
#define thf _p[5]
#define ths _p[6]
#define m _p[7]
#define hf _p[8]
#define hs _p[9]
#define ek _p[10]
#define ik _p[11]
#define Dm _p[12]
#define Dhf _p[13]
#define Dhs _p[14]
#define v _p[15]
#define _g _p[16]
#define _ion_ek	*_ppvar[0]._pval
#define _ion_ik	*_ppvar[1]._pval
#define _ion_dikdv	*_ppvar[2]._pval
 
#if MAC
#if !defined(v)
#define v _mlhv
#endif
#if !defined(h)
#define h _mlhh
#endif
#endif
 
#if defined(__cplusplus)
extern "C" {
#endif
 static int hoc_nrnpointerindex =  -1;
 static Datum* _extcall_thread;
 static Prop* _extcall_prop;
 /* external NEURON variables */
 /* declaration of user functions */
 static void _hoc_setparames(void);
 static int _mechtype;
extern void _nrn_cacheloop_reg(int, int);
extern void hoc_register_prop_size(int, int, int);
extern void hoc_register_limits(int, HocParmLimits*);
extern void hoc_register_units(int, HocParmUnits*);
extern void nrn_promote(Prop*, int, int);
extern Memb_func* memb_func;
 
#define NMODL_TEXT 1
#if NMODL_TEXT
static const char* nmodl_file_text;
static const char* nmodl_filename;
extern void hoc_reg_nmodl_text(int, const char*);
extern void hoc_reg_nmodl_filename(int, const char*);
#endif

 extern void _nrn_setdata_reg(int, void(*)(Prop*));
 static void _setdata(Prop* _prop) {
 _extcall_prop = _prop;
 }
 static void _hoc_setdata() {
 Prop *_prop, *hoc_getdata_range(int);
 _prop = hoc_getdata_range(_mechtype);
   _setdata(_prop);
 hoc_retpushx(1.);
}
 /* connect user functions to hoc names */
 static VoidFunc hoc_intfunc[] = {
 "setdata_shl1", _hoc_setdata,
 "setparames_shl1", _hoc_setparames,
 0, 0
};
 /* declare global and static user variables */
#define aths aths_shl1
 double aths = 8422;
#define athf athf_shl1
 double athf = 539.2;
#define atm atm_shl1
 double atm = 13.8;
#define bths bths_shl1
 double bths = -37.7;
#define bthf bthf_shl1
 double bthf = -28.2;
#define btm btm_shl1
 double btm = -17.5;
#define cths cths_shl1
 double cths = 6.4;
#define cthf cthf_shl1
 double cthf = 4.9;
#define ctm ctm_shl1
 double ctm = 12.9;
#define dths dths_shl1
 double dths = 118.9;
#define dthf dthf_shl1
 double dthf = 27.3;
#define dtm dtm_shl1
 double dtm = -3.7;
#define etm etm_shl1
 double etm = 6.5;
#define ftm ftm_shl1
 double ftm = 1.9;
#define ki ki_shl1
 double ki = 8.3;
#define ka ka_shl1
 double ka = 14.1;
#define vhh vhh_shl1
 double vhh = -33.1;
#define vhm vhm_shl1
 double vhm = 11.2;
 /* some parameters have upper and lower limits */
 static HocParmLimits _hoc_parm_limits[] = {
 0,0,0
};
 static HocParmUnits _hoc_parm_units[] = {
 "vhm_shl1", "mV",
 "ka_shl1", "mV",
 "vhh_shl1", "mV",
 "ki_shl1", "mV",
 "atm_shl1", "ms",
 "btm_shl1", "mV",
 "ctm_shl1", "mV",
 "dtm_shl1", "mV",
 "etm_shl1", "mV",
 "ftm_shl1", "ms",
 "athf_shl1", "ms",
 "bthf_shl1", "mV",
 "cthf_shl1", "mV",
 "dthf_shl1", "ms",
 "aths_shl1", "ms",
 "bths_shl1", "mV",
 "cths_shl1", "mV",
 "dths_shl1", "ms",
 "gbshl1_shl1", "nS/um2",
 "tm_shl1", "ms",
 "thf_shl1", "ms",
 "ths_shl1", "ms",
 0,0
};
 static double delta_t = 0.01;
 static double hs0 = 0;
 static double hf0 = 0;
 static double m0 = 0;
 /* connect global user variables to hoc */
 static DoubScal hoc_scdoub[] = {
 "vhm_shl1", &vhm_shl1,
 "ka_shl1", &ka_shl1,
 "vhh_shl1", &vhh_shl1,
 "ki_shl1", &ki_shl1,
 "atm_shl1", &atm_shl1,
 "btm_shl1", &btm_shl1,
 "ctm_shl1", &ctm_shl1,
 "dtm_shl1", &dtm_shl1,
 "etm_shl1", &etm_shl1,
 "ftm_shl1", &ftm_shl1,
 "athf_shl1", &athf_shl1,
 "bthf_shl1", &bthf_shl1,
 "cthf_shl1", &cthf_shl1,
 "dthf_shl1", &dthf_shl1,
 "aths_shl1", &aths_shl1,
 "bths_shl1", &bths_shl1,
 "cths_shl1", &cths_shl1,
 "dths_shl1", &dths_shl1,
 0,0
};
 static DoubVec hoc_vdoub[] = {
 0,0,0
};
 static double _sav_indep;
 static void nrn_alloc(Prop*);
static void  nrn_init(NrnThread*, _Memb_list*, int);
static void nrn_state(NrnThread*, _Memb_list*, int);
 static void nrn_cur(NrnThread*, _Memb_list*, int);
static void  nrn_jacob(NrnThread*, _Memb_list*, int);
 
static int _ode_count(int);
static void _ode_map(int, double**, double**, double*, Datum*, double*, int);
static void _ode_spec(NrnThread*, _Memb_list*, int);
static void _ode_matsol(NrnThread*, _Memb_list*, int);
 
#define _cvode_ieq _ppvar[3]._i
 static void _ode_matsol_instance1(_threadargsproto_);
 /* connect range variables in _p that hoc is supposed to know about */
 static const char *_mechanism[] = {
 "7.7.0",
"shl1",
 "gbshl1_shl1",
 0,
 "minf_shl1",
 "hfinf_shl1",
 "hsinf_shl1",
 "tm_shl1",
 "thf_shl1",
 "ths_shl1",
 0,
 "m_shl1",
 "hf_shl1",
 "hs_shl1",
 0,
 0};
 static Symbol* _k_sym;
 
extern Prop* need_memb(Symbol*);

static void nrn_alloc(Prop* _prop) {
	Prop *prop_ion;
	double *_p; Datum *_ppvar;
 	_p = nrn_prop_data_alloc(_mechtype, 17, _prop);
 	/*initialize range parameters*/
 	gbshl1 = 1;
 	_prop->param = _p;
 	_prop->param_size = 17;
 	_ppvar = nrn_prop_datum_alloc(_mechtype, 4, _prop);
 	_prop->dparam = _ppvar;
 	/*connect ionic variables to this model*/
 prop_ion = need_memb(_k_sym);
 nrn_promote(prop_ion, 0, 1);
 	_ppvar[0]._pval = &prop_ion->param[0]; /* ek */
 	_ppvar[1]._pval = &prop_ion->param[3]; /* ik */
 	_ppvar[2]._pval = &prop_ion->param[4]; /* _ion_dikdv */
 
}
 static void _initlists();
  /* some states have an absolute tolerance */
 static Symbol** _atollist;
 static HocStateTolerance _hoc_state_tol[] = {
 0,0
};
 static void _update_ion_pointer(Datum*);
 extern Symbol* hoc_lookup(const char*);
extern void _nrn_thread_reg(int, int, void(*)(Datum*));
extern void _nrn_thread_table_reg(int, void(*)(double*, Datum*, Datum*, NrnThread*, int));
extern void hoc_register_tolerance(int, HocStateTolerance*, Symbol***);
extern void _cvode_abstol( Symbol**, double*, int);

 void _shl1_reg() {
	int _vectorized = 1;
  _initlists();
 	ion_reg("k", -10000.);
 	_k_sym = hoc_lookup("k_ion");
 	register_mech(_mechanism, nrn_alloc,nrn_cur, nrn_jacob, nrn_state, nrn_init, hoc_nrnpointerindex, 1);
 _mechtype = nrn_get_mechtype(_mechanism[1]);
     _nrn_setdata_reg(_mechtype, _setdata);
     _nrn_thread_reg(_mechtype, 2, _update_ion_pointer);
 #if NMODL_TEXT
  hoc_reg_nmodl_text(_mechtype, nmodl_file_text);
  hoc_reg_nmodl_filename(_mechtype, nmodl_filename);
#endif
  hoc_register_prop_size(_mechtype, 17, 4);
  hoc_register_dparam_semantics(_mechtype, 0, "k_ion");
  hoc_register_dparam_semantics(_mechtype, 1, "k_ion");
  hoc_register_dparam_semantics(_mechtype, 2, "k_ion");
  hoc_register_dparam_semantics(_mechtype, 3, "cvodeieq");
 	hoc_register_cvode(_mechtype, _ode_count, _ode_map, _ode_spec, _ode_matsol);
 	hoc_register_tolerance(_mechtype, _hoc_state_tol, &_atollist);
 	hoc_register_var(hoc_scdoub, hoc_vdoub, hoc_intfunc);
 	ivoc_help("help ?1 shl1 /home/mahchine/DynamicNeuronX/dev_mahchine_worm_intergrate/dynamicneuronx/Metaworm/interact/neural_model/components/mechanism/modfile/shl1.mod\n");
 hoc_register_limits(_mechtype, _hoc_parm_limits);
 hoc_register_units(_mechtype, _hoc_parm_units);
 }
static int _reset;
static char *modelname = "shl1 current for C. elegans neuron model";

static int error;
static int _ninits = 0;
static int _match_recurse=1;
static void _modl_cleanup(){ _match_recurse=1;}
static int setparames(_threadargsprotocomma_ double);
 
static int _ode_spec1(_threadargsproto_);
/*static int _ode_matsol1(_threadargsproto_);*/
 static int _slist1[3], _dlist1[3];
 static int states(_threadargsproto_);
 
/*CVODE*/
 static int _ode_spec1 (double* _p, Datum* _ppvar, Datum* _thread, NrnThread* _nt) {int _reset = 0; {
   setparames ( _threadargscomma_ v ) ;
   Dm = ( minf - m ) / tm ;
   Dhf = ( hfinf - hf ) / thf ;
   Dhs = ( hsinf - hs ) / ths ;
   }
 return _reset;
}
 static int _ode_matsol1 (double* _p, Datum* _ppvar, Datum* _thread, NrnThread* _nt) {
 setparames ( _threadargscomma_ v ) ;
 Dm = Dm  / (1. - dt*( ( ( ( - 1.0 ) ) ) / tm )) ;
 Dhf = Dhf  / (1. - dt*( ( ( ( - 1.0 ) ) ) / thf )) ;
 Dhs = Dhs  / (1. - dt*( ( ( ( - 1.0 ) ) ) / ths )) ;
  return 0;
}
 /*END CVODE*/
 static int states (double* _p, Datum* _ppvar, Datum* _thread, NrnThread* _nt) { {
   setparames ( _threadargscomma_ v ) ;
    m = m + (1. - exp(dt*(( ( ( - 1.0 ) ) ) / tm)))*(- ( ( ( minf ) ) / tm ) / ( ( ( ( - 1.0 ) ) ) / tm ) - m) ;
    hf = hf + (1. - exp(dt*(( ( ( - 1.0 ) ) ) / thf)))*(- ( ( ( hfinf ) ) / thf ) / ( ( ( ( - 1.0 ) ) ) / thf ) - hf) ;
    hs = hs + (1. - exp(dt*(( ( ( - 1.0 ) ) ) / ths)))*(- ( ( ( hsinf ) ) / ths ) / ( ( ( ( - 1.0 ) ) ) / ths ) - hs) ;
   }
  return 0;
}
 
static int  setparames ( _threadargsprotocomma_ double _lv ) {
   minf = 1.0 / ( 1.0 + exp ( - ( _lv - vhm ) / ka ) ) ;
   hfinf = 1.0 / ( 1.0 + exp ( ( _lv - vhh ) / ki ) ) ;
   hsinf = 1.0 / ( 1.0 + exp ( ( _lv - vhh ) / ki ) ) ;
   tm = atm / ( exp ( - ( _lv - btm ) / ctm ) + exp ( ( _lv - dtm ) / etm ) ) + ftm ;
   thf = athf / ( 1.0 + exp ( ( _lv - bthf ) / cthf ) ) + dthf ;
   ths = aths / ( 1.0 + exp ( ( _lv - bths ) / cths ) ) + dths ;
    return 0; }
 
static void _hoc_setparames(void) {
  double _r;
   double* _p; Datum* _ppvar; Datum* _thread; NrnThread* _nt;
   if (_extcall_prop) {_p = _extcall_prop->param; _ppvar = _extcall_prop->dparam;}else{ _p = (double*)0; _ppvar = (Datum*)0; }
  _thread = _extcall_thread;
  _nt = nrn_threads;
 _r = 1.;
 setparames ( _p, _ppvar, _thread, _nt, *getarg(1) );
 hoc_retpushx(_r);
}
 
static int _ode_count(int _type){ return 3;}
 
static void _ode_spec(NrnThread* _nt, _Memb_list* _ml, int _type) {
   double* _p; Datum* _ppvar; Datum* _thread;
   Node* _nd; double _v; int _iml, _cntml;
  _cntml = _ml->_nodecount;
  _thread = _ml->_thread;
  for (_iml = 0; _iml < _cntml; ++_iml) {
    _p = _ml->_data[_iml]; _ppvar = _ml->_pdata[_iml];
    _nd = _ml->_nodelist[_iml];
    v = NODEV(_nd);
  ek = _ion_ek;
     _ode_spec1 (_p, _ppvar, _thread, _nt);
  }}
 
static void _ode_map(int _ieq, double** _pv, double** _pvdot, double* _pp, Datum* _ppd, double* _atol, int _type) { 
	double* _p; Datum* _ppvar;
 	int _i; _p = _pp; _ppvar = _ppd;
	_cvode_ieq = _ieq;
	for (_i=0; _i < 3; ++_i) {
		_pv[_i] = _pp + _slist1[_i];  _pvdot[_i] = _pp + _dlist1[_i];
		_cvode_abstol(_atollist, _atol, _i);
	}
 }
 
static void _ode_matsol_instance1(_threadargsproto_) {
 _ode_matsol1 (_p, _ppvar, _thread, _nt);
 }
 
static void _ode_matsol(NrnThread* _nt, _Memb_list* _ml, int _type) {
   double* _p; Datum* _ppvar; Datum* _thread;
   Node* _nd; double _v; int _iml, _cntml;
  _cntml = _ml->_nodecount;
  _thread = _ml->_thread;
  for (_iml = 0; _iml < _cntml; ++_iml) {
    _p = _ml->_data[_iml]; _ppvar = _ml->_pdata[_iml];
    _nd = _ml->_nodelist[_iml];
    v = NODEV(_nd);
  ek = _ion_ek;
 _ode_matsol_instance1(_threadargs_);
 }}
 extern void nrn_update_ion_pointer(Symbol*, Datum*, int, int);
 static void _update_ion_pointer(Datum* _ppvar) {
   nrn_update_ion_pointer(_k_sym, _ppvar, 0, 0);
   nrn_update_ion_pointer(_k_sym, _ppvar, 1, 3);
   nrn_update_ion_pointer(_k_sym, _ppvar, 2, 4);
 }

static void initmodel(double* _p, Datum* _ppvar, Datum* _thread, NrnThread* _nt) {
  int _i; double _save;{
  hs = hs0;
  hf = hf0;
  m = m0;
 {
   setparames ( _threadargscomma_ v ) ;
   m = minf ;
   hf = hfinf ;
   hs = hsinf ;
   }
 
}
}

static void nrn_init(NrnThread* _nt, _Memb_list* _ml, int _type){
double* _p; Datum* _ppvar; Datum* _thread;
Node *_nd; double _v; int* _ni; int _iml, _cntml;
#if CACHEVEC
    _ni = _ml->_nodeindices;
#endif
_cntml = _ml->_nodecount;
_thread = _ml->_thread;
for (_iml = 0; _iml < _cntml; ++_iml) {
 _p = _ml->_data[_iml]; _ppvar = _ml->_pdata[_iml];
#if CACHEVEC
  if (use_cachevec) {
    _v = VEC_V(_ni[_iml]);
  }else
#endif
  {
    _nd = _ml->_nodelist[_iml];
    _v = NODEV(_nd);
  }
 v = _v;
  ek = _ion_ek;
 initmodel(_p, _ppvar, _thread, _nt);
 }
}

static double _nrn_current(double* _p, Datum* _ppvar, Datum* _thread, NrnThread* _nt, double _v){double _current=0.;v=_v;{ {
   ik = gbshl1 * m * m * m * ( 0.7 * hf + 0.3 * hs ) * ( v + 80.0 ) ;
   }
 _current += ik;

} return _current;
}

static void nrn_cur(NrnThread* _nt, _Memb_list* _ml, int _type) {
double* _p; Datum* _ppvar; Datum* _thread;
Node *_nd; int* _ni; double _rhs, _v; int _iml, _cntml;
#if CACHEVEC
    _ni = _ml->_nodeindices;
#endif
_cntml = _ml->_nodecount;
_thread = _ml->_thread;
for (_iml = 0; _iml < _cntml; ++_iml) {
 _p = _ml->_data[_iml]; _ppvar = _ml->_pdata[_iml];
#if CACHEVEC
  if (use_cachevec) {
    _v = VEC_V(_ni[_iml]);
  }else
#endif
  {
    _nd = _ml->_nodelist[_iml];
    _v = NODEV(_nd);
  }
  ek = _ion_ek;
 _g = _nrn_current(_p, _ppvar, _thread, _nt, _v + .001);
 	{ double _dik;
  _dik = ik;
 _rhs = _nrn_current(_p, _ppvar, _thread, _nt, _v);
  _ion_dikdv += (_dik - ik)/.001 ;
 	}
 _g = (_g - _rhs)/.001;
  _ion_ik += ik ;
#if CACHEVEC
  if (use_cachevec) {
	VEC_RHS(_ni[_iml]) -= _rhs;
  }else
#endif
  {
	NODERHS(_nd) -= _rhs;
  }
 
}
 
}

static void nrn_jacob(NrnThread* _nt, _Memb_list* _ml, int _type) {
double* _p; Datum* _ppvar; Datum* _thread;
Node *_nd; int* _ni; int _iml, _cntml;
#if CACHEVEC
    _ni = _ml->_nodeindices;
#endif
_cntml = _ml->_nodecount;
_thread = _ml->_thread;
for (_iml = 0; _iml < _cntml; ++_iml) {
 _p = _ml->_data[_iml];
#if CACHEVEC
  if (use_cachevec) {
	VEC_D(_ni[_iml]) += _g;
  }else
#endif
  {
     _nd = _ml->_nodelist[_iml];
	NODED(_nd) += _g;
  }
 
}
 
}

static void nrn_state(NrnThread* _nt, _Memb_list* _ml, int _type) {
double* _p; Datum* _ppvar; Datum* _thread;
Node *_nd; double _v = 0.0; int* _ni; int _iml, _cntml;
#if CACHEVEC
    _ni = _ml->_nodeindices;
#endif
_cntml = _ml->_nodecount;
_thread = _ml->_thread;
for (_iml = 0; _iml < _cntml; ++_iml) {
 _p = _ml->_data[_iml]; _ppvar = _ml->_pdata[_iml];
 _nd = _ml->_nodelist[_iml];
#if CACHEVEC
  if (use_cachevec) {
    _v = VEC_V(_ni[_iml]);
  }else
#endif
  {
    _nd = _ml->_nodelist[_iml];
    _v = NODEV(_nd);
  }
 v=_v;
{
  ek = _ion_ek;
 {   states(_p, _ppvar, _thread, _nt);
  } }}

}

static void terminal(){}

static void _initlists(){
 double _x; double* _p = &_x;
 int _i; static int _first = 1;
  if (!_first) return;
 _slist1[0] = &(m) - _p;  _dlist1[0] = &(Dm) - _p;
 _slist1[1] = &(hf) - _p;  _dlist1[1] = &(Dhf) - _p;
 _slist1[2] = &(hs) - _p;  _dlist1[2] = &(Dhs) - _p;
_first = 0;
}

#if defined(__cplusplus)
} /* extern "C" */
#endif

#if NMODL_TEXT
static const char* nmodl_filename = "/home/mahchine/DynamicNeuronX/dev_mahchine_worm_intergrate/dynamicneuronx/Metaworm/interact/neural_model/components/mechanism/modfile/shl1.mod";
static const char* nmodl_file_text = 
  "TITLE shl1 current for C. elegans neuron model\n"
  "\n"
  "\n"
  "COMMENT\n"
  "channel type: voltage-gated potassium channel\n"
  "\n"
  "model and parameters from:\n"
  "Nicoletti, M., et al. (2019). \"Biophysical modeling of C. elegans neurons: Single ion currents and whole-cell dynamics of AWCon and RMD.\" PLoS One 14(7): e0218738.\n"
  "ENDCOMMENT\n"
  "\n"
  "\n"
  "UNITS {\n"
  "    (nS) = (nanosiemens)\n"
  "    (mV) = (millivolt)\n"
  "    :(mA) = (milliamp)\n"
  "    (pA) = (picoamp)\n"
  "    (um) = (micron)\n"
  "}\n"
  "\n"
  "\n"
  "NEURON {\n"
  "    SUFFIX shl1\n"
  "    USEION k READ ek WRITE ik\n"
  "    RANGE gbshl1, minf, hfinf, hsinf, tm, thf, ths, m, hf, hs\n"
  "}\n"
  "\n"
  "\n"
  "PARAMETER {\n"
  "    gbshl1 = 1 (nS/um2)\n"
  "    vhm = 11.2 (mV)\n"
  "    ka = 14.1 (mV)\n"
  "    vhh = -33.1 (mV)\n"
  "    ki = 8.3 (mV)\n"
  "    atm = 13.8 (ms)\n"
  "    btm = -17.5 (mV)\n"
  "    ctm = 12.9 (mV)\n"
  "    dtm = -3.7 (mV)\n"
  "    etm = 6.5 (mV)\n"
  "    ftm = 1.9 (ms)\n"
  "    athf = 539.2 (ms)\n"
  "    bthf = -28.2 (mV)\n"
  "    cthf = 4.9 (mV)\n"
  "    dthf = 27.3 (ms)\n"
  "    aths = 8422.0 (ms)\n"
  "    bths = -37.7 (mV)\n"
  "    cths = 6.4 (mV)\n"
  "    dths = 118.9 (ms)\n"
  "}\n"
  "\n"
  "\n"
  "ASSIGNED {\n"
  "    v (mV)\n"
  "    ek (mV)\n"
  "    ik (pA/um2) :(mA/cm2)\n"
  "    minf\n"
  "    hfinf\n"
  "    hsinf\n"
  "    tm (ms)\n"
  "    thf (ms)\n"
  "    ths (ms)\n"
  "}\n"
  "\n"
  "\n"
  "STATE {\n"
  "    m\n"
  "    hf\n"
  "    hs\n"
  "}\n"
  "\n"
  "\n"
  "BREAKPOINT {\n"
  "    SOLVE states METHOD cnexp\n"
  "    :ik = gbshl1 * m * m * m * (0.7 * hf + 0.3 * hs) * (v - ek)\n"
  "    ik = gbshl1 * m * m * m * (0.7 * hf + 0.3 * hs) * (v + 80) : ek = -80\n"
  "}\n"
  "\n"
  "\n"
  "INITIAL {\n"
  "    setparames(v)\n"
  "    m = minf\n"
  "    hf = hfinf\n"
  "    hs = hsinf\n"
  "}\n"
  "\n"
  "\n"
  "DERIVATIVE states {\n"
  "    setparames(v)\n"
  "    m' = (minf - m) / tm\n"
  "    hf' = (hfinf - hf) / thf\n"
  "    hs' = (hsinf - hs) / ths\n"
  "}\n"
  "\n"
  "\n"
  "PROCEDURE setparames(v (mV)) {\n"
  "    minf = 1 / (1 + exp(-(v - vhm) / ka))\n"
  "    hfinf = 1 / (1 + exp((v - vhh) / ki))\n"
  "    hsinf = 1 / (1 + exp((v - vhh) / ki))\n"
  "    tm = atm / (exp(-(v - btm) / ctm) + exp((v - dtm) / etm)) + ftm\n"
  "    thf = athf / (1 + exp((v - bthf) / cthf)) + dthf\n"
  "    ths = aths / (1 + exp((v - bths) / cths)) + dths\n"
  "} \n"
  ;
#endif
