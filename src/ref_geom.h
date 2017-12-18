
/* Copyright 2014 United States Government as represented by the
 * Administrator of the National Aeronautics and Space
 * Administration. No copyright is claimed in the United States under
 * Title 17, U.S. Code.  All Other Rights Reserved.
 *
 * The refine platform is licensed under the Apache License, Version
 * 2.0 (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#ifndef REF_GEOM_H
#define REF_GEOM_H

#include "ref_defs.h"

BEGIN_C_DECLORATION
typedef struct REF_GEOM_STRUCT REF_GEOM_STRUCT;
typedef REF_GEOM_STRUCT * REF_GEOM;
  END_C_DECLORATION

#include "ref_grid.h"
#include "ref_adj.h"

BEGIN_C_DECLORATION

#define REF_GEOM_NODE ( 0 )
#define REF_GEOM_EDGE ( 1 )
#define REF_GEOM_FACE ( 2 )

  struct REF_GEOM_STRUCT {
    REF_INT n, max;
    REF_INT blank;
    REF_INT *descr;
    REF_DBL *param;
    REF_DBL *uv_area_sign;
    REF_DBL segments_per_radian_of_curvature;
    REF_ADJ ref_adj;
    REF_INT nnode, nedge, nface;
    void *context;
    void *solid;
    void *faces;
    void *edges;
    void *nodes;
    REF_INT cad_data_size;
    REF_BYTE *cad_data;
  };

#define ref_geom_n(ref_geom)     (( ref_geom )->n )
#define ref_geom_max(ref_geom)   (( ref_geom )->max )
#define ref_geom_blank(ref_geom) (( ref_geom )->blank )
#define ref_geom_adj(ref_geom)   (( ref_geom )->ref_adj )
#define ref_geom_cad_data(ref_geom) (( ref_geom )->cad_data )
#define ref_geom_cad_data_size(ref_geom) (( ref_geom )->cad_data_size )

#define ref_geom_model_loaded(ref_geom) (NULL != (void *)((ref_geom)->solid))

#define ref_geom_descr(ref_geom,attribute,geom) \
  (( ref_geom )->descr[( attribute )+3*( geom )] )

#define ref_geom_type(ref_geom,geom) \
  (ref_geom_descr((ref_geom),0,(geom)))
#define ref_geom_id(ref_geom,geom) \
  (ref_geom_descr((ref_geom),1,(geom)))
#define ref_geom_node(ref_geom,geom) \
  (ref_geom_descr((ref_geom),2,(geom)))

#define ref_geom_param(ref_geom,dimension,geom) \
  (( ref_geom )->param[( dimension )+2*( geom )] )

#define ref_geom_segments_per_radian_of_curvature(ref_geom) \
  (( ref_geom )->segments_per_radian_of_curvature )

#define each_ref_type( ref_geom, type )				\
  for ( ( type ) = 0;						\
        ( type ) < 3;						\
        ( type )++ )

#define each_ref_geom( ref_geom, geom )				\
  for ( ( geom ) = 0;						\
        ( geom ) < ref_geom_max(ref_geom);			\
        ( geom )++ )						\
    if ( REF_EMPTY != ref_geom_type( ref_geom, geom ) )

#define each_ref_geom_of( ref_geom, type, geom )		\
  for ( ( geom ) = 0;						\
        ( geom ) < ref_geom_max(ref_geom);			\
        ( geom )++ )						\
    if ( ( type ) == ref_geom_type( ref_geom, geom ) )

#define each_ref_geom_node( ref_geom, geom )			\
  for ( ( geom ) = 0;						\
        ( geom ) < ref_geom_max(ref_geom);			\
        ( geom )++ )						\
    if ( REF_GEOM_NODE == ref_geom_type( ref_geom, geom ) )

#define each_ref_geom_edge( ref_geom, geom )			\
  for ( ( geom ) = 0;						\
        ( geom ) < ref_geom_max(ref_geom);			\
        ( geom )++ )						\
    if ( REF_GEOM_EDGE == ref_geom_type( ref_geom, geom ) )

#define each_ref_geom_face( ref_geom, geom )			\
  for ( ( geom ) = 0;						\
        ( geom ) < ref_geom_max(ref_geom);			\
        ( geom )++ )						\
    if ( REF_GEOM_FACE == ref_geom_type( ref_geom, geom ) )

#define each_ref_geom_having_node( ref_geom, node, item, geom )	   	\
  each_ref_adj_node_item_with_ref( ( ref_geom )->ref_adj, node, item, geom)

REF_STATUS ref_geom_create( REF_GEOM *ref_geom );
REF_STATUS ref_geom_free( REF_GEOM ref_geom );

REF_STATUS ref_geom_deep_copy( REF_GEOM *ref_geom, REF_GEOM original );

REF_STATUS ref_geom_save( REF_GRID ref_grid, const char *filename );
REF_STATUS ref_geom_load( REF_GRID ref_grid, const char *filename );
REF_STATUS ref_geom_edge_faces( REF_GRID ref_grid, REF_INT **edge_faces );
REF_STATUS ref_geom_recon( REF_GRID ref_grid );
REF_STATUS ref_geom_infer_nedge_nface( REF_GRID ref_grid );

REF_STATUS ref_geom_uv_area( REF_GEOM ref_geom, REF_INT *nodes,
			     REF_DBL *uv_area );
REF_STATUS ref_geom_uv_area_sign( REF_GRID ref_grid, REF_INT id,
				  REF_DBL *sign );
REF_STATUS ref_geom_uv_area_report( REF_GRID ref_grid );

REF_STATUS ref_geom_inspect( REF_GEOM ref_geom );
REF_STATUS ref_geom_tattle( REF_GEOM ref_geom, REF_INT node );

REF_STATUS ref_geom_supported( REF_GEOM ref_geom, REF_INT node,
			       REF_BOOL *has_support );

REF_STATUS ref_geom_add( REF_GEOM ref_geom, REF_INT node,
			 REF_INT type, REF_INT id,
			 REF_DBL *param );

REF_STATUS ref_geom_remove( REF_GEOM ref_geom, REF_INT node,
			    REF_INT type, REF_INT id);
REF_STATUS ref_geom_remove_all( REF_GEOM ref_geom, REF_INT node );

REF_STATUS ref_geom_is_a( REF_GEOM ref_geom, REF_INT node,
			  REF_INT type, REF_BOOL *it_is );
REF_STATUS ref_geom_unique_id( REF_GEOM ref_geom, REF_INT node,
			       REF_INT type, REF_BOOL *id );

REF_STATUS ref_geom_find( REF_GEOM ref_geom, REF_INT node,
			  REF_INT type, REF_INT id,
			  REF_INT *geom);

REF_STATUS ref_geom_tuv( REF_GEOM ref_geom, REF_INT node,
			 REF_INT type, REF_INT id,
			 REF_DBL *param );

REF_STATUS ref_geom_add_between( REF_GRID ref_grid,
				 REF_INT node0, REF_INT node1, 
				 REF_INT new_node );
REF_STATUS ref_geom_support_between( REF_GRID ref_grid,
				     REF_INT node0, REF_INT node1, 
				     REF_BOOL *needs_support );

REF_STATUS ref_geom_constrain( REF_GRID ref_grid, REF_INT node );

REF_STATUS ref_geom_eval( REF_GEOM ref_geom, REF_INT geom,
			  REF_DBL *xyz, REF_DBL *dxyz_dtuv );
REF_STATUS ref_geom_inverse_eval( REF_GEOM ref_geom, REF_INT type, REF_INT id,
				  REF_DBL *xyz, REF_DBL *param );
REF_STATUS ref_geom_curvature( REF_GEOM ref_geom, REF_INT geom,
			       REF_DBL *kr, REF_DBL *r,
			       REF_DBL *ks, REF_DBL *s );
REF_STATUS ref_geom_rsn( REF_GEOM ref_geom, REF_INT geom,
			 REF_DBL *r, REF_DBL *s, REF_DBL *n );
REF_STATUS ref_geom_uv_rsn( REF_DBL *uv,
			    REF_DBL *r, REF_DBL *s, REF_DBL *n,
			    REF_DBL *drsduv );

REF_STATUS ref_geom_verify_param( REF_GRID ref_grid );
REF_STATUS ref_geom_verify_topo( REF_GRID ref_grid );

REF_STATUS ref_geom_egads_export( const char *filename );

REF_STATUS ref_geom_egads_load( REF_GEOM ref_geom, const char *filename );
REF_STATUS ref_geom_egads_diagonal( REF_GEOM ref_geom, REF_DBL *diag );
/* positive max_length is absolute, negative max_length is relative to bbox */
REF_STATUS ref_geom_egads_tess( REF_GRID ref_grid, REF_DBL max_length );

REF_STATUS ref_geom_degen_param( REF_GRID ref_grid );

REF_STATUS ref_geom_tetgen_volume( REF_GRID ref_grid );

REF_STATUS ref_geom_edge_tec_zone( REF_GRID ref_grid, REF_INT id, FILE *file );
REF_STATUS ref_geom_face_tec_zone( REF_GRID ref_grid, REF_INT id, FILE *file );
REF_STATUS ref_geom_norm_tec_zone( REF_GRID ref_grid, REF_INT id, FILE *file );
REF_STATUS ref_geom_tec( REF_GRID ref_grid, const char *filename );

REF_STATUS ref_geom_ghost( REF_GEOM ref_geom, REF_NODE ref_node );

REF_STATUS ref_geom_faceid_range( REF_GRID ref_grid, 
				  REF_INT *min_faceid, REF_INT *max_faceid );
REF_STATUS ref_geom_edgeid_range( REF_GRID ref_grid, 
				  REF_INT *min_edgeid, REF_INT *max_edgeid );

END_C_DECLORATION

#endif /* REF_GEOM_H */
