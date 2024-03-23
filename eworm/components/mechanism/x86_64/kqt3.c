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
 
#define nrn_init _nrn_init__kqt3
#define _nrn_initial _nrn_initial__kqt3
#define nrn_cur _nrn_cur__kqt3
#define _nrn_current _nrn_current__kqt3
#define nrn_jacob _nrn_jacob__kqt3
#define nrn_state _nrn_state__kqt3
#define _net_receive _net_receive__kqt3 
#define setparames setparames__kqt3 
#define states states__kqt3 
 
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
#define gbkqt3 _p[0]
#define minf _p[1]
#define tmf _p[2]
#define tms _p[3]
#define winf _p[4]
#define sinf _p[5]
#define tw _p[6]
#define ts _p[7]
#define mf _p[8]
#define ms _p[9]
#define w _p[10]
#define s _p[11]
#define ek _p[12]
#define ik _p[13]
#define Dmf _p[14]
#define Dms _p[15]
#define Dw _p[16]
#define Ds _p[17]
#define v _p[18]
#define _g _p[19]
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
 "setdata_kqt3", _hoc_setdata,
 "setparames_kqt3", _hoc_setparames,
 0, 0
};
 /* declare global and static user variables */
#define ats ats_kqt3
 double ats = 500000;
#define atw atw_kqt3
 double atw = 0.544;
#define atms atms_kqt3
 double atms = 5503;
#define atmf atmf_kqt3
 double atmf = 395.3;
#define as as_kqt3
 double as = 0.34;
#define aw aw_kqt3
 double aw = 0.49;
#define btw btw_kqt3
 double btw = 29.2;
#define btms btms_kqt3
 double btms = -5345.4;
#define btmf btmf_kqt3
 double btmf = 38.1;
#define bs bs_kqt3
 double bs = 0.66;
#define bw bw_kqt3
 double bw = 0.51;
#define ctw ctw_kqt3
 double ctw = -48.09;
#define ctms ctms_kqt3
 double ctms = 0.02827;
#define ctmf ctmf_kqt3
 double ctmf = 33.59;
#define dtw dtw_kqt3
 double dtw = 48.83;
#define dtms dtms_kqt3
 double dtms = -23.9;
#define etms etms_kqt3
 double etms = -4590.6;
#define ftms ftms_kqt3
 double ftms = 0.0357;
#define gtms gtms_kqt3
 double gtms = 14.15;
#define kis kis_kqt3
 double kis = 12.3;
#define kiw kiw_kqt3
 double kiw = 28.78;
#define ka ka_kqt3
 double ka = 15.8008;
#define vhs vhs_kqt3
 double vhs = -45.3;
#define vhw vhw_kqt3
 double vhw = -1.084;
#define vhm vhm_kqt3
 double vhm = -12.6726;
 /* some parameters have upper and lower limits */
 static HocParmLimits _hoc_parm_limits[] = {
 0,0,0
};
 static HocParmUnits _hoc_parm_units[] = {
 "vhm_kqt3", "mV",
 "ka_kqt3", "mV",
 "vhw_kqt3", "mV",
 "kiw_kqt3", "mV",
 "vhs_kqt3", "mV",
 "kis_kqt3", "mV",
 "atmf_kqt3", "ms",
 "btmf_kqt3", "mV",
 "ctmf_kqt3", "mV",
 "atms_kqt3", "ms",
 "btms_kqt3", "ms",
 "ctms_kqt3", "1/mV",
 "dtms_kqt3", "mV",
 "etms_kqt3", "ms",
 "ftms_kqt3", "1/mV",
 "gtms_kqt3", "mV",
 "atw_kqt3", "ms",
 "btw_kqt3", "ms",
 "ctw_kqt3", "mV",
 "dtw_kqt3", "mV",
 "ats_kqt3", "ms",
 "gbkqt3_kqt3", "nS/um2",
 "tmf_kqt3", "ms",
 "tms_kqt3", "ms",
 "tw_kqt3", "ms",
 "ts_kqt3", "ms",
 0,0
};
 static double delta_t = 0.01;
 static double ms0 = 0;
 static double mf0 = 0;
 static double s0 = 0;
 static double w0 = 0;
 /* connect global user variables to hoc */
 static DoubScal hoc_scdoub[] = {
 "vhm_kqt3", &vhm_kqt3,
 "ka_kqt3", &ka_kqt3,
 "vhw_kqt3", &vhw_kqt3,
 "kiw_kqt3", &kiw_kqt3,
 "aw_kqt3", &aw_kqt3,
 "bw_kqt3", &bw_kqt3,
 "vhs_kqt3", &vhs_kqt3,
 "kis_kqt3", &kis_kqt3,
 "as_kqt3", &as_kqt3,
 "bs_kqt3", &bs_kqt3,
 "atmf_kqt3", &atmf_kqt3,
 "btmf_kqt3", &btmf_kqt3,
 "ctmf_kqt3", &ctmf_kqt3,
 "atms_kqt3", &atms_kqt3,
 "btms_kqt3", &btms_kqt3,
 "ctms_kqt3", &ctms_kqt3,
 "dtms_kqt3", &dtms_kqt3,
 "etms_kqt3", &etms_kqt3,
 "ftms_kqt3", &ftms_kqt3,
 "gtms_kqt3", &gtms_kqt3,
 "atw_kqt3", &atw_kqt3,
 "btw_kqt3", &btw_kqt3,
 "ctw_kqt3", &ctw_kqt3,
 "dtw_kqt3", &dtw_kqt3,
 "ats_kqt3", &ats_kqt3,
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
"kqt3",
 "gbkqt3_kqt3",
 0,
 "minf_kqt3",
 "tmf_kqt3",
 "tms_kqt3",
 "winf_kqt3",
 "sinf_kqt3",
 "tw_kqt3",
 "ts_kqt3",
 0,
 "mf_kqt3",
 "ms_kqt3",
 "w_kqt3",
 "s_kqt3",
 0,
 0};
 static Symbol* _k_sym;
 
extern Prop* need_memb(Symbol*);

static void nrn_alloc(Prop* _prop) {
	Prop *prop_ion;
	double *_p; Datum *_ppvar;
 	_p = nrn_prop_data_alloc(_mechtype, 20, _prop);
 	/*initialize range parameters*/
 	gbkqt3 = 1;
 	_prop->param = _p;
 	_prop->param_size = 20;
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

 void _kqt3_reg() {
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
  hoc_register_prop_size(_mechtype, 20, 4);
  hoc_register_dparam_semantics(_mechtype, 0, "k_ion");
  hoc_register_dparam_semantics(_mechtype, 1, "k_ion");
  hoc_register_dparam_semantics(_mechtype, 2, "k_ion");
  hoc_register_dparam_semantics(_mechtype, 3, "cvodeieq");
 	hoc_register_cvode(_mechtype, _ode_count, _ode_map, _ode_spec, _ode_matsol);
 	hoc_register_tolerance(_mechtype, _hoc_state_tol, &_atollist);
 	hoc_register_var(hoc_scdoub, hoc_vdoub, hoc_intfunc);
 	ivoc_help("help ?1 kqt3 /home/brains/worm_simulation/interact/worm-simulation/eworm/components/mechanism/modfile/kqt3.mod\n");
 hoc_register_limits(_mechtype, _hoc_parm_limits);
 hoc_register_units(_mechtype, _hoc_parm_units);
 }
static int _reset;
static char *modelname = "kqt3 current for C. elegans neuron model";

static int error;
static int _ninits = 0;
static int _match_recurse=1;
static void _modl_cleanup(){ _match_recurse=1;}
static int setparames(_threadargsprotocomma_ double);
 
static int _ode_spec1(_threadargsproto_);
/*static int _ode_matsol1(_threadargsproto_);*/
 static int _slist1[4], _dlist1[4];
 static int states(_threadargsproto_);
 
/*CVODE*/
 static int _ode_spec1 (double* _p, Datum* _ppvar, Datum* _thread, NrnThread* _nt) {int _reset = 0; {
   setparames ( _threadargscomma_ v ) ;
   Dmf = ( minf - mf ) / tmf ;
   Dms = ( minf - ms ) / tms ;
   Dw = ( winf - w ) / tw ;
   Ds = ( sinf - s ) / ts ;
   }
 return _reset;
}
 static int _ode_matsol1 (double* _p, Datum* _ppvar, Datum* _thread, NrnThread* _nt) {
 setparames ( _threadargscomma_ v ) ;
 Dmf = Dmf  / (1. - dt*( ( ( ( - 1.0 ) ) ) / tmf )) ;
 Dms = Dms  / (1. - dt*( ( ( ( - 1.0 ) ) ) / tms )) ;
 Dw = Dw  / (1. - dt*( ( ( ( - 1.0 ) ) ) / tw )) ;
 Ds = Ds  / (1. - dt*( ( ( ( - 1.0 ) ) ) / ts )) ;
  return 0;
}
 /*END CVODE*/
 static int states (double* _p, Datum* _ppvar, Datum* _thread, NrnThread* _nt) { {
   setparames ( _threadargscomma_ v ) ;
    mf = mf + (1. - exp(dt*(( ( ( - 1.0 ) ) ) / tmf)))*(- ( ( ( minf ) ) / tmf ) / ( ( ( ( - 1.0 ) ) ) / tmf ) - mf) ;
    ms = ms + (1. - exp(dt*(( ( ( - 1.0 ) ) ) / tms)))*(- ( ( ( minf ) ) / tms ) / ( ( ( ( - 1.0 ) ) ) / tms ) - ms) ;
    w = w + (1. - exp(dt*(( ( ( - 1.0 ) ) ) / tw)))*(- ( ( ( winf ) ) / tw ) / ( ( ( ( - 1.0 ) ) ) / tw ) - w) ;
    s = s + (1. - exp(dt*(( ( ( - 1.0 ) ) ) / ts)))*(- ( ( ( sinf ) ) / ts ) / ( ( ( ( - 1.0 ) ) ) / ts ) - s) ;
   }
  return 0;
}
 
static int  setparames ( _threadargsprotocomma_ double _lv ) {
   minf = 1.0 / ( 1.0 + exp ( - ( _lv - vhm ) / ka ) ) ;
   tmf = atmf / ( 1.0 + pow( ( ( _lv + btmf ) / ctmf ) , 2.0 ) ) ;
   tms = atms + btms / ( 1.0 + pow( 10.0 , ( - ctms * ( dtms - _lv ) ) ) ) + etms / ( 1.0 + pow( 10.0 , ( - ftms * ( gtms + _lv ) ) ) ) ;
   winf = aw + bw / ( 1.0 + exp ( ( _lv - vhw ) / kiw ) ) ;
   sinf = as + bs / ( 1.0 + exp ( ( _lv - vhs ) / kis ) ) ;
   tw = ( atw + btw / ( 1.0 + pow( ( ( _lv - ctw ) / dtw ) , 2.0 ) ) ) ;
   ts = ats ;
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
 
static int _ode_count(int _type){ return 4;}
 
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
	for (_i=0; _i < 4; ++_i) {
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
  ms = ms0;
  mf = mf0;
  s = s0;
  w = w0;
 {
   setparames ( _threadargscomma_ v ) ;
   mf = 0.0 ;
   ms = 0.0 ;
   w = winf ;
   s = sinf ;
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
   ik = gbkqt3 * ( 0.3 * mf + 0.7 * ms ) * w * s * ( v - ek ) ;
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
 _slist1[0] = &(mf) - _p;  _dlist1[0] = &(Dmf) - _p;
 _slist1[1] = &(ms) - _p;  _dlist1[1] = &(Dms) - _p;
 _slist1[2] = &(w) - _p;  _dlist1[2] = &(Dw) - _p;
 _slist1[3] = &(s) - _p;  _dlist1[3] = &(Ds) - _p;
_first = 0;
}

#if defined(__cplusplus)
} /* extern "C" */
#endif

#if NMODL_TEXT
static const char* nmodl_filename = "/home/brains/worm_simulation/interact/worm-simulation/eworm/components/mechanism/modfile/kqt3.mod";
static const char* nmodl_file_text = 
  "TITLE kqt3 current for C. elegans neuron model\n"
  "\n"
  "\n"
  "COMMENT\n"
  "voltage-gated potassium channel\n"
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
  "    SUFFIX kqt3\n"
  "    USEION k READ ek WRITE ik\n"
  "    RANGE gbkqt3, minf, tmf, tms, winf, sinf, tw, ts, mf, ms, w, s\n"
  "}\n"
  "\n"
  "\n"
  "PARAMETER {\n"
  "    gbkqt3 = 1 (nS/um2)\n"
  "    vhm = -12.6726 (mV)\n"
  "    ka = 15.8008 (mV)\n"
  "    vhw = -1.084 (mV)\n"
  "    kiw = 28.78 (mV)\n"
  "    aw = 0.49\n"
  "    bw = 0.51\n"
  "    vhs = -45.3 (mV)\n"
  "    kis = 12.3 (mV)\n"
  "    as = 0.34\n"
  "    bs =  0.66\n"
  "    atmf = 395.3 (ms)\n"
  "    btmf = 38.1 (mV)\n"
  "    ctmf = 33.59 (mV)\n"
  "    atms = 5503.0 (ms)\n"
  "    btms = -5345.4 (ms)\n"
  "    ctms = 0.02827 (1/mV)\n"
  "    dtms = -23.9 (mV)\n"
  "    etms = -4590.6 (ms)\n"
  "    ftms = 0.0357 (1/mV)\n"
  "    gtms = 14.15 (mV)\n"
  "    atw = 0.544 (ms)\n"
  "    btw = 29.2 (ms)\n"
  "    ctw = -48.09 (mV)\n"
  "    dtw = 48.83 (mV)\n"
  "    ats = 500e3 (ms)\n"
  "}\n"
  "\n"
  "\n"
  "ASSIGNED {\n"
  "    v (mV)\n"
  "    ek (mV)\n"
  "    ik (pA/um2)\n"
  "    minf\n"
  "    tmf (ms)\n"
  "    tms (ms)\n"
  "    winf\n"
  "    sinf\n"
  "    tw (ms)\n"
  "    ts (ms)\n"
  "}\n"
  "\n"
  "\n"
  "STATE {\n"
  "    mf\n"
  "    ms\n"
  "    w\n"
  "    s\n"
  "}\n"
  "\n"
  "\n"
  "BREAKPOINT {\n"
  "    SOLVE states METHOD cnexp\n"
  "    ik = gbkqt3 * (0.3 * mf + 0.7 * ms) * w * s * (v - ek)\n"
  "    :ik = gbkqt3 * (0.3 * mf + 0.7 * ms) * (v + 80)\n"
  "    :ek = -80\n"
  "}\n"
  "\n"
  "\n"
  "INITIAL {\n"
  "    setparames(v)\n"
  "    mf = 0\n"
  "    ms = 0\n"
  "    w = winf\n"
  "    s = sinf\n"
  "}\n"
  "\n"
  "\n"
  "DERIVATIVE states {\n"
  "    setparames(v)\n"
  "    mf' = (minf - mf) / tmf\n"
  "    ms' = (minf - ms) / tms\n"
  "    w' = (winf - w) / tw\n"
  "    s' = (sinf - s) / ts\n"
  "}\n"
  "\n"
  "\n"
  "PROCEDURE setparames(v (mV)) {\n"
  "    minf = 1 / (1 + exp(-(v - vhm) / ka))\n"
  "    tmf = atmf / (1 + ((v + btmf) / ctmf) ^ 2)\n"
  "    tms = atms + btms / (1 + 10 ^ (-ctms * (dtms - v))) + etms / (1 + 10 ^ (-ftms * (gtms + v)))\n"
  "    winf = aw + bw / (1 + exp((v - vhw) / kiw))\n"
  "    sinf = as + bs / (1 + exp((v - vhs) / kis))\n"
  "    tw = (atw + btw / (1 + ((v - ctw) / dtw) ^ 2))\n"
  "    ts = ats\n"
  "}\n"
  ;
#endif
