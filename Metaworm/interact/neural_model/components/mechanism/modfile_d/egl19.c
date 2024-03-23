/* Created by Language version: 7.7.0 */
/* VECTORIZED */
#define NRN_VECTORIZED 1
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "scoplib_ansi.h"
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
 
#define nrn_init _nrn_init__egl19
#define _nrn_initial _nrn_initial__egl19
#define nrn_cur _nrn_cur__egl19
#define _nrn_current _nrn_current__egl19
#define nrn_jacob _nrn_jacob__egl19
#define nrn_state _nrn_state__egl19
#define _net_receive _net_receive__egl19 
#define setparames setparames__egl19 
#define states states__egl19 
 
#define _threadargscomma_ _p, _ppvar, _thread, _nt,
#define _threadargsprotocomma_ double* _p, Datum* _ppvar, Datum* _thread, _NrnThread* _nt,
#define _threadargs_ _p, _ppvar, _thread, _nt
#define _threadargsproto_ double* _p, Datum* _ppvar, Datum* _thread, _NrnThread* _nt
 	/*SUPPRESS 761*/
	/*SUPPRESS 762*/
	/*SUPPRESS 763*/
	/*SUPPRESS 765*/
	 extern double *getarg();
 /* Thread safe. No static _p or _ppvar. */
 
#define t _nt->_t
#define dt _nt->_dt
#define gbegl19 _p[0]
#define m _p[1]
#define h _p[2]
#define eca _p[3]
#define ica _p[4]
#define minf _p[5]
#define hinf _p[6]
#define tm _p[7]
#define th _p[8]
#define Dm _p[9]
#define Dh _p[10]
#define v _p[11]
#define _g _p[12]
#define _ion_eca	*_ppvar[0]._pval
#define _ion_ica	*_ppvar[1]._pval
#define _ion_dicadv	*_ppvar[2]._pval
 
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
 "setdata_egl19", _hoc_setdata,
 "setparames_egl19", _hoc_setparames,
 0, 0
};
 /* declare global and static user variables */
#define ath ath_egl19
 double ath = 0.4;
#define atm atm_egl19
 double atm = 2.9;
#define ahinf ahinf_egl19
 double ahinf = 1.43;
#define bth bth_egl19
 double bth = 44.6;
#define btm btm_egl19
 double btm = 5.2;
#define bhinf bhinf_egl19
 double bhinf = 0.14;
#define cth cth_egl19
 double cth = -23;
#define ctm ctm_egl19
 double ctm = 6;
#define chinf chinf_egl19
 double chinf = 5.96;
#define dth dth_egl19
 double dth = 5;
#define dtm dtm_egl19
 double dtm = 1.9;
#define dhinf dhinf_egl19
 double dhinf = 0.6;
#define eth eth_egl19
 double eth = 36.4;
#define etm etm_egl19
 double etm = 1.4;
#define fth fth_egl19
 double fth = 28.7;
#define ftm ftm_egl19
 double ftm = 30;
#define gth gth_egl19
 double gth = 3.7;
#define gtm gtm_egl19
 double gtm = 2.3;
#define hth hth_egl19
 double hth = 43.1;
#define kib kib_egl19
 double kib = 8.1;
#define ki ki_egl19
 double ki = 12;
#define ka ka_egl19
 double ka = 7.5;
#define vhhb vhhb_egl19
 double vhhb = -20.5;
#define vhh vhh_egl19
 double vhh = 24.9;
#define vhm vhm_egl19
 double vhm = 5.6;
 /* some parameters have upper and lower limits */
 static HocParmLimits _hoc_parm_limits[] = {
 0,0,0
};
 static HocParmUnits _hoc_parm_units[] = {
 "vhm_egl19", "mV",
 "ka_egl19", "mV",
 "vhh_egl19", "mV",
 "ki_egl19", "mV",
 "vhhb_egl19", "mV",
 "kib_egl19", "mV",
 "atm_egl19", "ms",
 "btm_egl19", "mV",
 "ctm_egl19", "mV",
 "dtm_egl19", "ms",
 "etm_egl19", "mV",
 "ftm_egl19", "mV",
 "gtm_egl19", "ms",
 "bth_egl19", "ms",
 "cth_egl19", "mV",
 "dth_egl19", "mV",
 "eth_egl19", "ms",
 "fth_egl19", "mV",
 "gth_egl19", "mV",
 "hth_egl19", "ms",
 "gbegl19_egl19", "nS/um2",
 0,0
};
 static double delta_t = 0.01;
 static double h0 = 0;
 static double m0 = 0;
 /* connect global user variables to hoc */
 static DoubScal hoc_scdoub[] = {
 "vhm_egl19", &vhm_egl19,
 "ka_egl19", &ka_egl19,
 "vhh_egl19", &vhh_egl19,
 "ki_egl19", &ki_egl19,
 "vhhb_egl19", &vhhb_egl19,
 "kib_egl19", &kib_egl19,
 "ahinf_egl19", &ahinf_egl19,
 "bhinf_egl19", &bhinf_egl19,
 "chinf_egl19", &chinf_egl19,
 "dhinf_egl19", &dhinf_egl19,
 "atm_egl19", &atm_egl19,
 "btm_egl19", &btm_egl19,
 "ctm_egl19", &ctm_egl19,
 "dtm_egl19", &dtm_egl19,
 "etm_egl19", &etm_egl19,
 "ftm_egl19", &ftm_egl19,
 "gtm_egl19", &gtm_egl19,
 "ath_egl19", &ath_egl19,
 "bth_egl19", &bth_egl19,
 "cth_egl19", &cth_egl19,
 "dth_egl19", &dth_egl19,
 "eth_egl19", &eth_egl19,
 "fth_egl19", &fth_egl19,
 "gth_egl19", &gth_egl19,
 "hth_egl19", &hth_egl19,
 0,0
};
 static DoubVec hoc_vdoub[] = {
 0,0,0
};
 static double _sav_indep;
 static void nrn_alloc(Prop*);
static void  nrn_init(_NrnThread*, _Memb_list*, int);
static void nrn_state(_NrnThread*, _Memb_list*, int);
 static void nrn_cur(_NrnThread*, _Memb_list*, int);
static void  nrn_jacob(_NrnThread*, _Memb_list*, int);
 
static int _ode_count(int);
static void _ode_map(int, double**, double**, double*, Datum*, double*, int);
static void _ode_spec(_NrnThread*, _Memb_list*, int);
static void _ode_matsol(_NrnThread*, _Memb_list*, int);
 
#define _cvode_ieq _ppvar[3]._i
 static void _ode_matsol_instance1(_threadargsproto_);
 /* connect range variables in _p that hoc is supposed to know about */
 static const char *_mechanism[] = {
 "7.7.0",
"egl19",
 "gbegl19_egl19",
 0,
 0,
 "m_egl19",
 "h_egl19",
 0,
 0};
 static Symbol* _ca_sym;
 
extern Prop* need_memb(Symbol*);

static void nrn_alloc(Prop* _prop) {
	Prop *prop_ion;
	double *_p; Datum *_ppvar;
 	_p = nrn_prop_data_alloc(_mechtype, 13, _prop);
 	/*initialize range parameters*/
 	gbegl19 = 1;
 	_prop->param = _p;
 	_prop->param_size = 13;
 	_ppvar = nrn_prop_datum_alloc(_mechtype, 4, _prop);
 	_prop->dparam = _ppvar;
 	/*connect ionic variables to this model*/
 prop_ion = need_memb(_ca_sym);
 nrn_promote(prop_ion, 0, 1);
 	_ppvar[0]._pval = &prop_ion->param[0]; /* eca */
 	_ppvar[1]._pval = &prop_ion->param[3]; /* ica */
 	_ppvar[2]._pval = &prop_ion->param[4]; /* _ion_dicadv */
 
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
extern void _nrn_thread_table_reg(int, void(*)(double*, Datum*, Datum*, _NrnThread*, int));
extern void hoc_register_tolerance(int, HocStateTolerance*, Symbol***);
extern void _cvode_abstol( Symbol**, double*, int);

 void _egl19_reg() {
	int _vectorized = 1;
  _initlists();
 	ion_reg("ca", -10000.);
 	_ca_sym = hoc_lookup("ca_ion");
 	register_mech(_mechanism, nrn_alloc,nrn_cur, nrn_jacob, nrn_state, nrn_init, hoc_nrnpointerindex, 1);
 _mechtype = nrn_get_mechtype(_mechanism[1]);
     _nrn_setdata_reg(_mechtype, _setdata);
     _nrn_thread_reg(_mechtype, 2, _update_ion_pointer);
 #if NMODL_TEXT
  hoc_reg_nmodl_text(_mechtype, nmodl_file_text);
  hoc_reg_nmodl_filename(_mechtype, nmodl_filename);
#endif
  hoc_register_prop_size(_mechtype, 13, 4);
  hoc_register_dparam_semantics(_mechtype, 0, "ca_ion");
  hoc_register_dparam_semantics(_mechtype, 1, "ca_ion");
  hoc_register_dparam_semantics(_mechtype, 2, "ca_ion");
  hoc_register_dparam_semantics(_mechtype, 3, "cvodeieq");
 	hoc_register_cvode(_mechtype, _ode_count, _ode_map, _ode_spec, _ode_matsol);
 	hoc_register_tolerance(_mechtype, _hoc_state_tol, &_atollist);
 	hoc_register_var(hoc_scdoub, hoc_vdoub, hoc_intfunc);
 	ivoc_help("help ?1 egl19 C:/Users/maxy/PycharmProjects/SingleNeuron/generator/data/mechanism/modfile/egl19.mod\n");
 hoc_register_limits(_mechtype, _hoc_parm_limits);
 hoc_register_units(_mechtype, _hoc_parm_units);
 }
static int _reset;
static char *modelname = "egl19 current for C. elegans neuron model";

static int error;
static int _ninits = 0;
static int _match_recurse=1;
static void _modl_cleanup(){ _match_recurse=1;}
static int setparames(_threadargsprotocomma_ double);
 
static int _ode_spec1(_threadargsproto_);
/*static int _ode_matsol1(_threadargsproto_);*/
 static int _slist1[2], _dlist1[2];
 static int states(_threadargsproto_);
 
/*CVODE*/
 static int _ode_spec1 (double* _p, Datum* _ppvar, Datum* _thread, _NrnThread* _nt) {int _reset = 0; {
   setparames ( _threadargscomma_ v ) ;
   Dm = ( minf - m ) / tm ;
   Dh = ( hinf - h ) / th ;
   }
 return _reset;
}
 static int _ode_matsol1 (double* _p, Datum* _ppvar, Datum* _thread, _NrnThread* _nt) {
 setparames ( _threadargscomma_ v ) ;
 Dm = Dm  / (1. - dt*( ( ( ( - 1.0 ) ) ) / tm )) ;
 Dh = Dh  / (1. - dt*( ( ( ( - 1.0 ) ) ) / th )) ;
  return 0;
}
 /*END CVODE*/
 static int states (double* _p, Datum* _ppvar, Datum* _thread, _NrnThread* _nt) { {
   setparames ( _threadargscomma_ v ) ;
    m = m + (1. - exp(dt*(( ( ( - 1.0 ) ) ) / tm)))*(- ( ( ( minf ) ) / tm ) / ( ( ( ( - 1.0 ) ) ) / tm ) - m) ;
    h = h + (1. - exp(dt*(( ( ( - 1.0 ) ) ) / th)))*(- ( ( ( hinf ) ) / th ) / ( ( ( ( - 1.0 ) ) ) / th ) - h) ;
   }
  return 0;
}
 
static int  setparames ( _threadargsprotocomma_ double _lv ) {
   minf = 1.0 / ( 1.0 + exp ( - ( _lv - vhm ) / ka ) ) ;
   hinf = ( ahinf / ( 1.0 + exp ( - ( _lv - vhh ) / ki ) ) + bhinf ) * ( chinf / ( 1.0 + exp ( ( _lv - vhhb ) / kib ) ) + dhinf ) ;
   tm = atm * exp ( - pow( ( ( _lv - btm ) / ctm ) , 2.0 ) ) + dtm * exp ( - pow( ( ( _lv - etm ) / ftm ) , 2.0 ) ) + gtm ;
   th = ath * ( ( bth / ( 1.0 + exp ( ( _lv - cth ) / dth ) ) ) + ( eth / ( 1.0 + exp ( ( _lv - fth ) / gth ) ) ) + hth ) ;
    return 0; }
 
static void _hoc_setparames(void) {
  double _r;
   double* _p; Datum* _ppvar; Datum* _thread; _NrnThread* _nt;
   if (_extcall_prop) {_p = _extcall_prop->param; _ppvar = _extcall_prop->dparam;}else{ _p = (double*)0; _ppvar = (Datum*)0; }
  _thread = _extcall_thread;
  _nt = nrn_threads;
 _r = 1.;
 setparames ( _p, _ppvar, _thread, _nt, *getarg(1) );
 hoc_retpushx(_r);
}
 
static int _ode_count(int _type){ return 2;}
 
static void _ode_spec(_NrnThread* _nt, _Memb_list* _ml, int _type) {
   double* _p; Datum* _ppvar; Datum* _thread;
   Node* _nd; double _v; int _iml, _cntml;
  _cntml = _ml->_nodecount;
  _thread = _ml->_thread;
  for (_iml = 0; _iml < _cntml; ++_iml) {
    _p = _ml->_data[_iml]; _ppvar = _ml->_pdata[_iml];
    _nd = _ml->_nodelist[_iml];
    v = NODEV(_nd);
  eca = _ion_eca;
     _ode_spec1 (_p, _ppvar, _thread, _nt);
  }}
 
static void _ode_map(int _ieq, double** _pv, double** _pvdot, double* _pp, Datum* _ppd, double* _atol, int _type) { 
	double* _p; Datum* _ppvar;
 	int _i; _p = _pp; _ppvar = _ppd;
	_cvode_ieq = _ieq;
	for (_i=0; _i < 2; ++_i) {
		_pv[_i] = _pp + _slist1[_i];  _pvdot[_i] = _pp + _dlist1[_i];
		_cvode_abstol(_atollist, _atol, _i);
	}
 }
 
static void _ode_matsol_instance1(_threadargsproto_) {
 _ode_matsol1 (_p, _ppvar, _thread, _nt);
 }
 
static void _ode_matsol(_NrnThread* _nt, _Memb_list* _ml, int _type) {
   double* _p; Datum* _ppvar; Datum* _thread;
   Node* _nd; double _v; int _iml, _cntml;
  _cntml = _ml->_nodecount;
  _thread = _ml->_thread;
  for (_iml = 0; _iml < _cntml; ++_iml) {
    _p = _ml->_data[_iml]; _ppvar = _ml->_pdata[_iml];
    _nd = _ml->_nodelist[_iml];
    v = NODEV(_nd);
  eca = _ion_eca;
 _ode_matsol_instance1(_threadargs_);
 }}
 extern void nrn_update_ion_pointer(Symbol*, Datum*, int, int);
 static void _update_ion_pointer(Datum* _ppvar) {
   nrn_update_ion_pointer(_ca_sym, _ppvar, 0, 0);
   nrn_update_ion_pointer(_ca_sym, _ppvar, 1, 3);
   nrn_update_ion_pointer(_ca_sym, _ppvar, 2, 4);
 }

static void initmodel(double* _p, Datum* _ppvar, Datum* _thread, _NrnThread* _nt) {
  int _i; double _save;{
  h = h0;
  m = m0;
 {
   setparames ( _threadargscomma_ v ) ;
   m = minf ;
   h = hinf ;
   }
 
}
}

static void nrn_init(_NrnThread* _nt, _Memb_list* _ml, int _type){
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
  eca = _ion_eca;
 initmodel(_p, _ppvar, _thread, _nt);
 }
}

static double _nrn_current(double* _p, Datum* _ppvar, Datum* _thread, _NrnThread* _nt, double _v){double _current=0.;v=_v;{ {
   ica = gbegl19 * m * h * ( v - 60.0 ) ;
   }
 _current += ica;

} return _current;
}

static void nrn_cur(_NrnThread* _nt, _Memb_list* _ml, int _type) {
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
  eca = _ion_eca;
 _g = _nrn_current(_p, _ppvar, _thread, _nt, _v + .001);
 	{ double _dica;
  _dica = ica;
 _rhs = _nrn_current(_p, _ppvar, _thread, _nt, _v);
  _ion_dicadv += (_dica - ica)/.001 ;
 	}
 _g = (_g - _rhs)/.001;
  _ion_ica += ica ;
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

static void nrn_jacob(_NrnThread* _nt, _Memb_list* _ml, int _type) {
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

static void nrn_state(_NrnThread* _nt, _Memb_list* _ml, int _type) {
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
  eca = _ion_eca;
 {   states(_p, _ppvar, _thread, _nt);
  } }}

}

static void terminal(){}

static void _initlists(){
 double _x; double* _p = &_x;
 int _i; static int _first = 1;
  if (!_first) return;
 _slist1[0] = &(m) - _p;  _dlist1[0] = &(Dm) - _p;
 _slist1[1] = &(h) - _p;  _dlist1[1] = &(Dh) - _p;
_first = 0;
}

#if defined(__cplusplus)
} /* extern "C" */
#endif

#if NMODL_TEXT
static const char* nmodl_filename = "egl19.mod";
static const char* nmodl_file_text = 
  "TITLE egl19 current for C. elegans neuron model\n"
  "\n"
  "\n"
  "COMMENT\n"
  "voltage-gated calcium channel\n"
  "\n"
  "model and parameters from:\n"
  "Nicoletti, M., et al. (2019). \"Biophysical modeling of C. elegans neurons: Single ion currents and whole-cell dynamics of AWCon and RMD.\" PLoS One 14(7): e0218738.\n"
  "ENDCOMMENT\n"
  "\n"
  "\n"
  "UNITS {\n"
  "    (nS) = (nanosiemens)\n"
  "    (mV) = (millivolt)\n"
  "    (pA) = (picoamp)\n"
  "    (um) = (micron)\n"
  "}\n"
  "\n"
  "\n"
  "NEURON {\n"
  "    SUFFIX egl19\n"
  "    USEION ca READ eca WRITE ica\n"
  "    RANGE gbegl19\n"
  "}\n"
  "\n"
  "\n"
  "PARAMETER {\n"
  "    gbegl19 = 1 (nS/um2)\n"
  "    vhm = 5.6 (mV)\n"
  "    ka = 7.5 (mV)\n"
  "    vhh = 24.9 (mV)\n"
  "    ki = 12.0 (mV)\n"
  "    vhhb = -20.5 (mV)\n"
  "    kib = 8.1 (mV)\n"
  "    ahinf = 1.43\n"
  "    bhinf = 0.14\n"
  "    chinf = 5.96\n"
  "    dhinf = 0.60\n"
  "    atm = 2.9 (ms)\n"
  "    btm = 5.2 (mV)\n"
  "    ctm = 6.0 (mV)\n"
  "    dtm = 1.9 (ms)\n"
  "    etm = 1.4 (mV)\n"
  "    ftm = 30.0 (mV)\n"
  "    gtm = 2.3 (ms)\n"
  "    ath = 0.4\n"
  "    bth = 44.6 (ms)\n"
  "    cth = -23.0 (mV)\n"
  "    dth = 5.0 (mV)\n"
  "    eth = 36.4 (ms)\n"
  "    fth = 28.7 (mV)\n"
  "    gth = 3.7 (mV)\n"
  "    hth = 43.1 (ms)\n"
  "}\n"
  "\n"
  "\n"
  "ASSIGNED {\n"
  "    v (mV)\n"
  "    eca (mV)\n"
  "    ica (pA/um2)\n"
  "    minf\n"
  "    hinf\n"
  "    tm (ms)\n"
  "    th (ms)\n"
  "}\n"
  "\n"
  "\n"
  "STATE {\n"
  "    m\n"
  "    h\n"
  "}\n"
  "\n"
  "\n"
  "BREAKPOINT {\n"
  "    SOLVE states METHOD cnexp\n"
  "    :ica = gbegl19 * m * h * (v - eca)\n"
  "    ica = gbegl19 * m * h * (v - 60)  : eca=60 mV\n"
  "}\n"
  "\n"
  "\n"
  "INITIAL {\n"
  "    setparames(v)\n"
  "    m = minf\n"
  "    h = hinf\n"
  "}\n"
  "\n"
  "\n"
  "DERIVATIVE states {\n"
  "    setparames(v)\n"
  "    m' = (minf - m) / tm\n"
  "    h' = (hinf - h) / th\n"
  "}\n"
  "\n"
  "\n"
  "PROCEDURE setparames(v (mV)) {\n"
  "    minf = 1 / (1 + exp(-(v - vhm) / ka))\n"
  "    hinf = (ahinf / (1 + exp(-(v - vhh) / ki)) + bhinf) * (chinf / (1 + exp((v - vhhb) / kib)) + dhinf)\n"
  "    tm = atm * exp(-((v - btm) / ctm) ^ 2) + dtm * exp(-((v - etm) / ftm) ^ 2) + gtm\n"
  "    th = ath * ((bth / (1 + exp((v - cth) / dth))) + (eth / (1 + exp((v - fth) / gth))) + hth)\n"
  "}\n"
  ;
#endif
