
#ifndef REF_FORTRAN_H
#define REF_FORTRAN_H

#include "ref_defs.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define FC_FUNC_(name,NAME) name ## __
#endif

BEGIN_C_DECLORATION

REF_STATUS FC_FUNC_(ref_fortran_init,REF_FORTRAN_INIT)
     ( REF_INT *nnodes, REF_INT *nnodesg,
       REF_INT *l2g, REF_INT *part, REF_INT *partition, 
       REF_DBL *x, REF_DBL *y, REF_DBL *z );

REF_STATUS FC_FUNC_(ref_fortran_import_cell,REF_FORTRAN_IMPORT_CELL)
     (REF_INT *node_per_cell, REF_INT *ncell,REF_INT *c2n);

REF_STATUS FC_FUNC_(ref_fortran_import_boundary,REF_FORTRAN_IMPORT_BOUNDARY)
     (REF_INT *node_per_face, REF_INT *nface,
      REF_INT *f2n, REF_INT *boundary_index);

REF_STATUS FC_FUNC_(ref_fortran_import_metric,REF_FORTRAN_IMPORT_METRIC)
     (REF_INT *nnodes, REF_DBL *metric);

REF_STATUS FC_FUNC_(ref_fortran_import_ratio,REF_FORTRAN_IMPORT_RATIO)
     (REF_INT *nnodes, REF_DBL *ratio);

REF_STATUS FC_FUNC_(ref_fortran_viz,REF_FORTRAN_VIZ)( void );

REF_STATUS FC_FUNC_(ref_fortran_size_node,REF_FORTRAN_SIZE_NODE)
     (REF_INT *nnodes, REF_INT *nnodesg);

REF_STATUS FC_FUNC_(ref_fortran_free,REF_FORTRAN_FREE)( void );

END_C_DECLORATION

#endif /* REF_FORTRAN_H */
