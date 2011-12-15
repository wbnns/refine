
#include <stdlib.h>
#include <stdio.h>

#include "ref_grid_import.h"

#define VTK_TETRA      (10)
#define VTK_HEXAHEDRON (12)
#define VTK_WEDGE      (13)
#define VTK_PYRAMID    (14)

/*
3-4 UGRID
| |\ 
| | 2
| |/
0-1
2-3 VTK
| |\ 
| | 4
| |/
1-0
 */

#define VTK_PYRAMID_ORDER(vtk_nodes)			\
  {							\
    REF_INT ugrid_nodes[5];				\
    ugrid_nodes[0] = (vtk_nodes)[0];			\
    ugrid_nodes[1] = (vtk_nodes)[1];			\
    ugrid_nodes[2] = (vtk_nodes)[2];			\
    ugrid_nodes[3] = (vtk_nodes)[3];			\
    ugrid_nodes[4] = (vtk_nodes)[4];			\
    (vtk_nodes)[0] = ugrid_nodes[1];			\
    (vtk_nodes)[1] = ugrid_nodes[0];			\
    (vtk_nodes)[2] = ugrid_nodes[3];			\
    (vtk_nodes)[3] = ugrid_nodes[4];			\
    (vtk_nodes)[4] = ugrid_nodes[2];			\
  }

REF_STATUS ref_grid_export_vtk( REF_GRID ref_grid, char *filename  )
{
  FILE *file;
  REF_NODE ref_node;
  REF_CELL ref_cell;
  REF_INT node;
  REF_INT *o2n;
  REF_INT ncell,size;
  REF_INT *nodes;
  REF_INT node_per, cell;
  REF_INT group;

  ref_node = ref_grid_node(ref_grid);

  file = fopen(filename,"w");
  if (NULL == (void *)file) printf("unable to open %s\n",filename);
  RNS(file, "unable to open file" );

  fprintf(file,"# vtk DataFile Version 2.0\n");
  fprintf(file,"ref_grid_export_vtk\n");
  fprintf(file,"ASCII\n");

  RSS( ref_node_compact( ref_node, &o2n), "compact" );

  fprintf(file,"DATASET UNSTRUCTURED_GRID\n");
  fprintf(file,"POINTS %d double\n",ref_node_n(ref_node));

  for ( node = 0; node < ref_node_max(ref_node); node++ )
    if ( REF_EMPTY != o2n[node] )
      {
	fprintf(file, " %.16e %.16e %.16e\n",
		ref_node_xyz(ref_node,0,node),
		ref_node_xyz(ref_node,1,node),
		ref_node_xyz(ref_node,2,node) ) ;
      }

  ncell = 0;
  each_ref_grid_ref_cell( ref_grid, group, ref_cell )
    ncell += ref_cell_n(ref_cell);

  size = 0;
  each_ref_grid_ref_cell( ref_grid, group, ref_cell )
    size += ref_cell_n(ref_cell)*(1+ref_cell_node_per(ref_cell));

  fprintf(file,"CELLS %d %d\n",ncell,size);

  each_ref_grid_ref_cell( ref_grid, group, ref_cell )
    {
      node_per = ref_cell_node_per(ref_cell);
      nodes = (REF_INT *) malloc( node_per * sizeof(REF_INT) );
      each_ref_cell_valid_cell_with_nodes( ref_cell, cell, nodes )
	{
	  fprintf(file," %d",node_per);
	  if ( 5 == node_per ) VTK_PYRAMID_ORDER(nodes);
	  for ( node = 0; node < node_per; node++ )
	    fprintf(file," %d",o2n[nodes[node]]);
	  fprintf(file,"\n");
	}
      free(nodes);
    }

  fprintf(file,"CELL_TYPES %d\n",ncell);

  ref_cell = ref_grid_tet(ref_grid);
  each_ref_cell_valid_cell( ref_cell, cell )
    fprintf(file," %d\n",VTK_TETRA);

  ref_cell = ref_grid_pyr(ref_grid);
  each_ref_cell_valid_cell( ref_cell, cell )
    fprintf(file," %d\n",VTK_PYRAMID);

  ref_cell = ref_grid_pri(ref_grid);
  each_ref_cell_valid_cell( ref_cell, cell )
    fprintf(file," %d\n",VTK_WEDGE);

  ref_cell = ref_grid_hex(ref_grid);
  each_ref_cell_valid_cell( ref_cell, cell )
    fprintf(file," %d\n",VTK_HEXAHEDRON);

  free(o2n);

  fclose(file);

  return REF_SUCCESS;
}

REF_STATUS ref_grid_export_fgrid( REF_GRID ref_grid, char *filename  )
{
  FILE *file;
  REF_NODE ref_node;
  REF_CELL ref_cell;
  REF_INT node;
  REF_INT *o2n;
  REF_INT nnode,ntri,ntet;
  REF_INT *nodes;
  REF_INT node_per, cell;
  REF_INT ixyz;

  ref_node = ref_grid_node(ref_grid);

  file = fopen(filename,"w");
  if (NULL == (void *)file) printf("unable to open %s\n",filename);
  RNS(file, "unable to open file" );

  nnode = ref_node_n(ref_node);

  ntri = ref_cell_n(ref_grid_tri(ref_grid));

  ntet = ref_cell_n(ref_grid_tet(ref_grid));

  fprintf(file,"%d %d %d\n",nnode,ntri,ntet);

  RSS( ref_node_compact( ref_node, &o2n), "compact" );

  for ( ixyz = 0 ; ixyz< 3; ixyz++)
    for ( node = 0; node < nnode; node++ )
      if ( REF_EMPTY != o2n[node] )
	fprintf(file, " %.16e\n", ref_node_xyz(ref_node,ixyz,node) ) ;

  ref_cell = ref_grid_tri(ref_grid);
  node_per = ref_cell_node_per(ref_cell);
  nodes = (REF_INT *) malloc( node_per * sizeof(REF_INT) );
  each_ref_cell_valid_cell_with_nodes( ref_cell, cell, nodes )
    {
      for ( node = 0; node < 3; node++ )
	fprintf(file," %d",o2n[nodes[node]]+1);
      fprintf(file,"\n");
    }
  each_ref_cell_valid_cell_with_nodes( ref_cell, cell, nodes )
    {
      fprintf(file," %d",nodes[3]);
      fprintf(file,"\n");
    }
  free(nodes);

  ref_cell = ref_grid_tet(ref_grid);
  node_per = ref_cell_node_per(ref_cell);
  nodes = (REF_INT *) malloc( node_per * sizeof(REF_INT) );
  each_ref_cell_valid_cell_with_nodes( ref_cell, cell, nodes )
    {
      for ( node = 0; node < node_per; node++ )
	fprintf(file," %d",o2n[nodes[node]]+1);
      fprintf(file,"\n");
    }
  free(nodes);

  free(o2n);

  fclose(file);

  return REF_SUCCESS;
}

REF_STATUS ref_grid_export_tec( REF_GRID ref_grid, char *filename  )
{
  FILE *file;
  REF_NODE ref_node;
  REF_CELL ref_cell;
  REF_INT node;
  REF_INT *o2n;
  REF_INT nface;
  REF_INT nodes[5];
  REF_INT cell;

  ref_node = ref_grid_node(ref_grid);

  RSS( ref_node_compact( ref_node, &o2n), "compact" );

  file = fopen(filename,"w");
  if (NULL == (void *)file) printf("unable to open %s\n",filename);
  RNS(file, "unable to open file" );

  fprintf(file, "title=\"tecplot refine geometry file\"\n");
  fprintf(file, "variables = \"x\" \"y\" \"z\"\n");

  nface = ref_cell_n(ref_grid_tri(ref_grid)) +
          ref_cell_n(ref_grid_qua(ref_grid)) ;

  if ( nface > 0 )
    {

      fprintf(file,
	  "zone t=surf, nodes=%d, elements=%d, datapacking=%s, zonetype=%s\n",
	  ref_node_n(ref_node), nface, "point", "fequadrilateral" );

      for ( node = 0; node < ref_node_max(ref_node); node++ )
	if ( REF_EMPTY != o2n[node] )
	  {
	    fprintf(file, " %.16e %.16e %.16e\n",
		    ref_node_xyz(ref_node,0,node),
		    ref_node_xyz(ref_node,1,node),
		    ref_node_xyz(ref_node,2,node) ) ;
	  }

      ref_cell = ref_grid_tri(ref_grid);
      each_ref_cell_valid_cell_with_nodes( ref_cell, cell, nodes )
	{
	  nodes[3] = nodes[2];
	  for ( node = 0; node < 4; node++ )
	    {
	      fprintf(file," %d",o2n[nodes[node]] + 1);
	    }
	  fprintf(file,"\n");
	}

      ref_cell = ref_grid_qua(ref_grid);
      each_ref_cell_valid_cell_with_nodes( ref_cell, cell, nodes )
	{
	  for ( node = 0; node < 4; node++ )
	    fprintf(file," %d",o2n[nodes[node]] + 1);
	  fprintf(file,"\n");
	}
    }

  free(o2n);

  fclose(file);

  return REF_SUCCESS;
}

REF_STATUS ref_grid_export_ugrid( REF_GRID ref_grid, char *filename  )
{
  FILE *file;
  REF_NODE ref_node;
  REF_CELL ref_cell;
  REF_INT node;
  REF_INT *o2n;
  REF_INT nnode,ntri,nqua,ntet,npyr,npri,nhex;
  REF_INT *nodes;
  REF_INT node_per, cell;
  REF_INT group;

  ref_node = ref_grid_node(ref_grid);

  file = fopen(filename,"w");
  if (NULL == (void *)file) printf("unable to open %s\n",filename);
  RNS(file, "unable to open file" );

  nnode = ref_node_n(ref_node);

  ntri = ref_cell_n(ref_grid_tri(ref_grid));
  nqua = ref_cell_n(ref_grid_tri(ref_grid));

  ntet = ref_cell_n(ref_grid_tet(ref_grid));
  npyr = ref_cell_n(ref_grid_pyr(ref_grid));
  npri = ref_cell_n(ref_grid_pri(ref_grid));
  nhex = ref_cell_n(ref_grid_hex(ref_grid));

  fprintf(file,"%d %d %d %d %d %d %d\n",nnode,ntri,nqua,ntet,npyr,npri,nhex);

  RSS( ref_node_compact( ref_node, &o2n), "compact" );

  for ( node = 0; node < nnode; node++ )
    if ( REF_EMPTY != o2n[node] )
      fprintf(file, " %.16e %.16e %.16e\n", 
	      ref_node_xyz(ref_node,0,node),
	      ref_node_xyz(ref_node,1,node),
	      ref_node_xyz(ref_node,2,node) ) ;

  ref_cell = ref_grid_tri(ref_grid);
  node_per = ref_cell_node_per(ref_cell);
  nodes = (REF_INT *) malloc( node_per * sizeof(REF_INT) );
  each_ref_cell_valid_cell_with_nodes( ref_cell, cell, nodes )
    {
      for ( node = 0; node < 3; node++ )
	fprintf(file," %d",o2n[nodes[node]]+1);
      fprintf(file,"\n");
    }
  free(nodes);
  ref_cell = ref_grid_qua(ref_grid);
  node_per = ref_cell_node_per(ref_cell);
  nodes = (REF_INT *) malloc( node_per * sizeof(REF_INT) );
  each_ref_cell_valid_cell_with_nodes( ref_cell, cell, nodes )
    {
      for ( node = 0; node < 4; node++ )
	fprintf(file," %d",o2n[nodes[node]]+1);
      fprintf(file,"\n");
    }
  free(nodes);

  ref_cell = ref_grid_tri(ref_grid);
  node_per = ref_cell_node_per(ref_cell);
  nodes = (REF_INT *) malloc( node_per * sizeof(REF_INT) );
  each_ref_cell_valid_cell_with_nodes( ref_cell, cell, nodes )
    {
      fprintf(file," %d",nodes[3]);
      fprintf(file,"\n");
    }
  free(nodes);
  ref_cell = ref_grid_qua(ref_grid);
  node_per = ref_cell_node_per(ref_cell);
  nodes = (REF_INT *) malloc( node_per * sizeof(REF_INT) );
  each_ref_cell_valid_cell_with_nodes( ref_cell, cell, nodes )
    {
      fprintf(file," %d",nodes[4]);
      fprintf(file,"\n");
    }
  free(nodes);

  each_ref_grid_ref_cell( ref_grid, group, ref_cell )
    {
      node_per = ref_cell_node_per(ref_cell);
      nodes = (REF_INT *) malloc( node_per * sizeof(REF_INT) );
      each_ref_cell_valid_cell_with_nodes( ref_cell, cell, nodes )
	{
	  for ( node = 0; node < node_per; node++ )
	    fprintf(file," %d",o2n[nodes[node]]+1);
	  fprintf(file,"\n");
	}
      free(nodes);
    }

  free(o2n);

  fclose(file);

  return REF_SUCCESS;
}

