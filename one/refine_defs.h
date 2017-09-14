
/* Michael A. Park
 * Computational Modeling & Simulation Branch
 * NASA Langley Research Center
 * Phone:(757)864-6604
 * Email:m.a.park@larc.nasa.gov 
 */
  
#ifndef MASTER_HEADER_H
#define MASTER_HEADER_H

#ifdef __cplusplus
#  define BEGIN_C_DECLORATION extern "C" {
#  define END_C_DECLORATION }
#else
#  define BEGIN_C_DECLORATION
#  define END_C_DECLORATION
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

BEGIN_C_DECLORATION

#define EMPTY (-1)

#if !defined(ABS)
#define ABS(a)   ((a)>0?(a):-(a))
#endif

#if !defined(SIGN)
#define SIGN(a)   ( (a)==0 ? 0 : ((a)>0?1:-1) )
#endif

typedef int REF_STATUS;

#define REF_SUCCESS       (0)
#define REF_FAILURE       (1)

#define TSS(fcn,msg)							\
  {									\
    REF_STATUS code;							\
    code = (fcn);							\
    if (REF_SUCCESS != code){						\
      printf("%s: %d: %s: %d %s\n",__FILE__,__LINE__,__func__,code,(msg)); \
      return code;							\
    }									\
  }

#define SUPRESS_UNUSED_COMPILER_WARNING(ptr)                    \
  if (NULL == (&(ptr)+1)) printf("unused macro failed\n");

END_C_DECLORATION

#ifdef HAVE_SDK
#include <MeatLib/Common.h>
typedef GeoBool GridBool;
#else
/* lifted defs from the SDK/MeatLib/Common.h */

BEGIN_C_DECLORATION

typedef short   GridBool;

#undef TRUE
#undef FALSE
#define TRUE    ((GridBool)1)
#define FALSE   ((GridBool)0)

#if !defined(MIN)
#define MIN(a,b) ((a)<(b)?(a):(b)) 
#endif
#if !defined(MAX)
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

END_C_DECLORATION

#endif /* HAVE_SDK */
#endif /* MASTER_HEADER_H */