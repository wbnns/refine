
#ifndef REF_EDGE_H
#define REF_EDGE_H

#include "ref_defs.h"

BEGIN_C_DECLORATION
typedef struct REF_EDGE_STRUCT REF_EDGE_STRUCT;
typedef REF_EDGE_STRUCT * REF_EDGE;
END_C_DECLORATION

#include "ref_grid.h"
#include "ref_adj.h"

BEGIN_C_DECLORATION

struct REF_EDGE_STRUCT {
  REF_INT n;
  REF_INT *e2n;
  REF_ADJ adj;
  REF_NODE node;
};

REF_STATUS ref_edge_create( REF_EDGE *ref_edge, REF_GRID ref_grid );
REF_STATUS ref_edge_free( REF_EDGE ref_edge );

#define ref_edge_n(ref_edge) ((ref_edge)->n)

#define ref_edge_e2n( ref_edge, node, edge ) \
  ((ref_edge)->e2n[node+2*edge])

#define ref_edge_adj( ref_edge ) ((ref_edge)->adj)
#define ref_edge_node(ref_edge) ((ref_edge)->node)

#define each_ref_edge( ref_edge, edge )					\
  for ( (edge) = 0 ;							\
	(edge) < ref_edge_n(ref_edge);					\
	(edge)++ )							\

#define each_edge_having_node( ref_edge, node, item, edge ) \
  each_ref_adj_node_item_with_ref( ref_edge_adj( ref_edge ), node, item, edge)

REF_STATUS ref_edge_with( REF_EDGE ref_edge, 
			  REF_INT node0, REF_INT node1,
			  REF_INT *edge );

REF_STATUS ref_edge_part( REF_EDGE ref_edge, REF_INT edge, 
			  REF_INT *part );

REF_STATUS ref_edge_ghost_int( REF_EDGE ref_edge, REF_INT *data );
REF_STATUS ref_edge_ghost_dbl( REF_EDGE ref_edge, REF_DBL *data, REF_INT dim );

REF_STATUS ref_edge_tec_int( REF_EDGE ref_edge,  REF_NODE ref_node,
			     const char *filename, REF_INT *data );
REF_STATUS ref_edge_tec_dbl( REF_EDGE ref_edge,  REF_NODE ref_node,
			     const char *filename, REF_DBL *data );

REF_STATUS ref_edge_tec_ratio( REF_EDGE ref_edge,  REF_NODE ref_node,
			       const char *root_filename );

END_C_DECLORATION

#endif /* REF_EDGE_H */