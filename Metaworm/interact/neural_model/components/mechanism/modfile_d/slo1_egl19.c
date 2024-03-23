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
 
#define nrn_init _nrn_init__slo1_egl19
#define _nrn_initial _nrn_initial__slo1_egl19
#define nrn_cur _nrn_cur__slo1_egl19
#define _nrn_current _nrn_current__slo1_egl19
#define nrn_jacob _nrn_jacob__slo1_egl19
#define nrn_state _nrn_state__slo1_egl19
#define _net_receive _net_receive__slo1_egl19 
#define setparames setparames__slo1_egl19 
#define states states__slo1_egl19 
 
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
#define gbslo1 _p[0]
#define m _p[1]
#define hcav _p[2]
#define mcav _p[3]
#define ek _p[4]
#define ik _p[5]
#define cai _p[6]
#define minf _p[7]
#define tm _p[8]
#define mcavinf _p[9]
#define tmcav _p[10]
#define hcavinf _p[11]
#define thcav _p[12]
#define Dm _p[13]
#define Dhcav _p[14]
#define Dmcav _p[15]
#define v _p[16]
#define _g _p[17]
#define _ion_ek	*_ppvar[0]._pval
#define _ion_ik	*_ppvar[1]._pval
#define _ion_dikdv	*_ppvar[2]._pval
#define _ion_cai	*_ppvar[3]._pval
 
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
 "setdata_slo1_egl19", _hoc_setdata,
 "setparames_slo1_egl19", _hoc_setparames,
 0, 0
};
 /* declare global and static user variables */
#define ath ath_slo1_egl19
 double ath = 0.4;
#define atm atm_slo1_egl19
 double atm = 2.9;
#define ahinf ahinf_slo1_egl19
 double ahinf = 1.43;
#define bth bth_slo1_egl19
 double bth = 44.6;
#define btm btm_slo1_egl19
 double btm = 5.2;
#define bhinf bhinf_slo1_egl19
 double bhinf = 0.14;
#define cth cth_slo1_egl19
 double cth = -23;
#define ctm ctm_slo1_egl19
 double ctm = 6;
#define chinf chinf_slo1_egl19
 double chinf = 5.96;
#define canci canci_slo1_egl19
 double canci = 0.05;
#define dth dth_slo1_egl19
 double dth = 5;
#define dtm dtm_slo1_egl19
 double dtm = 1.9;
#define dhinf dhinf_slo1_egl19
 double dhinf = 0.6;
#define eth eth_slo1_egl19
 double eth = 36.4;
#define etm etm_slo1_egl19
 double etm = 1.4;
#define fth fth_slo1_egl19
 double fth = 28.7;
#define ftm ftm_slo1_egl19
 double ftm = 30;
#define gth gth_slo1_egl19
 double gth = 3.7;
#define gtm gtm_slo1_egl19
 double gtm = 2.3;
#define hth hth_slo1_egl19
 double hth = 43.1;
#define kib kib_slo1_egl19
 double kib = 8.1;
#define ki ki_slo1_egl19
 double ki = 12;
#define ka ka_slo1_egl19
 double ka = 7.5;
#define kyx kyx_slo1_egl19
 double kyx = 34.34;
#define kxy kxy_slo1_egl19
 double kxy = 55.73;
#define nyx nyx_slo1_egl19
 double nyx = 0.0001;
#define nxy nxy_slo1_egl19
 double nxy = 1.3;
#define vhhb vhhb_slo1_egl19
 double vhhb = -20.5;
#define vhh vhh_slo1_egl19
 double vhh = 24.9;
#define vhm vhm_slo1_egl19
 double vhm = 5.6;
#define wop wop_slo1_egl19
 double wop = 0.16;
#define wom wom_slo1_egl19
 double wom = 3.15;
#define wxy wxy_slo1_egl19
 double wxy = -0.028;
#define wyx wyx_slo1_egl19
 double wyx = 0.013;
 /* some parameters have upper and lower limits */
 static HocParmLimits _hoc_parm_limits[] = {
 0,0,0
};
 static HocParmUnits _hoc_parm_units[] = {
 "wyx_slo1_egl19", "1/mV",
 "wxy_slo1_egl19", "1/mV",
 "wom_slo1_egl19", "1/ms",
 "wop_slo1_egl19", "1/ms",
 "kxy_slo1_egl19", "uM/um2",
 "kyx_slo1_egl19", "uM/um2",
 "canci_slo1_egl19", "uM/um2",
 "vhm_slo1_egl19", "mV",
 "ka_slo1_egl19", "mV",
 "vhh_slo1_egl19", "mV",
 "ki_slo1_egl19", "mV",
 "vhhb_slo1_egl19", "mV",
 "kib_slo1_egl19", "mV",
 "atm_slo1_egl19", "ms",
 "btm_slo1_egl19", "mV",
 "ctm_slo1_egl19", "mV",
 "dtm_slo1_egl19", "ms",
 "etm_slo1_egl19", "mV",
 "ftm_slo1_egl19", "mV",
 "gtm_slo1_egl19", "ms",
 "bth_slo1_egl19", "ms",
 "cth_slo1_egl19", "mV",
 "dth_slo1_egl19", "mV",
 "eth_slo1_egl19", "ms",
 "fth_slo1_egl19", "mV",
 "gth_slo1_egl19", "mV",
 "hth_slo1_egl19", "ms",
 "gbslo1_slo1_egl19", "nS/um2",
 0,0
};
 static double delta_t = 0.01;
 static double hcav0 = 0;
 static double mcav0 = 0;
 static double m0 = 0;
 /* connect global user variables to hoc */
 static DoubScal hoc_scdoub[] = {
 "wyx_slo1_egl19", &wyx_slo1_egl19,
 "wxy_slo1_egl19", &wxy_slo1_egl19,
 "wom_slo1_egl19", &wom_slo1_egl19,
 "wop_slo1_egl19", &wop_slo1_egl19,
 "kxy_slo1_egl19", &kxy_slo1_egl19,
 "nxy_slo1_egl19", &nxy_slo1_egl19,
 "kyx_slo1_egl19", &kyx_slo1_egl19,
 "nyx_slo1_egl19", &nyx_slo1_egl19,
 "canci_slo1_egl19", &canci_slo1_egl19,
 "vhm_slo1_egl19", &vhm_slo1_egl19,
 "ka_slo1_egl19", &ka_slo1_egl19,
 "vhh_slo1_egl19", &vhh_slo1_egl19,
 "ki_slo1_egl19", &ki_slo1_egl19,
 "vhhb_slo1_egl19", &vhhb_slo1_egl19,
 "kib_slo1_egl19", &kib_slo1_egl19,
 "ahinf_slo1_egl19", &ahinf_slo1_egl19,
 "bhinf_slo1_egl19", &bhinf_slo1_egl19,
 "chinf_slo1_egl19", &chinf_slo1_egl19,
 "dhinf_slo1_egl19", &dhinf_slo1_egl19,
 "atm_slo1_egl19", &atm_slo1_egl19,
 "btm_slo1_egl19", &btm_slo1_egl19,
 "ctm_slo1_egl19", &ctm_slo1_egl19,
 "dtm_slo1_egl19", &dtm_slo1_egl19,
 "etm_slo1_egl19", &etm_slo1_egl19,
 "ftm_slo1_egl19", &ftm_slo1_egl19,
 "gtm_slo1_egl19", &gtm_slo1_egl19,
 "ath_slo1_egl19", &ath_slo1_egl19,
 "bth_slo1_egl19", &bth_slo1_egl19,
 "cth_slo1_egl19", &cth_slo1_egl19,
 "dth_slo1_egl19", &dth_slo1_egl19,
 "eth_slo1_egl19", &eth_slo1_egl19,
 "fth_slo1_egl19", &fth_slo1_egl19,
 "gth_slo1_egl19", &gth_slo1_egl19,
 "hth_slo1_egl19", &hth_slo1_egl19,
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
 
#define _cvode_ieq _ppvar[4]._i
 static void _ode_matsol_instance1(_threadargsproto_);
 /* connect range variables in _p that hoc is supposed to know about */
 static const char *_mechanism[] = {
 "7.7.0",
"slo1_egl19",
 "gbslo1_slo1_egl19",
 0,
 0,
 "m_slo1_egl19",
 "hcav_slo1_egl19",
 "mcav_slo1_egl19",
 0,
 0};
 static Symbol* _k_sym;
 static Symbol* _ca_sym;
 
extern Prop* need_memb(Symbol*);

static void nrn_alloc(Prop* _prop) {
	Prop *prop_ion;
	double *_p; Datum *_ppvar;
 	_p = nrn_prop_data_alloc(_mechtype, 18, _prop);
 	/*initialize range parameters*/
 	gbslo1 = 1;
 	_prop->param = _p;
 	_prop->param_size = 18;
 	_ppvar = nrn_prop_datum_alloc(_mechtype, 5, _prop);
 	_prop->dparam = _ppvar;
 	/*connect ionic variables to this model*/
 prop_ion = need_memb(_k_sym);
 nrn_promote(prop_ion, 0, 1);
 	_ppvar[0]._pval = &prop_ion->param[0]; /* ek */
 	_ppvar[1]._pval = &prop_ion->param[3]; /* ik */
 	_ppvar[2]._pval = &prop_ion->param[4]; /* _ion_dikdv */
 prop_ion = need_memb(_ca_sym);
 nrn_promote(prop_ion, 1, 0);
 	_ppvar[3]._pval = &prop_ion->param[1]; /* cai */
 
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

 void _slo1_egl19_reg() {
	int _vectorized = 1;
  _initlists();
 	ion_reg("k", -10000.);
 	ion_reg("ca", -10000.);
 	_k_sym = hoc_lookup("k_ion");
 	_ca_sym = hoc_lookup("ca_ion");
 	register_mech(_mechanism, nrn_alloc,nrn_cur, nrn_jacob, nrn_state, nrn_init, hoc_nrnpointerindex, 1);
 _mechtype = nrn_get_mechtype(_mechanism[1]);
     _nrn_setdata_reg(_mechtype, _setdata);
     _nrn_thread_reg(_mechtype, 2, _update_ion_pointer);
 #if NMODL_TEXT
  hoc_reg_nmodl_text(_mechtype, nmodl_file_text);
  hoc_reg_nmodl_filename(_mechtype, nmodl_filename);
#endif
  hoc_register_prop_size(_mechtype, 18, 5);
  hoc_register_dparam_semantics(_mechtype, 0, "k_ion");
  hoc_register_dparam_semantics(_mechtype, 1, "k_ion");
  hoc_register_dparam_semantics(_mechtype, 2, "k_ion");
  hoc_register_dparam_semantics(_mechtype, 3, "ca_ion");
  hoc_register_dparam_semantics(_mechtype, 4, "cvodeieq");
 	hoc_register_cvode(_mechtype, _ode_count, _ode_map, _ode_spec, _ode_matsol);
 	hoc_register_tolerance(_mechtype, _hoc_state_tol, &_atollist);
 	hoc_register_var(hoc_scdoub, hoc_vdoub, hoc_intfunc);
 	ivoc_help("help ?1 slo1_egl19 C:/Users/maxy/PycharmProjects/SingleNeuron/generator/data/mechanism/modfile/slo1_egl19.mod\n");
 hoc_register_limits(_mechtype, _hoc_parm_limits);
 hoc_register_units(_mechtype, _hoc_parm_units);
 }
static int _reset;
static char *modelname = "slo1 current for C. elegans neuron model (suppose to interact with egl19)";

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
 static int _ode_spec1 (double* _p, Datum* _ppvar, Datum* _thread, _NrnThread* _nt) {int _reset = 0; {
   setparames ( _threadargscomma_ v ) ;
   Dmcav = ( mcavinf - mcav ) / tmcav ;
   Dhcav = ( hcavinf - hcav ) / thcav ;
   Dm = ( minf - m ) / tm ;
   }
 return _reset;
}
 static int _ode_matsol1 (double* _p, Datum* _ppvar, Datum* _thread, _NrnThread* _nt) {
 setparames ( _threadargscomma_ v ) ;
 Dmcav = Dmcav  / (1. - dt*( ( ( ( - 1.0 ) ) ) / tmcav )) ;
 Dhcav = Dhcav  / (1. - dt*( ( ( ( - 1.0 ) ) ) / thcav )) ;
 Dm = Dm  / (1. - dt*( ( ( ( - 1.0 ) ) ) / tm )) ;
  return 0;
}
 /*END CVODE*/
 static int states (double* _p, Datum* _ppvar, Datum* _thread, _NrnThread* _nt) { {
   setparames ( _threadargscomma_ v ) ;
    mcav = mcav + (1. - exp(dt*(( ( ( - 1.0 ) ) ) / tmcav)))*(- ( ( ( mcavinf ) ) / tmcav ) / ( ( ( ( - 1.0 ) ) ) / tmcav ) - mcav) ;
    hcav = hcav + (1. - exp(dt*(( ( ( - 1.0 ) ) ) / thcav)))*(- ( ( ( hcavinf ) ) / thcav ) / ( ( ( ( - 1.0 ) ) ) / thcav ) - hcav) ;
    m = m + (1. - exp(dt*(( ( ( - 1.0 ) ) ) / tm)))*(- ( ( ( minf ) ) / tm ) / ( ( ( ( - 1.0 ) ) ) / tm ) - m) ;
   }
  return 0;
}
 
static int  setparames ( _threadargsprotocomma_ double _lv ) {
   double _lalpha , _lbeta , _lwm , _lwp , _lfm , _lfp , _lkom , _lkop , _lkcm ;
 mcavinf = 1.0 / ( 1.0 + exp ( - ( _lv - vhm ) / ka ) ) ;
   hcavinf = ( ahinf / ( 1.0 + exp ( - ( _lv - vhh ) / ki ) ) + bhinf ) * ( chinf / ( 1.0 + exp ( ( _lv - vhhb ) / kib ) ) + dhinf ) ;
   tmcav = atm * exp ( - pow( ( ( _lv - btm ) / ctm ) , 2.0 ) ) + dtm * exp ( - pow( ( ( _lv - etm ) / ftm ) , 2.0 ) ) + gtm ;
   thcav = ath * ( ( bth / ( 1.0 + exp ( ( _lv - cth ) / dth ) ) ) + ( eth / ( 1.0 + exp ( ( _lv - fth ) / gth ) ) ) + hth ) ;
   _lalpha = mcavinf / tmcav ;
   _lbeta = 1.0 / tmcav - _lalpha ;
   _lwm = wom * exp ( - wyx * _lv ) ;
   _lwp = wop * exp ( - wxy * _lv ) ;
   _lfm = 1.0 / ( 1.0 + pow( ( cai / kyx ) , nyx ) ) ;
   _lfp = 1.0 / ( 1.0 + pow( ( kxy / cai ) , nxy ) ) ;
   _lkom = _lwm * _lfm ;
   _lkop = _lwp * _lfp ;
   _lkcm = _lwm * 1.0 / ( 1.0 + pow( ( canci / kyx ) , nyx ) ) ;
   minf = mcav * _lkop * ( _lalpha + _lbeta + _lkcm ) / ( ( _lkop + _lkom ) * ( _lkcm + _lalpha ) + _lbeta * _lkcm ) ;
   tm = ( _lalpha + _lbeta + _lkcm ) / ( ( _lkop + _lkom ) * ( _lkcm + _lalpha ) + _lbeta * _lkcm ) ;
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
 
static int _ode_count(int _type){ return 3;}
 
static void _ode_spec(_NrnThread* _nt, _Memb_list* _ml, int _type) {
   double* _p; Datum* _ppvar; Datum* _thread;
   Node* _nd; double _v; int _iml, _cntml;
  _cntml = _ml->_nodecount;
  _thread = _ml->_thread;
  for (_iml = 0; _iml < _cntml; ++_iml) {
    _p = _ml->_data[_iml]; _ppvar = _ml->_pdata[_iml];
    _nd = _ml->_nodelist[_iml];
    v = NODEV(_nd);
  ek = _ion_ek;
  cai = _ion_cai;
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
 
static void _ode_matsol(_NrnThread* _nt, _Memb_list* _ml, int _type) {
   double* _p; Datum* _ppvar; Datum* _thread;
   Node* _nd; double _v; int _iml, _cntml;
  _cntml = _ml->_nodecount;
  _thread = _ml->_thread;
  for (_iml = 0; _iml < _cntml; ++_iml) {
    _p = _ml->_data[_iml]; _ppvar = _ml->_pdata[_iml];
    _nd = _ml->_nodelist[_iml];
    v = NODEV(_nd);
  ek = _ion_ek;
  cai = _ion_cai;
 _ode_matsol_instance1(_threadargs_);
 }}
 extern void nrn_update_ion_pointer(Symbol*, Datum*, int, int);
 static void _update_ion_pointer(Datum* _ppvar) {
   nrn_update_ion_pointer(_k_sym, _ppvar, 0, 0);
   nrn_update_ion_pointer(_k_sym, _ppvar, 1, 3);
   nrn_update_ion_pointer(_k_sym, _ppvar, 2, 4);
   nrn_update_ion_pointer(_ca_sym, _ppvar, 3, 1);
 }

static void initmodel(double* _p, Datum* _ppvar, Datum* _thread, _NrnThread* _nt) {
  int _i; double _save;{
  hcav = hcav0;
  mcav = mcav0;
  m = m0;
 {
   setparames ( _threadargscomma_ v ) ;
   hcav = hcavinf ;
   mcav = mcavinf ;
   m = minf ;
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
  ek = _ion_ek;
  cai = _ion_cai;
 initmodel(_p, _ppvar, _thread, _nt);
 }
}

static double _nrn_current(double* _p, Datum* _ppvar, Datum* _thread, _NrnThread* _nt, double _v){double _current=0.;v=_v;{ {
   ik = gbslo1 * m * hcav * ( v + 80.0 ) ;
   }
 _current += ik;

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
  ek = _ion_ek;
  cai = _ion_cai;
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
  ek = _ion_ek;
  cai = _ion_cai;
 {   states(_p, _ppvar, _thread, _nt);
  } }}

}

static void terminal(){}

static void _initlists(){
 double _x; double* _p = &_x;
 int _i; static int _first = 1;
  if (!_first) return;
 _slist1[0] = &(mcav) - _p;  _dlist1[0] = &(Dmcav) - _p;
 _slist1[1] = &(hcav) - _p;  _dlist1[1] = &(Dhcav) - _p;
 _slist1[2] = &(m) - _p;  _dlist1[2] = &(Dm) - _p;
_first = 0;
}

#if defined(__cplusplus)
} /* extern "C" */
#endif

#if NMODL_TEXT
static const char* nmodl_filename = "slo1_egl19.mod";
static const char* nmodl_file_text = 
  "TITLE slo1 current for C. elegans neuron model (suppose to interact with egl19)\n"
  "\n"
  "\n"
  "COMMENT\n"
  "channel type: calcium-related potassium channel\n"
  "\n"
  "slo1-egl19 1:1 stoichiometry\n"
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
  "    (molar) = (1/liter)  : moles do not appear in units\n"
  "    (uM) = (micromolar)\n"
  "}\n"
  "\n"
  "\n"
  "NEURON {\n"
  "    SUFFIX slo1_egl19\n"
  "    USEION k READ ek WRITE ik\n"
  "    USEION ca READ cai\n"
  "    RANGE gbslo1\n"
  "}\n"
  "\n"
  "\n"
  "PARAMETER {\n"
  "    : slo1\n"
  "    gbslo1 = 1 (nS/um2)\n"
  "    wyx = 0.013 (1/mV)\n"
  "    wxy = -0.028 (1/mV)\n"
  "    wom = 3.15 (1/ms)\n"
  "    wop = 0.16 (1/ms)\n"
  "    kxy = 55.73 (uM/um2)\n"
  "    nxy = 1.30\n"
  "    kyx = 34.34 (uM/um2)\n"
  "    nyx = 0.0001\n"
  "    canci = 0.05 (uM/um2) : calcium concentration When the calcium channel is closed\n"
  "\n"
  "    : egl19\n"
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
  "    ek (mV)\n"
  "    ik (pA/um2)\n"
  "    cai (uM/um2)\n"
  "    minf\n"
  "    tm (ms)\n"
  "    mcavinf\n"
  "    tmcav (ms)\n"
  "    hcavinf\n"
  "    thcav (ms)\n"
  "}\n"
  "\n"
  "\n"
  "STATE {\n"
  "    m\n"
  "    hcav\n"
  "    mcav\n"
  "}\n"
  "\n"
  "\n"
  "BREAKPOINT {\n"
  "    SOLVE states METHOD cnexp\n"
  "    ik = gbslo1 * m * hcav * (v + 80) : ek=-80\n"
  "}\n"
  "\n"
  "\n"
  "INITIAL {\n"
  "    setparames(v)\n"
  "    hcav = hcavinf\n"
  "    mcav = mcavinf\n"
  "    m = minf\n"
  "}\n"
  "\n"
  "\n"
  "DERIVATIVE states {\n"
  "    setparames(v)\n"
  "    mcav' = (mcavinf - mcav) / tmcav\n"
  "    hcav' = (hcavinf - hcav) / thcav\n"
  "    m' = (minf - m) / tm    \n"
  "}\n"
  "\n"
  "\n"
  "PROCEDURE setparames(v (mV)) {\n"
  "    LOCAL alpha, beta, wm, wp, fm, fp, kom, kop, kcm\n"
  "    : egl19 model\n"
  "    mcavinf = 1 / (1 + exp(-(v - vhm) / ka))\n"
  "    hcavinf = (ahinf / (1 + exp(-(v - vhh) / ki)) + bhinf) * (chinf / (1 + exp((v - vhhb) / kib)) + dhinf)\n"
  "    tmcav = atm * exp(-((v - btm) / ctm) ^ 2) + dtm * exp(-((v - etm) / ftm) ^ 2) + gtm\n"
  "    thcav = ath * ((bth / (1 + exp((v - cth) / dth))) + (eth / (1 + exp((v - fth) / gth))) + hth)\n"
  "    : slo1-egl19 model\n"
  "    alpha = mcavinf / tmcav\n"
  "    beta = 1 / tmcav - alpha\n"
  "    wm = wom * exp(-wyx * v)\n"
  "    wp = wop * exp(-wxy * v)\n"
  "    fm = 1 / (1 + (cai / kyx) ^ nyx)\n"
  "    fp = 1 / (1 + (kxy / cai) ^ nxy)\n"
  "    kom = wm * fm\n"
  "    kop = wp * fp\n"
  "    kcm = wm * 1 / (1 + (canci / kyx) ^ nyx)\n"
  "    minf = mcav * kop * (alpha + beta + kcm) / ((kop + kom) * (kcm + alpha) + beta * kcm)\n"
  "    tm = (alpha + beta + kcm) / ((kop + kom) * (kcm + alpha) + beta * kcm)\n"
  "}\n"
  ;
#endif
