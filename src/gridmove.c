/* Michael A. Park
 * Computational Modeling & Simulation Branch
 * NASA Langley Research Center
 * Phone:(757)864-6604
 * Email:m.a.park@larc.nasa.gov 
 */
  
/* $Id$ */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "gridmove.h"

GridMove *gridmoveCreate( Grid *grid )
{
  int i;
  GridMove *gm;
  gm = malloc(sizeof(GridMove));
  gm->grid = grid;

  gridAttachPacker( grid, gridmovePack, gm );
  gridAttachNodeSorter( grid, gridmoveSortNode, gm );
  gridAttachReallocator( grid, gridmoveReallocator, gm );

  gm->displacement = malloc(3*gridMaxNode(grid)*sizeof(double));
  for (i=0;i<3*gridMaxNode(grid);i++) gm->displacement[i] = 0.0;

  gm->specified = malloc(gridMaxNode(grid)*sizeof(GridBool));
  for (i=0;i<gridMaxNode(grid);i++) gm->specified[i] = FALSE;

  return gm;
}

Grid *gridmoveGrid(GridMove *gm)
{
  return gm->grid;
}

void gridmoveFree(GridMove *gm)
{
  free(gm->displacement);
  gridDetachPacker( gm->grid );
  gridDetachNodeSorter( gm->grid );
  gridDetachReallocator( gm->grid );
  free(gm);
}

void gridmovePack(void *voidGridMove, 
		  int nnode, int maxnode, int *nodeo2n,
		  int ncell, int maxcell, int *cello2n,
		  int nface, int maxface, int *faceo2n,
		  int nedge, int maxedge, int *edgeo2n)
{
  GridMove *gm = (GridMove *)voidGridMove;
  int orignode, packnode;
  int ixyz;
  for ( orignode = 0 ; orignode < maxnode ; orignode++ ){
    packnode = nodeo2n[orignode];
    if (EMPTY!=packnode) {
      for ( ixyz = 0; ixyz < 3 ; ixyz++ ){
	gm->displacement[ixyz+3*packnode] = gm->displacement[ixyz+3*orignode];
      }
    }
  }
  for ( packnode=nnode ; packnode < maxnode ; packnode++ ){ 
    for ( ixyz = 0; ixyz < 3 ; ixyz++ ){
      gm->displacement[ixyz+3*packnode] = 0.0;
    }
  }

  for ( orignode = 0 ; orignode < maxnode ; orignode++ ){
    packnode = nodeo2n[orignode];
    if (EMPTY!=packnode) {
      gm->specified[packnode] = gm->specified[orignode];
    }
  }
  for ( packnode=nnode ; packnode < maxnode ; packnode++ ){ 
    gm->specified[packnode] = FALSE;
  }
}

void gridmoveSortNode(void *voidGridMove, int maxnode, int *o2n)
{
  GridMove *gm = (GridMove *)voidGridMove;
  int node, ixyz;
  double *temp_xyz;
  GridBool *temp_bool;

  temp_xyz = malloc( maxnode * sizeof(double) );
  for ( ixyz = 0; ixyz < 3 ; ixyz++ ){
    for ( node = 0 ; node < maxnode ; node++ )temp_xyz[node]=0.0;
    for ( node = 0 ; node < maxnode ; node++ ){
      if (EMPTY != o2n[node])
	temp_xyz[o2n[node]] = gm->displacement[ixyz+3*node];
    }
    for ( node = 0 ; node < maxnode ; node++ ){
      gm->displacement[ixyz+3*node] = temp_xyz[node];
    }
  }
  free(temp_xyz);

  temp_bool = malloc( maxnode * sizeof(GridBool) );
  for ( node = 0 ; node < maxnode ; node++ )temp_bool[node]=FALSE;
  for ( node = 0 ; node < maxnode ; node++ ){
    if (EMPTY != o2n[node])
      temp_bool[o2n[node]] = gm->specified[node];
  }
  for ( node = 0 ; node < maxnode ; node++ ){
    gm->specified[node] = temp_bool[node];
  }
  free(temp_bool);
}

void gridmoveReallocator(void *voidGridMove, int reallocType, 
			 int lastSize, int newSize)
{
  GridMove *gm = (GridMove *)voidGridMove;
  int i;
  if (gridREALLOC_NODE == reallocType) {
    gm->displacement = realloc(gm->displacement, 3*newSize*sizeof(double));
    for (i=3*lastSize;i<3*newSize;i++) gm->displacement[i] = 0.0;
    gm->specified = realloc(gm->specified, newSize*sizeof(GridBool));
    for (i=lastSize;i<newSize;i++) gm->specified[i] = FALSE;
  }
}

GridMove *gridmoveDisplace(GridMove *gm, int node, double *displace)
{
  if (node < 0 || node >= gridMaxNode(gm->grid)) return NULL;
  gm->displacement[0+3*node] = displace[0];
  gm->displacement[1+3*node] = displace[1];
  gm->displacement[2+3*node] = displace[2];
  gm->specified[node] = TRUE;
  return gm;
}

GridMove *gridmoveDisplacement(GridMove *gm, int node, double *displacement)
{
  if (node < 0 || node >= gridMaxNode(gm->grid)) return NULL;
  displacement[0] = gm->displacement[0+3*node];
  displacement[1] = gm->displacement[1+3*node];
  displacement[2] = gm->displacement[2+3*node];
  return gm;
}

GridBool gridmoveSpecified(GridMove *gm, int node)
{
  if (node < 0 || node >= gridMaxNode(gm->grid)) return FALSE;
  return gm->specified[node];
}

GridMove *gridmoveMove(GridMove *gm)
{
  return gm;
}

void grimoveAddEdgeToC2E(Grid *grid, int *c2e, int edge, int node0, int node1)
{
  int iedge;
  int cell, nodes[4];
  int edge2node0[6] = {0, 0, 0, 1, 1, 2};
  int edge2node1[6] = {1, 2, 3, 2, 3, 3};
  int local0, local1;
  AdjIterator it;

  for ( it = adjFirst(gridCellAdj(grid),node0); 
	adjValid(it); 
	it = adjNext(it) ) {
    cell = adjItem(it);
    gridCell( grid, cell, nodes );
    for(iedge=0;iedge<6;iedge++) {
      local0 = nodes[edge2node0[iedge]];
      local1 = nodes[edge2node1[iedge]];
      if ( MIN(local0, local1) == MIN(node0,node1) &&
	   MAX(local0, local1) == MAX(node0,node1) ) {
	c2e[iedge+6*cell] = edge;
      } 
    }
  }

}

GridMove *gridmoveSprings(GridMove *gm, int *nsprings, int **springs)
{
  Grid *grid = gridmoveGrid(gm);
  int cell, edge, nedge;
  int nodes[4];
  int edge2node0[6] = {0, 0, 0, 1, 1, 2};
  int edge2node1[6] = {1, 2, 3, 2, 3, 3};
  int node0, node1;
  int *c2e;

  c2e = malloc(6*gridMaxCell(grid)*sizeof(int));
  for(cell=0;cell<6*gridMaxCell(grid);cell++) c2e[cell] = EMPTY;

  nedge = 0;
  for(cell=0;cell<gridMaxCell(grid);cell++) {
    if (grid == gridCell(grid,cell,nodes)) {
      for(edge=0;edge<6;edge++) {
	if ( EMPTY == c2e[edge+6*cell] ) {
	  c2e[edge+6*cell] = nedge;
	  grimoveAddEdgeToC2E(grid, c2e, nedge,
			      nodes[edge2node0[edge]], nodes[edge2node1[edge]]);
	  nedge++;
	}
      }
    }
  }
  *nsprings = nedge;
  *springs = malloc(2*nedge*sizeof(int));
  for(cell=0;cell<gridMaxCell(grid);cell++) {
    if (grid == gridCell(grid,cell,nodes)) {
      for(edge=0;edge<6;edge++) {
	if (EMPTY!=c2e[edge+6*cell]) {
	  node0 = MIN(nodes[edge2node0[edge]],nodes[edge2node1[edge]]);
	  node1 = MAX(nodes[edge2node0[edge]],nodes[edge2node1[edge]]);
	  (*springs)[0+2*c2e[edge+6*cell]] = node0;
	  (*springs)[1+2*c2e[edge+6*cell]] = node1;
	}else{
	  printf("ERROR: %s: %d: c2e EMPTY.\n",__FILE__,__LINE__);
	}
      }
    }
  }

  free(c2e);
  return gm;
}
