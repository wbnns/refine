#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "ref_grid.h"
#include "ref_cell.h"
#include "ref_node.h"
#include "ref_adj.h"
#include "ref_metric.h"
#include "ref_sort.h"

#include "ref_subdiv.h"

#include "ref_test.h"

static REF_STATUS set_up_tet_for_subdiv( REF_SUBDIV *ref_subdiv_ptr )
{
  REF_GRID ref_grid;
  REF_INT nodes[4] = {0,1,2,3};
  REF_INT cell, node;

  TSS(ref_grid_create(&ref_grid),"create");

  TSS(ref_node_add( ref_grid_node(ref_grid), 0, &node),"n0");
  TSS(ref_node_add( ref_grid_node(ref_grid), 1, &node),"n1");
  TSS(ref_node_add( ref_grid_node(ref_grid), 2, &node),"n2");
  TSS(ref_node_add( ref_grid_node(ref_grid), 3, &node),"n3");

  TSS(ref_cell_add(ref_grid_tet(ref_grid),nodes,&cell),"add prism");
  TSS(ref_subdiv_create(ref_subdiv_ptr,ref_grid),"create");

  return REF_SUCCESS;
}

static REF_STATUS set_up_prism_for_subdiv( REF_SUBDIV *ref_subdiv_ptr )
{
  REF_GRID ref_grid;
  REF_INT nodes[6] = {0,1,2,3,4,5};
  REF_INT cell, node;

  TSS(ref_grid_create(&ref_grid),"create");

  TSS(ref_node_add( ref_grid_node(ref_grid), 0, &node),"n0");
  TSS(ref_node_add( ref_grid_node(ref_grid), 1, &node),"n1");
  TSS(ref_node_add( ref_grid_node(ref_grid), 2, &node),"n2");
  TSS(ref_node_add( ref_grid_node(ref_grid), 3, &node),"n3");
  TSS(ref_node_add( ref_grid_node(ref_grid), 4, &node),"n4");
  TSS(ref_node_add( ref_grid_node(ref_grid), 5, &node),"n5");

  TSS(ref_cell_add(ref_grid_pri(ref_grid),nodes,&cell),"add prism");
  TSS(ref_subdiv_create(ref_subdiv_ptr,ref_grid),"create");

  return REF_SUCCESS;
}

static REF_STATUS set_up_prism_with_boundary( REF_SUBDIV *ref_subdiv_ptr )
{
  REF_GRID ref_grid;
  REF_INT nodes[6] = {0,1,2,3,4,5};
  REF_INT cell, node;

  TSS(ref_grid_create(&ref_grid),"create");

  TSS(ref_node_add( ref_grid_node(ref_grid), 0, &node),"n0");
  TSS(ref_node_add( ref_grid_node(ref_grid), 1, &node),"n1");
  TSS(ref_node_add( ref_grid_node(ref_grid), 2, &node),"n2");
  TSS(ref_node_add( ref_grid_node(ref_grid), 3, &node),"n3");
  TSS(ref_node_add( ref_grid_node(ref_grid), 4, &node),"n4");
  TSS(ref_node_add( ref_grid_node(ref_grid), 5, &node),"n5");

  TSS(ref_cell_add(ref_grid_pri(ref_grid),nodes,&cell),"add prism");

  nodes[0] = 0; nodes[1] = 3; nodes[2] = 4; nodes[3] = 1; nodes[4] = 10;
  TSS(ref_cell_add(ref_grid_qua(ref_grid),nodes,&cell),"add quad");

  nodes[0] = 3; nodes[1] = 5; nodes[2] = 4; nodes[3] = 100;
  TSS(ref_cell_add(ref_grid_tri(ref_grid),nodes,&cell),"add tri");

  nodes[0] = 0; nodes[1] = 1; nodes[2] = 2; nodes[3] = 101;
  TSS(ref_cell_add(ref_grid_tri(ref_grid),nodes,&cell),"add tri");

  TSS(ref_subdiv_create(ref_subdiv_ptr,ref_grid),"create");

  return REF_SUCCESS;
}

static REF_STATUS tear_down( REF_SUBDIV ref_subdiv )
{
  REF_GRID ref_grid;

  ref_grid = ref_subdiv_grid(ref_subdiv);

  TSS(ref_subdiv_free(ref_subdiv),"free");

  TSS( ref_grid_free(ref_grid),"free" );

  return REF_SUCCESS;
}

int main( void )
{

  { /* mark and relax prism*/
    REF_SUBDIV ref_subdiv;
    TSS(set_up_prism_for_subdiv(&ref_subdiv),"set up");

    TES(0,ref_subdiv_mark(ref_subdiv,0),"init mark");
    TES(0,ref_subdiv_mark(ref_subdiv,6),"init mark");

    TSS(ref_subdiv_mark_to_split(ref_subdiv,0,1),"mark edge 0-1");

    TES(1,ref_subdiv_mark(ref_subdiv,0),"been marked");
    TES(0,ref_subdiv_mark(ref_subdiv,6),"not been marked");

    TSS(ref_subdiv_mark_relax(ref_subdiv),"relax");

    TES(1,ref_subdiv_mark(ref_subdiv,0),"been marked");
    TES(1,ref_subdiv_mark(ref_subdiv,6),"been relaxed");

    TSS( tear_down( ref_subdiv ), "tear down");
  }

  { /* mark and relax prism*/
    REF_SUBDIV ref_subdiv;
    TSS(set_up_prism_for_subdiv(&ref_subdiv),"set up");

    TSS(ref_subdiv_mark_to_split(ref_subdiv,0,1),"mark edge 0-1");
    TSS(ref_subdiv_mark_to_split(ref_subdiv,1,2),"mark edge 1-2");

    TES(0,ref_subdiv_mark(ref_subdiv,1),"no yet");
    TSS(ref_subdiv_mark_relax(ref_subdiv),"relax");
    TES(1,ref_subdiv_mark(ref_subdiv,1),"yet");

    TSS( tear_down( ref_subdiv ), "tear down");
  }

  { /* relax tet */
    REF_SUBDIV ref_subdiv;
    TSS(set_up_tet_for_subdiv(&ref_subdiv),"set up");

    TSS(ref_subdiv_mark_to_split(ref_subdiv,0,1),"mark edge 0-1");
    TSS(ref_subdiv_mark_to_split(ref_subdiv,1,2),"mark edge 1-2");

    TEIS(1,ref_subdiv_mark(ref_subdiv,0),"yet");
    TEIS(1,ref_subdiv_mark(ref_subdiv,3),"yet");

    TEIS(0,ref_subdiv_mark(ref_subdiv,1),"no yet");
    TSS(ref_subdiv_mark_relax(ref_subdiv),"relax");
    TEIS(1,ref_subdiv_mark(ref_subdiv,1),"yet");

    TEIS(0,ref_subdiv_mark(ref_subdiv,2),"no yet");
    TEIS(0,ref_subdiv_mark(ref_subdiv,4),"no yet");
    TEIS(0,ref_subdiv_mark(ref_subdiv,5),"no yet");

    TSS( tear_down( ref_subdiv ), "tear down");
  }

  { /* new nodes */
    REF_SUBDIV ref_subdiv;
    REF_NODE ref_node;
    TSS(set_up_prism_for_subdiv(&ref_subdiv),"set up");
    ref_node = ref_grid_node(ref_subdiv_grid(ref_subdiv));

    TSS(ref_subdiv_mark_to_split(ref_subdiv,0,1),"mark edge 0 0-1");
    TSS(ref_subdiv_mark_to_split(ref_subdiv,3,4),"mark edge 6 3-4");

    TEIS(6, ref_node_n(ref_node), "6 node prism" );
    TSS(ref_subdiv_new_node(ref_subdiv),"new nodes");

    TEIS(8, ref_node_n(ref_node), "two new nodes" );

    TEIS(6, ref_subdiv_node(ref_subdiv,0), "new 6" );
    TEIS(7, ref_subdiv_node(ref_subdiv,6), "new 6" );

    TSS( tear_down( ref_subdiv ), "tear down");
  }

  { /* split prism in two */
    REF_SUBDIV ref_subdiv;
    REF_GRID ref_grid;
    TSS(set_up_prism_for_subdiv(&ref_subdiv),"set up");
    ref_grid = ref_subdiv_grid(ref_subdiv);

    TSS(ref_subdiv_mark_to_split(ref_subdiv,0,1),"mark edge 0-1");

    TSS(ref_subdiv_mark_relax(ref_subdiv),"relax");
    TSS(ref_subdiv_new_node(ref_subdiv),"new nodes");

    TSS(ref_subdiv_split(ref_subdiv),"split");
    TEIS(2, ref_cell_n(ref_grid_pri(ref_grid)),"two");

    TSS( tear_down( ref_subdiv ), "tear down");
  }

  { /* split prism in two with bcs */
    REF_SUBDIV ref_subdiv;
    REF_GRID ref_grid;
    TSS(set_up_prism_with_boundary(&ref_subdiv),"set up");
    ref_grid = ref_subdiv_grid(ref_subdiv);

    TSS(ref_subdiv_mark_to_split(ref_subdiv,0,1),"mark edge 0-1");

    TSS(ref_subdiv_mark_relax(ref_subdiv),"relax");
    TSS(ref_subdiv_new_node(ref_subdiv),"new nodes");

    TSS(ref_subdiv_split(ref_subdiv),"split");
    TEIS(2, ref_cell_n(ref_grid_pri(ref_grid)),"two pri");

    TEIS(2, ref_cell_n(ref_grid_qua(ref_grid)),"two qua");

    TEIS(4, ref_cell_n(ref_grid_tri(ref_grid)),"four tri");

    TSS( tear_down( ref_subdiv ), "tear down");
  }

  SKIP_TEST("4 split")
  { /* split prism in four with bcs */
    REF_SUBDIV ref_subdiv;
    REF_GRID ref_grid;
    TSS(set_up_prism_with_boundary(&ref_subdiv),"set up");
    ref_grid = ref_subdiv_grid(ref_subdiv);

    TSS(ref_subdiv_mark_to_split(ref_subdiv,0,1),"mark edge 0-1");
    TSS(ref_subdiv_mark_to_split(ref_subdiv,1,2),"mark edge 1-2");
    TSS(ref_subdiv_mark_to_split(ref_subdiv,2,0),"mark edge 2-0");

    TSS(ref_subdiv_mark_relax(ref_subdiv),"relax");
    TSS(ref_subdiv_new_node(ref_subdiv),"new nodes");

    TSS(ref_subdiv_split(ref_subdiv),"split");
    TEIS(4, ref_cell_n(ref_grid_pri(ref_grid)),"two pri");

    TEIS(2, ref_cell_n(ref_grid_qua(ref_grid)),"two qua");

    TEIS(8, ref_cell_n(ref_grid_tri(ref_grid)),"four tri");

    TSS( tear_down( ref_subdiv ), "tear down");
  }

  return 0;
}
