/* This header file provides the reentrancy.  */

/* WARNING: All identifiers here must begin with an underscore.  This file is
   included by stdio.h and others and we therefore must only use identifiers
   in the namespace allotted to us.  */

#ifndef _SYS_REENT_H_
#ifdef __cplusplus
extern "C" {
#endif
#define _SYS_REENT_H_

#include <_ansi.h>
#include <stddef.h>
#include <sys/cdefs.h>
#include <sys/_types.h>

#define _NULL 0

#ifndef __Long
#if __LONG_MAX__ == 2147483647L
#define __Long long
typedef unsigned __Long __ULong;
#elif __INT_MAX__ == 2147483647
#define __Long int
typedef unsigned __Long __ULong;
#endif
#endif

#if !defined( __Long)
#include <sys/types.h>
#endif

#ifndef __machine_flock_t_defined
#include <sys/lock.h>
typedef _LOCK_RECURSIVE_T _flock_t;
#endif

#ifndef __Long
#define __Long __int32_t
typedef __uint32_t __ULong;
#endif

struct _reent;

struct __locale_t;

/*
 * If _REENT_SMALL is defined, we make struct _reent as small as possible,
 * by having nearly everything possible allocated at first use.
 */

struct _Bigint
{
  struct _Bigint *_next;
  int _k, _maxwds, _sign, _wds;
  __ULong _x[1];
};

/* needed by reentrant structure */
struct __tm
{
  int   __tm_sec;
  int   __tm_min;
  int   __tm_hour;
  int   __tm_mday;
  int   __tm_mon;
  int   __tm_year;
  int   __tm_wday;
  int   __tm_yday;
  int   __tm_isdst;
};

/*
 * atexit() support.
 */

#define	_ATEXIT_SIZE 32	/* must be at least 32 to guarantee ANSI conformance */

struct _on_exit_args {
	void *  _fnargs[_ATEXIT_SIZE];	        /* user fn args */
	void *	_dso_handle[_ATEXIT_SIZE];
	/* Bitmask is set if user function takes arguments.  */
	__ULong _fntypes;           	        /* type of exit routine -
				   Must have at least _ATEXIT_SIZE bits */
	/* Bitmask is set if function was registered via __cxa_atexit.  */
	__ULong _is_cxa;
};

#ifdef _REENT_SMALL
struct _atexit {
	struct	_atexit *_next;			/* next in list */
	int	_ind;				/* next index in this table */
	void	(*_fns[_ATEXIT_SIZE])(void);	/* the table itself */
        struct _on_exit_args * _on_exit_args_ptr;
};
# define _ATEXIT_INIT {_NULL, 0, {_NULL}, _NULL}
#else
struct _atexit {
	struct	_atexit *_next;			/* next in list */
	int	_ind;				/* next index in this table */
	/* Some entries may already have been called, and will be NULL.  */
	void	(*_fns[_ATEXIT_SIZE])(void);	/* the table itself */
        struct _on_exit_args _on_exit_args;
};
# define _ATEXIT_INIT {_NULL, 0, {_NULL}, {{_NULL}, {_NULL}, 0, 0}}
#endif

/*
 * Stdio buffers.
 *
 * This and __FILE are defined here because we need them for struct _reent,
 * but we don't want stdio.h included when stdlib.h is.
 */

struct __sbuf {
	unsigned char *_base;
	int	_size;
};

/*
 * Stdio state variables.
 *
 * The following always hold:
 *
 *	if (_flags&(__SLBF|__SWR)) == (__SLBF|__SWR),
 *		_lbfsize is -_bf._size, else _lbfsize is 0
 *	if _flags&__SRD, _w is 0
 *	if _flags&__SWR, _r is 0
 *
 * This ensures that the getc and putc macros (or inline functions) never
 * try to write or read from a file that is in `read' or `write' mode.
 * (Moreover, they can, and do, automatically switch from read mode to
 * write mode, and back, on "r+" and "w+" files.)
 *
 * _lbfsize is used only to make the inline line-buffered output stream
 * code as compact as possible.
 *
 * _ub, _up, and _ur are used when ungetc() pushes back more characters
 * than fit in the current _bf, or when ungetc() pushes back a character
 * that does not match the previous one in _bf.  When this happens,
 * _ub._base becomes non-nil (i.e., a stream has ungetc() data iff
 * _ub._base!=NULL) and _up and _ur save the current values of _p and _r.
 */

#define _REENT_SMALL_CHECK_INIT(ptr) /* nothing */

/* Cygwin must use __sFILE64 for backward compatibility, even though
   it's not defining __LARGE64_FILES anymore.  To make sure that __sFILE
   is never defined, disable it here explicitely. */
#ifndef __CYGWIN__

struct __sFILE {
  unsigned char *_p;	/* current position in (some) buffer */
  int	_r;		/* read space left for getc() */
  int	_w;		/* write space left for putc() */
  short	_flags;		/* flags, below; this FILE is free if 0 */
  short	_file;		/* fileno, if Unix descriptor, else -1 */
  struct __sbuf _bf;	/* the buffer (at least 1 byte, if !NULL) */
  int	_lbfsize;	/* 0 or -_bf._size, for inline putc */

#ifdef _REENT_SMALL
  struct _reent *_data;
#endif

  /* operations */
  void *	_cookie;	/* cookie passed to io functions */

  _READ_WRITE_RETURN_TYPE (*_read) (struct _reent *, void *,
					   char *, _READ_WRITE_BUFSIZE_TYPE);
  _READ_WRITE_RETURN_TYPE (*_write) (struct _reent *, void *,
					    const char *,
					    _READ_WRITE_BUFSIZE_TYPE);
  _fpos_t (*_seek) (struct _reent *, void *, _fpos_t, int);
  int (*_close) (struct _reent *, void *);

  /* separate buffer for long sequences of ungetc() */
  struct __sbuf _ub;	/* ungetc buffer */
  unsigned char *_up;	/* saved _p when _p is doing ungetc data */
  int	_ur;		/* saved _r when _r is counting ungetc data */

  /* tricks to meet minimum requirements even when malloc() fails */
  unsigned char _ubuf[3];	/* guarantee an ungetc() buffer */
  unsigned char _nbuf[1];	/* guarantee a getc() buffer */

  /* separate buffer for fgetline() when line crosses buffer boundary */
  struct __sbuf _lb;	/* buffer for fgetline() */

  /* Unix stdio files get aligned to block boundaries on fseek() */
  int	_blksize;	/* stat.st_blksize (may be != _bf._size) */
  _off_t _offset;	/* current lseek offset */

#ifndef _REENT_SMALL
  struct _reent *_data;	/* Here for binary compatibility? Remove? */
#endif

#ifndef __SINGLE_THREAD__
  _flock_t _lock;	/* for thread-safety locking */
#endif
  _mbstate_t _mbstate;	/* for wide char stdio functions. */
  int   _flags2;        /* for future use */
};

#endif /* !__CYGWIN__ */

#ifdef __CUSTOM_FILE_IO__

/* Get custom _FILE definition.  */
#include <sys/custom_file.h>

#else /* !__CUSTOM_FILE_IO__ */
/* Cygwin must use __sFILE64 for backward compatibility, even though
   it's not defining __LARGE64_FILES anymore.  It also has to make
   sure the name is the same to satisfy C++ name mangling.  Overloading
   _fpos64_t just fixes a build problem.  The _seek64 function is
   actually never used without __LARGE64_FILES being defined. */
#if defined (__LARGE64_FILES) || defined (__CYGWIN__)
#ifdef __CYGWIN__
#define _fpos64_t _fpos_t
#endif

struct __sFILE64 {
  unsigned char *_p;	/* current position in (some) buffer */
  int	_r;		/* read space left for getc() */
  int	_w;		/* write space left for putc() */
  short	_flags;		/* flags, below; this FILE is free if 0 */
  short	_file;		/* fileno, if Unix descriptor, else -1 */
  struct __sbuf _bf;	/* the buffer (at least 1 byte, if !NULL) */
  int	_lbfsize;	/* 0 or -_bf._size, for inline putc */

  struct _reent *_data;

  /* operations */
  void *	_cookie;	/* cookie passed to io functions */

  _READ_WRITE_RETURN_TYPE (*_read) (struct _reent *, void *,
					   char *, _READ_WRITE_BUFSIZE_TYPE);
  _READ_WRITE_RETURN_TYPE (*_write) (struct _reent *, void *,
					    const char *,
					    _READ_WRITE_BUFSIZE_TYPE);
  _fpos_t (*_seek) (struct _reent *, void *, _fpos_t, int);
  int (*_close) (struct _reent *, void *);

  /* separate buffer for long sequences of ungetc() */
  struct __sbuf _ub;	/* ungetc buffer */
  unsigned char *_up;	/* saved _p when _p is doing ungetc data */
  int	_ur;		/* saved _r when _r is counting ungetc data */

  /* tricks to meet minimum requirements even when malloc() fails */
  unsigned char _ubuf[3];	/* guarantee an ungetc() buffer */
  unsigned char _nbuf[1];	/* guarantee a getc() buffer */

  /* separate buffer for fgetline() when line crosses buffer boundary */
  struct __sbuf _lb;	/* buffer for fgetline() */

  /* Unix stdio files get aligned to block boundaries on fseek() */
  int	_blksize;	/* stat.st_blksize (may be != _bf._size) */
  int   _flags2;        /* for future use */

  _off64_t _offset;     /* current lseek offset */
  _fpos64_t (*_seek64) (struct _reent *, void *, _fpos64_t, int);

#ifndef __SINGLE_THREAD__
  _flock_t _lock;	/* for thread-safety locking */
#endif
  _mbstate_t _mbstate;	/* for wide char stdio functions. */
};
typedef struct __sFILE64 __FILE;
#else
typedef struct __sFILE   __FILE;
#endif /* __LARGE64_FILES */
#endif /* !__CUSTOM_FILE_IO__ */

extern __FILE __sf[3];

struct _glue
{
  struct _glue *_next;
  int _niobs;
  __FILE *_iobs;
};

extern struct _glue __sglue;

/*
 * rand48 family support
 *
 * Copyright (c) 1993 Martin Birgmeier
 * All rights reserved.
 *
 * You may redistribute unmodified or modified versions of this source
 * code provided that the above copyright notice and this and the
 * following conditions are retained.
 *
 * This software is provided ``as is'', and comes with no warranties
 * of any kind. I shall in no event be liable for anything that happens
 * to anyone/anything when using this software.
 */
#define        _RAND48_SEED_0  (0x330e)
#define        _RAND48_SEED_1  (0xabcd)
#define        _RAND48_SEED_2  (0x1234)
#define        _RAND48_MULT_0  (0xe66d)
#define        _RAND48_MULT_1  (0xdeec)
#define        _RAND48_MULT_2  (0x0005)
#define        _RAND48_ADD     (0x000b)
struct _rand48 {
  unsigned short _seed[3];
  unsigned short _mult[3];
  unsigned short _add;
#ifdef _REENT_SMALL
  /* Put this in here as well, for good luck.  */
  __extension__ unsigned long long _rand_next;
#endif
};

/* How big the some arrays are.  */
#define _REENT_EMERGENCY_SIZE 25
#define _REENT_ASCTIME_SIZE 26
#define _REENT_SIGNAL_SIZE 24

#ifndef _REENT_THREAD_LOCAL

#ifdef _REENT_BACKWARD_BINARY_COMPAT
#define _REENT_INIT_RESERVED_0 0,
#define _REENT_INIT_RESERVED_1 0,
#define _REENT_INIT_RESERVED_2 0,
#define _REENT_INIT_RESERVED_6_7 _NULL, _ATEXIT_INIT,
#define _REENT_INIT_RESERVED_8 {_NULL, 0, _NULL},
#else
#define _REENT_INIT_RESERVED_0 /* Nothing to initialize */
#define _REENT_INIT_RESERVED_1 /* Nothing to initialize */
#define _REENT_INIT_RESERVED_2 /* Nothing to initialize */
#define _REENT_INIT_RESERVED_6_7 /* Nothing to initialize */
#define _REENT_INIT_RESERVED_8 /* Nothing to initialize */
#endif

/*
 * struct _reent
 *
 * This structure contains the thread-local objects needed by the library.
 * It's raison d'etre is to facilitate threads by making all library routines
 * reentrant.  The exit handler support and FILE maintenance use dedicated
 * global objects which are not included in this structure.
 */

#ifdef _REENT_SMALL

struct _mprec
{
  /* used by mprec routines */
  struct _Bigint *_result;
  int _result_k;
  struct _Bigint *_p5s;
  struct _Bigint **_freelist;
};


struct _misc_reent
{
  /* miscellaneous reentrant data */
  char *_strtok_last;
  _mbstate_t _mblen_state;
  _mbstate_t _wctomb_state;
  _mbstate_t _mbtowc_state;
  char _l64a_buf[8];
  int _getdate_err;
  _mbstate_t _mbrlen_state;
  _mbstate_t _mbrtowc_state;
  _mbstate_t _mbsrtowcs_state;
  _mbstate_t _wcrtomb_state;
  _mbstate_t _wcsrtombs_state;
#ifdef _MB_CAPABLE
  char _getlocalename_l_buf[32 /*ENCODING + 1*/];
#endif
};

/* This version of _reent is laid out with "int"s in pairs, to help
 * ports with 16-bit int's but 32-bit pointers, align nicely.  */
struct _reent
{
  /* As an exception to the above put _errno first for binary
     compatibility with non _REENT_SMALL targets.  */
  int _errno;			/* local copy of errno */

  /* FILE is a big struct and may change over time.  To try to achieve binary
     compatibility with future versions, put stdin,stdout,stderr here.
     These are pointers into member __sf defined below.  */
  __FILE *_stdin, *_stdout, *_stderr;	/* XXX */

  int  _inc;			/* used by tmpnam */

  char *_emergency;

#ifdef _REENT_BACKWARD_BINARY_COMPAT
  int _reserved_0;
  int _reserved_1;
#endif
  struct __locale_t *_locale;/* per-thread locale */

  struct _mprec *_mp;

  void (*__cleanup) (struct _reent *);

  int _gamma_signgam;

  /* used by some fp conversion routines */
  int _cvtlen;			/* should be size_t */
  char *_cvtbuf;

  struct _rand48 *_r48;
  struct __tm *_localtime_buf;
  char *_asctime_buf;

  /* signal info */
  void (** _sig_func)(int);

#ifdef _REENT_BACKWARD_BINARY_COMPAT
  struct _atexit *_reserved_6;
  struct _atexit _reserved_7;
  struct _glue _reserved_8;
#endif

  __FILE *__sf;			        /* file descriptors */
  struct _misc_reent *_misc;            /* strtok, multibyte states */
  char *_signal_buf;                    /* strsignal */

  void *deviceData;
};

# define _REENT_INIT(var) \
  { 0, \
    &__sf[0], \
    &__sf[1], \
    &__sf[2], \
    0,   \
    _NULL, \
    _REENT_INIT_RESERVED_0 \
    _REENT_INIT_RESERVED_1 \
    _NULL, \
    _NULL, \
    _NULL, \
    0, \
    0, \
    _NULL, \
    _NULL, \
    _NULL, \
    _NULL, \
    _NULL, \
    _REENT_INIT_RESERVED_6_7 \
    _REENT_INIT_RESERVED_8 \
    _NULL, \
    _NULL, \
    _NULL \
  }

#define _REENT_INIT_PTR_ZEROED(var) \
  { (var)->_stdin = &__sf[0]; \
    (var)->_stdout = &__sf[1]; \
    (var)->_stderr = &__sf[2]; \
  }

/* Specify how to handle reent_check malloc failures. */
#ifdef _REENT_CHECK_VERIFY
#include <assert.h>
#define __reent_assert(x) ((x) ? (void)0 : __assert_func(__FILE__, __LINE__, (char *)0, "REENT malloc succeeded"))
#else
#define __reent_assert(x) ((void)0)
#endif

#ifdef __CUSTOM_FILE_IO__
#error Custom FILE I/O and _REENT_SMALL not currently supported.
#endif

/* Generic _REENT check macro.  */
#define _REENT_CHECK(var, what, type, size, init) do { \
  struct _reent *_r = (var); \
  if (_r->what == NULL) { \
    _r->what = (type)malloc(size); \
    __reent_assert(_r->what); \
    init; \
  } \
} while (0)

#define _REENT_CHECK_TM(var) \
  _REENT_CHECK(var, _localtime_buf, struct __tm *, sizeof *((var)->_localtime_buf), \
    /* nothing */)

#define _REENT_CHECK_ASCTIME_BUF(var) \
  _REENT_CHECK(var, _asctime_buf, char *, _REENT_ASCTIME_SIZE, \
    __builtin_memset((var)->_asctime_buf, 0, _REENT_ASCTIME_SIZE))

/* Handle the dynamically allocated rand48 structure. */
#define _REENT_INIT_RAND48(var) do { \
  struct _reent *_r = (var); \
  _r->_r48->_seed[0] = _RAND48_SEED_0; \
  _r->_r48->_seed[1] = _RAND48_SEED_1; \
  _r->_r48->_seed[2] = _RAND48_SEED_2; \
  _r->_r48->_mult[0] = _RAND48_MULT_0; \
  _r->_r48->_mult[1] = _RAND48_MULT_1; \
  _r->_r48->_mult[2] = _RAND48_MULT_2; \
  _r->_r48->_add = _RAND48_ADD; \
  _r->_r48->_rand_next = 1; \
} while (0)
#define _REENT_CHECK_RAND48(var) \
  _REENT_CHECK(var, _r48, struct _rand48 *, sizeof *((var)->_r48), _REENT_INIT_RAND48((var)))

#define _REENT_INIT_MP(var) do { \
  struct _reent *_r = (var); \
  _r->_mp->_result_k = 0; \
  _r->_mp->_result = _r->_mp->_p5s = _NULL; \
  _r->_mp->_freelist = _NULL; \
} while (0)
#define _REENT_CHECK_MP(var) \
  _REENT_CHECK(var, _mp, struct _mprec *, sizeof *((var)->_mp), _REENT_INIT_MP(var))

#define _REENT_CHECK_EMERGENCY(var) \
  _REENT_CHECK(var, _emergency, char *, _REENT_EMERGENCY_SIZE, /* nothing */)

#ifdef _MB_CAPABLE
#define __REENT_INIT_MISC_GETLOCALENAME_L _r->_misc->_getlocalename_l_buf[0] = '\0'
#else
#define __REENT_INIT_MISC_GETLOCALENAME_L
#endif
#define _REENT_INIT_MISC(var) do { \
  struct _reent *_r = (var); \
  _r->_misc->_strtok_last = _NULL; \
  _r->_misc->_mblen_state.__count = 0; \
  _r->_misc->_mblen_state.__value.__wch = 0; \
  _r->_misc->_wctomb_state.__count = 0; \
  _r->_misc->_wctomb_state.__value.__wch = 0; \
  _r->_misc->_mbtowc_state.__count = 0; \
  _r->_misc->_mbtowc_state.__value.__wch = 0; \
  _r->_misc->_mbrlen_state.__count = 0; \
  _r->_misc->_mbrlen_state.__value.__wch = 0; \
  _r->_misc->_mbrtowc_state.__count = 0; \
  _r->_misc->_mbrtowc_state.__value.__wch = 0; \
  _r->_misc->_mbsrtowcs_state.__count = 0; \
  _r->_misc->_mbsrtowcs_state.__value.__wch = 0; \
  _r->_misc->_wcrtomb_state.__count = 0; \
  _r->_misc->_wcrtomb_state.__value.__wch = 0; \
  _r->_misc->_wcsrtombs_state.__count = 0; \
  _r->_misc->_wcsrtombs_state.__value.__wch = 0; \
  __REENT_INIT_MISC_GETLOCALENAME_L; \
  _r->_misc->_l64a_buf[0] = '\0'; \
  _r->_misc->_getdate_err = 0; \
} while (0)
#define _REENT_CHECK_MISC(var) \
  _REENT_CHECK(var, _misc, struct _misc_reent *, sizeof *((var)->_misc), _REENT_INIT_MISC(var))

#define _REENT_CHECK_SIGNAL_BUF(var) \
  _REENT_CHECK(var, _signal_buf, char *, _REENT_SIGNAL_SIZE, /* nothing */)

#define _REENT_SIGNGAM(ptr)	((ptr)->_gamma_signgam)
#define _REENT_RAND_NEXT(ptr)	((ptr)->_r48->_rand_next)
#define _REENT_RAND48_SEED(ptr)	((ptr)->_r48->_seed)
#define _REENT_RAND48_MULT(ptr)	((ptr)->_r48->_mult)
#define _REENT_RAND48_ADD(ptr)	((ptr)->_r48->_add)
#define _REENT_MP_RESULT(ptr)	((ptr)->_mp->_result)
#define _REENT_MP_RESULT_K(ptr)	((ptr)->_mp->_result_k)
#define _REENT_MP_P5S(ptr)	((ptr)->_mp->_p5s)
#define _REENT_MP_FREELIST(ptr)	((ptr)->_mp->_freelist)
#define _REENT_ASCTIME_BUF(ptr)	((ptr)->_asctime_buf)
#define _REENT_TM(ptr)		((ptr)->_localtime_buf)
#define _REENT_STRTOK_LAST(ptr)	((ptr)->_misc->_strtok_last)
#define _REENT_MBLEN_STATE(ptr)	((ptr)->_misc->_mblen_state)
#define _REENT_MBTOWC_STATE(ptr)((ptr)->_misc->_mbtowc_state)
#define _REENT_WCTOMB_STATE(ptr)((ptr)->_misc->_wctomb_state)
#define _REENT_MBRLEN_STATE(ptr) ((ptr)->_misc->_mbrlen_state)
#define _REENT_MBRTOWC_STATE(ptr) ((ptr)->_misc->_mbrtowc_state)
#define _REENT_MBSRTOWCS_STATE(ptr) ((ptr)->_misc->_mbsrtowcs_state)
#define _REENT_WCRTOMB_STATE(ptr) ((ptr)->_misc->_wcrtomb_state)
#define _REENT_WCSRTOMBS_STATE(ptr) ((ptr)->_misc->_wcsrtombs_state)
#define _REENT_L64A_BUF(ptr)    ((ptr)->_misc->_l64a_buf)
#define _REENT_GETDATE_ERR_P(ptr) (&((ptr)->_misc->_getdate_err))
#define _REENT_GETLOCALENAME_L_BUF(ptr) ((ptr)->_misc->_getlocalename_l_buf)
#define _REENT_SIGNAL_BUF(ptr)  ((ptr)->_signal_buf)

#else /* !_REENT_SMALL */

struct _reent
{
  int _errno;			/* local copy of errno */

  /* FILE is a big struct and may change over time.  To try to achieve binary
     compatibility with future versions, put stdin,stdout,stderr here.
     These are pointers into member __sf defined below.  */
  __FILE *_stdin, *_stdout, *_stderr;

  int  _inc;			/* used by tmpnam */
  char _emergency[_REENT_EMERGENCY_SIZE];

#ifdef _REENT_BACKWARD_BINARY_COMPAT
  int _reserved_1;
#endif
  struct __locale_t *_locale;/* per-thread locale */

#ifdef _REENT_BACKWARD_BINARY_COMPAT
  int _reserved_0;
#endif

  void (*__cleanup) (struct _reent *);

  /* used by mprec routines */
  struct _Bigint *_result;
  int _result_k;
  struct _Bigint *_p5s;
  struct _Bigint **_freelist;

  /* used by some fp conversion routines */
  int _cvtlen;			/* should be size_t */
  char *_cvtbuf;

  union
    {
      struct
        {
#ifdef _REENT_BACKWARD_BINARY_COMPAT
          unsigned int _reserved_2;
#endif
          char * _strtok_last;
          char _asctime_buf[_REENT_ASCTIME_SIZE];
          struct __tm _localtime_buf;
          int _gamma_signgam;
          __extension__ unsigned long long _rand_next;
          struct _rand48 _r48;
          _mbstate_t _mblen_state;
          _mbstate_t _mbtowc_state;
          _mbstate_t _wctomb_state;
          char _l64a_buf[8];
          char _signal_buf[_REENT_SIGNAL_SIZE];
          int _getdate_err;
          _mbstate_t _mbrlen_state;
          _mbstate_t _mbrtowc_state;
          _mbstate_t _mbsrtowcs_state;
          _mbstate_t _wcrtomb_state;
          _mbstate_t _wcsrtombs_state;
	  int _h_errno;
#ifdef __CYGWIN__
          _mbstate_t _c8rtomb_state;
          _mbstate_t _c16rtomb_state;
          _mbstate_t _c32rtomb_state;
          _mbstate_t _mbrtoc8_state;
          _mbstate_t _mbrtoc16_state;
          _mbstate_t _mbrtoc32_state;
#endif
	  /* No errors are defined for getlocalename_l.  So we can't use
	     buffer allocation which might lead to an ENOMEM error.  We
	     have to use a "static" buffer here instead. */
	  char _getlocalename_l_buf[32 /* ENCODING_LEN + 1 */];
        } _reent;
#ifdef _REENT_BACKWARD_BINARY_COMPAT
      struct
        {
          unsigned char * _reserved_3[30];
          unsigned int _reserved_4[30];
        } _reserved_5;
#endif
    } _new;

#ifdef _REENT_BACKWARD_BINARY_COMPAT
  struct _atexit *_reserved_6;
  struct _atexit _reserved_7;
#endif

  /* signal info */
  void (**_sig_func)(int);

  void *deviceData;
};

#define _REENT_INIT(var) \
  { 0, \
    &__sf[0], \
    &__sf[1], \
    &__sf[2], \
    0, \
    "", \
    _REENT_INIT_RESERVED_1 \
    _NULL, \
    _REENT_INIT_RESERVED_0 \
    _NULL, \
    _NULL, \
    0, \
    _NULL, \
    _NULL, \
    0, \
    _NULL, \
    { \
      { \
        _REENT_INIT_RESERVED_2 \
        _NULL, \
        "", \
        {0, 0, 0, 0, 0, 0, 0, 0, 0}, \
        0, \
        1, \
        { \
          {_RAND48_SEED_0, _RAND48_SEED_1, _RAND48_SEED_2}, \
          {_RAND48_MULT_0, _RAND48_MULT_1, _RAND48_MULT_2}, \
          _RAND48_ADD \
        }, \
        {0, {0}}, \
        {0, {0}}, \
        {0, {0}}, \
        "", \
        "", \
        0, \
        {0, {0}}, \
        {0, {0}}, \
        {0, {0}}, \
        {0, {0}}, \
        {0, {0}} \
      } \
    }, \
    _REENT_INIT_RESERVED_6_7 \
    _NULL \
  }

#define _REENT_INIT_PTR_ZEROED(var) \
  { (var)->_stdin = &__sf[0]; \
    (var)->_stdout = &__sf[1]; \
    (var)->_stderr = &__sf[2]; \
    (var)->_new._reent._rand_next = 1; \
    (var)->_new._reent._r48._seed[0] = _RAND48_SEED_0; \
    (var)->_new._reent._r48._seed[1] = _RAND48_SEED_1; \
    (var)->_new._reent._r48._seed[2] = _RAND48_SEED_2; \
    (var)->_new._reent._r48._mult[0] = _RAND48_MULT_0; \
    (var)->_new._reent._r48._mult[1] = _RAND48_MULT_1; \
    (var)->_new._reent._r48._mult[2] = _RAND48_MULT_2; \
    (var)->_new._reent._r48._add = _RAND48_ADD; \
  }

#define _REENT_CHECK_RAND48(ptr)	/* nothing */
#define _REENT_CHECK_MP(ptr)		/* nothing */
#define _REENT_CHECK_TM(ptr)		/* nothing */
#define _REENT_CHECK_ASCTIME_BUF(ptr)	/* nothing */
#define _REENT_CHECK_EMERGENCY(ptr)	/* nothing */
#define _REENT_CHECK_MISC(ptr)	        /* nothing */
#define _REENT_CHECK_SIGNAL_BUF(ptr)	/* nothing */

#define _REENT_SIGNGAM(ptr)	((ptr)->_new._reent._gamma_signgam)
#define _REENT_RAND_NEXT(ptr)	((ptr)->_new._reent._rand_next)
#define _REENT_RAND48_SEED(ptr)	((ptr)->_new._reent._r48._seed)
#define _REENT_RAND48_MULT(ptr)	((ptr)->_new._reent._r48._mult)
#define _REENT_RAND48_ADD(ptr)	((ptr)->_new._reent._r48._add)
#define _REENT_MP_RESULT(ptr)	((ptr)->_result)
#define _REENT_MP_RESULT_K(ptr)	((ptr)->_result_k)
#define _REENT_MP_P5S(ptr)	((ptr)->_p5s)
#define _REENT_MP_FREELIST(ptr)	((ptr)->_freelist)
#define _REENT_ASCTIME_BUF(ptr)	((ptr)->_new._reent._asctime_buf)
#define _REENT_TM(ptr)		(&(ptr)->_new._reent._localtime_buf)
#define _REENT_STRTOK_LAST(ptr)	((ptr)->_new._reent._strtok_last)
#define _REENT_MBLEN_STATE(ptr)	((ptr)->_new._reent._mblen_state)
#define _REENT_MBTOWC_STATE(ptr)((ptr)->_new._reent._mbtowc_state)
#define _REENT_WCTOMB_STATE(ptr)((ptr)->_new._reent._wctomb_state)
#define _REENT_MBRLEN_STATE(ptr)((ptr)->_new._reent._mbrlen_state)
#define _REENT_MBRTOWC_STATE(ptr)((ptr)->_new._reent._mbrtowc_state)
#define _REENT_MBSRTOWCS_STATE(ptr)((ptr)->_new._reent._mbsrtowcs_state)
#define _REENT_WCRTOMB_STATE(ptr)((ptr)->_new._reent._wcrtomb_state)
#define _REENT_WCSRTOMBS_STATE(ptr)((ptr)->_new._reent._wcsrtombs_state)
#ifdef __CYGWIN__
#  define _REENT_C8RTOMB_STATE(ptr)((ptr)->_new._reent._c8rtomb_state)
#  define _REENT_C16RTOMB_STATE(ptr)((ptr)->_new._reent._c16rtomb_state)
#  define _REENT_C32RTOMB_STATE(ptr)((ptr)->_new._reent._c32rtomb_state)
#  define _REENT_MBRTOC8_STATE(ptr)((ptr)->_new._reent._mbrtoc8_state)
#  define _REENT_MBRTOC16_STATE(ptr)((ptr)->_new._reent._mbrtoc16_state)
#  define _REENT_MBRTOC32_STATE(ptr)((ptr)->_new._reent._mbrtoc32_state)
#endif
#define _REENT_L64A_BUF(ptr)    ((ptr)->_new._reent._l64a_buf)
#define _REENT_SIGNAL_BUF(ptr)  ((ptr)->_new._reent._signal_buf)
#define _REENT_GETDATE_ERR_P(ptr) (&((ptr)->_new._reent._getdate_err))
#define _REENT_GETLOCALENAME_L_BUF(ptr)((ptr)->_new._reent._getlocalename_l_buf)

#endif /* !_REENT_SMALL */

#define _REENT_CLEANUP(_ptr)	((_ptr)->__cleanup)
#define _REENT_CVTBUF(_ptr)	((_ptr)->_cvtbuf)
#define _REENT_CVTLEN(_ptr)	((_ptr)->_cvtlen)
#define _REENT_EMERGENCY(_ptr)	((_ptr)->_emergency)
#define _REENT_ERRNO(_ptr)	((_ptr)->_errno)
#define _REENT_INC(_ptr)	((_ptr)->_inc)
#define _REENT_LOCALE(_ptr)	((_ptr)->_locale)
#define _REENT_SIG_FUNC(_ptr)	((_ptr)->_sig_func)
#define _REENT_STDIN(_ptr)	((_ptr)->_stdin)
#define _REENT_STDOUT(_ptr)	((_ptr)->_stdout)
#define _REENT_STDERR(_ptr)	((_ptr)->_stderr)

#define _REENT_INIT_PTR(var) \
  { __builtin_memset((var), 0, sizeof(*(var))); \
    _REENT_INIT_PTR_ZEROED(var); \
  }

/*
 * All references to struct _reent are via this pointer.
 * Internally, newlib routines that need to reference it should use _REENT.
 */

#ifndef __ATTRIBUTE_IMPURE_PTR__
#define __ATTRIBUTE_IMPURE_PTR__
#endif

extern struct _reent *_impure_ptr __ATTRIBUTE_IMPURE_PTR__;

#ifndef __ATTRIBUTE_IMPURE_DATA__
#define __ATTRIBUTE_IMPURE_DATA__
#endif

extern struct _reent _impure_data __ATTRIBUTE_IMPURE_DATA__;

/* #define _REENT_ONLY define this to get only reentrant routines */

#if defined(__DYNAMIC_REENT__) && !defined(__SINGLE_THREAD__)
#ifndef __getreent
  struct _reent * __getreent (void);
#endif
# define _REENT (__getreent())
#else /* __SINGLE_THREAD__ || !__DYNAMIC_REENT__ */
# define _REENT _impure_ptr
#endif /* __SINGLE_THREAD__ || !__DYNAMIC_REENT__ */

#define _REENT_IS_NULL(_ptr) ((_ptr) == NULL)

#define _GLOBAL_REENT (&_impure_data)

#else /* _REENT_THREAD_LOCAL */

#define _REENT NULL
#define _GLOBAL_REENT NULL

/*
 * Since _REENT is defined as NULL, this macro ensures that calls to
 * CHECK_INIT() do not automatically fail.
 */
#define _REENT_IS_NULL(_ptr) 0

#define _REENT_CHECK_RAND48(ptr)	/* nothing */
#define _REENT_CHECK_MP(ptr)		/* nothing */
#define _REENT_CHECK_TM(ptr)		/* nothing */
#define _REENT_CHECK_ASCTIME_BUF(ptr)	/* nothing */
#define _REENT_CHECK_EMERGENCY(ptr)	/* nothing */
#define _REENT_CHECK_MISC(ptr)		/* nothing */
#define _REENT_CHECK_SIGNAL_BUF(ptr)	/* nothing */

extern _Thread_local char _tls_asctime_buf[_REENT_ASCTIME_SIZE];
#define _REENT_ASCTIME_BUF(_ptr) (_tls_asctime_buf)
extern _Thread_local char *_tls_cvtbuf;
#define _REENT_CVTBUF(_ptr) (_tls_cvtbuf)
extern _Thread_local int _tls_cvtlen;
#define _REENT_CVTLEN(_ptr) (_tls_cvtlen)
extern _Thread_local void (*_tls_cleanup)(struct _reent *);
#define _REENT_CLEANUP(_ptr) (_tls_cleanup)
extern _Thread_local char _tls_emergency[_REENT_EMERGENCY_SIZE];
#define _REENT_EMERGENCY(_ptr) (_tls_emergency)
extern _Thread_local int _tls_errno;
#define _REENT_ERRNO(_ptr) (_tls_errno)
extern _Thread_local int _tls_getdate_err;
#define _REENT_GETDATE_ERR_P(_ptr) (&_tls_getdate_err)
extern _Thread_local int _tls_inc;
#define _REENT_INC(_ptr) (_tls_inc)
extern _Thread_local char _tls_l64a_buf[8];
#define _REENT_L64A_BUF(_ptr) (_tls_l64a_buf)
extern _Thread_local struct __locale_t *_tls_locale;
#define _REENT_LOCALE(_ptr) (_tls_locale)
extern _Thread_local _mbstate_t _tls_mblen_state;
#define _REENT_GETLOCALENAME_L_BUF(ptr) (_tls_getlocalename_l_buf)
extern _Thread_local char _tls_getlocalename_l_buf[32 /*ENCODING + 1*/];
#define _REENT_MBLEN_STATE(_ptr) (_tls_mblen_state)
extern _Thread_local _mbstate_t _tls_mbrlen_state;
#define _REENT_MBRLEN_STATE(_ptr) (_tls_mbrlen_state)
extern _Thread_local _mbstate_t _tls_mbrtowc_state;
#define _REENT_MBRTOWC_STATE(_ptr) (_tls_mbrtowc_state)
extern _Thread_local _mbstate_t _tls_mbsrtowcs_state;
#define _REENT_MBSRTOWCS_STATE(_ptr) (_tls_mbsrtowcs_state)
extern _Thread_local _mbstate_t _tls_mbtowc_state;
#define _REENT_MBTOWC_STATE(_ptr) (_tls_mbtowc_state)
extern _Thread_local struct _Bigint **_tls_mp_freelist;
#define _REENT_MP_FREELIST(_ptr) (_tls_mp_freelist)
extern _Thread_local struct _Bigint *_tls_mp_p5s;
#define _REENT_MP_P5S(_ptr) (_tls_mp_p5s)
extern _Thread_local struct _Bigint *_tls_mp_result;
#define _REENT_MP_RESULT(_ptr) (_tls_mp_result)
extern _Thread_local int _tls_mp_result_k;
#define _REENT_MP_RESULT_K(_ptr) (_tls_mp_result_k)
extern _Thread_local unsigned short _tls_rand48_add;
#define _REENT_RAND48_ADD(_ptr) (_tls_rand48_add)
extern _Thread_local unsigned short _tls_rand48_mult[3];
#define _REENT_RAND48_MULT(_ptr) (_tls_rand48_mult)
extern _Thread_local unsigned short _tls_rand48_seed[3];
#define _REENT_RAND48_SEED(_ptr) (_tls_rand48_seed)
extern _Thread_local unsigned long long _tls_rand_next;
#define _REENT_RAND_NEXT(_ptr) (_tls_rand_next)
extern _Thread_local void (**_tls_sig_func)(int);
#define _REENT_SIG_FUNC(_ptr) (_tls_sig_func)
extern _Thread_local char _tls_signal_buf[_REENT_SIGNAL_SIZE];
#define _REENT_SIGNAL_BUF(_ptr) (_tls_signal_buf)
extern _Thread_local int _tls_gamma_signgam;
#define _REENT_SIGNGAM(_ptr) (_tls_gamma_signgam)
extern _Thread_local __FILE *_tls_stdin;
#define _REENT_STDIN(_ptr) (_tls_stdin)
extern _Thread_local __FILE *_tls_stdout;
#define _REENT_STDOUT(_ptr) (_tls_stdout)
extern _Thread_local __FILE *_tls_stderr;
#define _REENT_STDERR(_ptr) (_tls_stderr)
extern _Thread_local char *_tls_strtok_last;
#define _REENT_STRTOK_LAST(_ptr) (_tls_strtok_last)
extern _Thread_local struct __tm _tls_localtime_buf;
#define _REENT_TM(_ptr) (&_tls_localtime_buf)
extern _Thread_local _mbstate_t _tls_wctomb_state;
#define _REENT_WCTOMB_STATE(_ptr) (_tls_wctomb_state)
extern _Thread_local _mbstate_t _tls_wcrtomb_state;
#define _REENT_WCRTOMB_STATE(_ptr) (_tls_wcrtomb_state)
extern _Thread_local _mbstate_t _tls_wcsrtombs_state;
#define _REENT_WCSRTOMBS_STATE(_ptr) (_tls_wcsrtombs_state)

#endif /* !_REENT_THREAD_LOCAL */

/* This value is used in stdlib/misc.c.  reent/reent.c has to know it
   as well to make sure the freelist is correctly free'd.  Therefore
   we define it here, rather than in stdlib/misc.c, as before. */
#define _Kmax (sizeof (size_t) << 3)

extern struct _atexit *__atexit; /* points to head of LIFO stack */
extern struct _atexit __atexit0; /* one guaranteed table, required by ANSI */

extern void (*__stdio_exit_handler) (void);

void _reclaim_reent (struct _reent *);

extern int _fwalk_sglue (struct _reent *, int (*)(struct _reent *, __FILE *),
			 struct _glue *);

#ifdef __cplusplus
}
#endif
#endif /* _SYS_REENT_H_ */
