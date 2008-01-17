
/* Michael A. Park
 * Computational Modeling & Simulation Branch
 * NASA Langley Research Center
 * Phone:(757)864-6604
 * Email:m.a.park@larc.nasa.gov
 */

/* $Id: conform_main.c 3827 2007-12-03 18:48:44Z mikepark $ */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <values.h>
#include "grid.h"
#include "gridmetric.h"
#include "gridswap.h"
#include "gridinsert.h"
#include "gridcad.h"
#include "gridshape.h"
#include "gridmove.h"
#include "gridmpi.h"
#include "gridfiller.h"
#include "gridedger.h"
#include "gridfacer.h"
#include "gridmove.h"
#include "interp.h"
#include "plan.h"

static  GridBool tecplotOutput = TRUE;
static  int iview=0;


#define PRINT_STATUS {printf("AR %12.5e err %12.5e Vol %10.6e\n", gridMinThawedAR(grid),interpTotalError(grid), gridMinVolume(grid)); fflush(stdout);}

#define DUMP_TEC if (tecplotOutput) { \
 iview++;printf("Frame %d\n",iview);\
 gridWriteTecplotSurfaceGeom(grid,NULL); \
  {int cell, nodes[4]; \
    for (cell=0;cell<gridMaxCell(grid);cell++) \
      if (grid==gridCell(grid, cell, nodes)) { \
        if ( -0.5 > gridAR(grid,nodes) ) \
          gridWriteTecplotCellGeom(grid,nodes,NULL,NULL); \
   }}; }

#define STATUS { \
  PRINT_STATUS; \
  printf("%10d %12.5e %%oct\n",gridNNode(grid),interpTotalError(grid)); \
}

void adapt_equal_swap(Grid *grid, double error_tol)
{
  Plan *plan;
  double target_cost;
  int cell, nodes[4], ranking;
  double cost,cost1;
  int report;

  target_cost = sqrt(error_tol*error_tol / ((double) gridNCell(grid) ));

  plan = planCreate( gridNCell(grid), MAX(gridNCell(grid)/10,1000) );
  for (cell=0;cell<gridMaxCell(grid);cell++) {
    if (grid==gridCell(grid, cell, nodes)) {
      cost = gridAR(grid,nodes);
      if ( cost > target_cost ) planAddItemWithPriority( plan, cell, cost );
    }
  }
  planDeriveRankingsFromPriorities( plan );
  report = 10; if (planSize(plan) > 100) report = planSize(plan)/10;
  for ( ranking=planSize(plan)-1; ranking>=0; ranking-- ) { 
    cell = planItemWithThisRanking(plan,ranking);
    if (grid==gridCell(grid, cell, nodes)) {
      cost = gridAR(grid,nodes);
      if ( cost > target_cost ) {
      if ( ( NULL != gridSwapFace( grid, NULL, nodes[1],nodes[2],nodes[3]) ) ||
	   ( NULL != gridSwapFace( grid, NULL, nodes[0],nodes[2],nodes[3]) ) ||
	   ( NULL != gridSwapFace( grid, NULL, nodes[0],nodes[1],nodes[3]) ) ||
	   ( NULL != gridSwapFace( grid, NULL, nodes[0],nodes[1],nodes[2]) ) ||
	   ( NULL != gridSwapEdge( grid, NULL, nodes[0], nodes[1] ) ) ||
	   ( NULL != gridSwapEdge( grid, NULL, nodes[0], nodes[2] ) ) ||
	   ( NULL != gridSwapEdge( grid, NULL, nodes[0], nodes[3] ) ) ||
	   ( NULL != gridSwapEdge( grid, NULL, nodes[1], nodes[2] ) ) ||
	   ( NULL != gridSwapEdge( grid, NULL, nodes[1], nodes[3] ) ) ||
	   ( NULL != gridSwapEdge( grid, NULL, nodes[2], nodes[3] ) ) ) {
      }
      }
      if ( ranking/report*report==ranking ){
	printf("rank %d cost %f \n",ranking,cost/target_cost);
	fflush(stdout);
      }
    }
  }
  planFree(plan);
}

void adapt_equal_smooth(Grid *grid, double error_tol)
{
  Plan *plan;
  int node, ranking;
  double cost, cost0;
  int report;
  double target_cost;

  target_cost = sqrt(error_tol*error_tol / ((double) gridNCell(grid) ));  

  printf("target %f\n",target_cost);

  plan = planCreate( gridNNode(grid), MAX(gridNNode(grid)/10,1000) );
  for (node=0;node<gridMaxNode(grid);node++) {
    if (gridValidNode(grid, node)) {
      gridNodeAR(grid,node,&cost);
      if ( cost > target_cost ) planAddItemWithPriority( plan, node, cost );
    }
  }
  planDeriveRankingsFromPriorities( plan );
  report = 10; if (planSize(plan) > 100) report = planSize(plan)/10;
  for ( ranking=planSize(plan)-1; ranking>=planSize(plan)/2; ranking-- ) { 
    node = planItemWithThisRanking(plan,ranking);
    gridNodeAR(grid,node,&cost0);
    gridSmoothNode(grid, node, TRUE );
    gridNodeAR(grid,node,&cost);
    if ( cost > cost0 )
      printf("rank %d %f smooth %f\n",ranking,cost0,cost/cost0);
    if ( ranking/report*report==ranking ){
      gridNodeAR(grid,node,&cost);
      printf("rank %d smooth %f\n",ranking,cost/target_cost);
      fflush(stdout);
    }
  }
  planFree(plan);
}
void adapt_equal_insert(Grid *grid, double error_tol )
{
  Plan *plan;
  double target_cost;
  int conn2node0[] = {0, 0, 0, 1, 1, 2};
  int conn2node1[] = {1, 2, 3, 2, 3, 3};
  int cell, nodes[4], oriented[4];
  int conn;
  int report, ranking, nnodeAdd;
  double cost;
  double xyz0[3], xyz1[3], xyz2[3], xyz3[3];
  double error_before, error_after;
  int best_conn;
  double best_cost;
  double ratio;
  int newnode;
  
  target_cost = sqrt(error_tol*error_tol / ((double) gridNCell(grid) ));

  printf("form plan\n");
  plan = planCreate( gridNCell(grid), MAX(gridNCell(grid)/10,1000) );
  for (cell=0;cell<gridMaxCell(grid);cell++) {
    if (grid==gridCell(grid, cell, nodes)) {
      cost = gridAR(grid,nodes);
      //	printf("cost %f %f\n",target_cost,cost);
      if ( cost > target_cost ) planAddItemWithPriority( plan, cell, 
							  cost/target_cost );
    }
  }
  planDeriveRankingsFromPriorities( plan );

  printf("split edges\n");
  nnodeAdd = 0;
  report = 10; if (planSize(plan) > 100) report = planSize(plan)/20;
  for ( ranking=planSize(plan)-1; ranking>=0; ranking-- ) { 
    cell = planItemWithThisRanking(plan,ranking);
    if (grid==gridCell(grid, cell, nodes)) {
      cost = gridAR(grid,nodes);
      if ( ranking/report*report==ranking ){
	printf("rank %d cost %f add %d\n",ranking,cost/target_cost,nnodeAdd);
	fflush(stdout);
      }
      if ( cost < target_cost ) continue;
      conn = 0;    
      oriented[0]=nodes[conn2node0[conn]];
      oriented[1]=nodes[conn2node1[conn]];
      gridOrient(grid, nodes, oriented );
      gridNodeXYZ(grid,oriented[0],xyz0);
      gridNodeXYZ(grid,oriented[1],xyz1);
      gridNodeXYZ(grid,oriented[2],xyz2);
      gridNodeXYZ(grid,oriented[3],xyz3);
      interpSplitImprovement( gridInterp(grid), xyz0, xyz1, xyz2, xyz3,
			      &error_before, &error_after );
      best_conn = conn;
      best_cost = error_after;
      for(conn=1;conn<6;conn++) {
	oriented[0]=nodes[conn2node0[conn]];
	oriented[1]=nodes[conn2node1[conn]];
	gridOrient(grid, nodes, oriented );
	gridNodeXYZ(grid,oriented[0],xyz0);
	gridNodeXYZ(grid,oriented[1],xyz1);
	gridNodeXYZ(grid,oriented[2],xyz2);
	gridNodeXYZ(grid,oriented[3],xyz3);
	interpSplitImprovement( gridInterp(grid), xyz0, xyz1, xyz2, xyz3,
				&error_before, &error_after );
	if ( error_after < best_cost )
	  {
	    best_conn = conn;      
	    best_cost = error_after;
	  }
      }
      if ( best_cost > error_before ) {
	printf("increase %f %f\n",best_cost > error_before);
	continue;
      }
      ratio = 0.5;
      newnode = gridSplitEdgeRatio( grid, NULL, 
				    nodes[conn2node0[best_conn]], 
				    nodes[conn2node1[best_conn]], 
				    ratio );
      if ( EMPTY != newnode ) {
	nnodeAdd++;
	//	gridSmoothNode(grid, newnode, TRUE );
      }
    }
  }
  planFree(plan);
}

void adapt_equal_remove(Grid *grid, double error_tol )
{
  Plan *plan;
  double target_cost;
  int nodes[2];
  int conn;
  int report, ranking, nnodeRemove;
  double cost;
  double currentCost, node0Cost, node1Cost;
  double ratio;
  
  target_cost = sqrt(error_tol*error_tol / ((double) gridNCell(grid) ));

  printf("form plan\n");
  gridCreateConn(grid);
  plan = planCreate( gridNConn(grid)/2, MAX(gridNConn(grid)/10,1000) );
  for(conn=0;conn<gridNConn(grid);conn++) {
    gridConn2Node(grid,conn,nodes);
    if (grid!=gridEquator(grid,nodes[0],nodes[1])) return;
    if (grid !=  gridGemMaxAR( grid, &cost )) return;
    if ( cost < 0.5*target_cost ) planAddItemWithPriority( plan, conn, 
						       cost/target_cost );
  }
  planDeriveRankingsFromPriorities( plan );

  printf("collapse edges %d\n",planSize(plan));
  nnodeRemove = 0;
  report = 10; if (planSize(plan) > 100) report = planSize(plan)/10;
  for ( ranking=0; ranking<planSize(plan); ranking++ ) { 
    conn = planItemWithThisRanking(plan,ranking);
    gridConn2Node(grid,conn,nodes);
    if (!gridValidNode(grid,nodes[0])) continue;
    if (!gridValidNode(grid,nodes[1])) continue;
    if (grid!=gridEquator(grid,nodes[0],nodes[1])) continue;
    if (grid !=  gridGemMaxAR( grid, &cost )) continue;
    if ( ranking/report*report==ranking ){
      printf("rank %d cost %f remove %d\n",
	     ranking,cost/target_cost,nnodeRemove);
      fflush(stdout);
    }
    if ( cost > target_cost ) continue;    
    if (grid != gridCollapseMaxCost(grid, nodes[0], nodes[1], 
				    &currentCost, &node0Cost, &node1Cost) )
	printf("%s: %d: %s: gridCollapseMaxCost not grid\n",
	       __FILE__, __LINE__, __func__ );
    if ( node0Cost > target_cost && node1Cost > target_cost ) continue;
    ratio = 1.0;
    if ( node0Cost < node1Cost ) ratio = 0.0;
    if ( grid == gridCollapseEdge(grid, NULL, nodes[0], nodes[1], ratio))
      nnodeRemove++;
  }
  planFree(plan);
  gridEraseConn(grid);
}


#ifdef PROE_MAIN
int GridEx_Main( int argc, char *argv[] )
#else
int main( int argc, char *argv[] )
#endif
{
  Grid *grid;
  char modeler[256];
  char project[256];
  char ref_input[256];
  char ref_output[256];
  double error_tol;

  int function_id=EMPTY;
  int order=2;

  int i;

  sprintf( modeler,    "Unknown" );
  sprintf( project,    "" );
  sprintf( ref_input,  "" );
  sprintf( ref_output, "" );

  i = 1;
  while( i < argc ) {
    if( strcmp(argv[i],"-p") == 0 ) {
      i++; sprintf( project, "%s", argv[i] );
      printf("-p argument %d: %s\n",i, project);
    } else if( strcmp(argv[i],"-r") == 0 ) {
      i++; sprintf( ref_input, "%s", argv[i] );
      printf("-r argument %d: %s\n",i, ref_input);
    } else if( strcmp(argv[i],"-o") == 0 ) {
      i++; sprintf( ref_output, "%s", argv[i] );
      printf("-o argument %d: %s\n",i, ref_output);
    } else if( strcmp(argv[i],"-h") == 0 ) {
      printf("Usage: flag value pairs:\n");
      printf(" -p input project name\n");
      printf(" -r input ref name\n");
      printf(" -o output ref name\n");
      return(0);
    } else {
      fprintf(stderr,"Argument \"%s %s\" Ignored\n",argv[i],argv[i+1]);
      i++;
    }
    i++;
  }
  
  if(strcmp(project,"")==0) sprintf( project, "../domain01.fgrid" );
  if(strcmp(ref_output,"")==0) sprintf( ref_output, "domain01_out" );

  if(!(strcmp(ref_input,"")==0)) {
    printf("running ref %s\n",ref_input);
    grid = gridImportRef( ref_input );
    if(strcmp(project,"")==0){
      printf("no geom specified.\n");
      printf("Done.\n");  
      return 0;
    }
    printf("loading geometry %s\n",project);
    gridGeomStartOnly( grid, project );
  }else{
    if(!(strcmp(project,"")==0)) {
      printf("running project %s\n",project);
      grid = gridImportFAST( project );
    }
  }

  printf("restart grid size: %d nodes %d faces %d cells.\n",
	 gridNNode(grid),gridNFace(grid),gridNCell(grid));

  if (!gridRightHandedBoundary(grid)) {
    printf("ERROR: loaded part does not have right handed boundaries\n");
    return 1;
  }

  printf("project all.\n");
  gridRobustProject(grid);

  if (!gridRightHandedBoundary(grid)) {
    printf("ERROR: projected part does not have right handed boundaries\n");
    return 1;
  }

  gridInterp(grid) = interpDirect( grid );
  interpTecplot( gridInterp(grid), "rho.t" );

  gridSetCostFunction(grid, gridCOST_FCN_INTERPOLATION );
  gridSetCostConstraint(grid, gridCOST_CNST_VOLUME );
  gridSetMinInsertCost(grid, 1.0e99 );

  STATUS;
  DUMP_TEC;

  for(i=0;i<3;i++) {
    error_tol = interpTotalError(grid);
    adapt_equal_swap (grid,error_tol);
    STATUS;
    //    adapt_equal_smooth(grid,error_tol);
    //    STATUS;
    adapt_equal_remove(grid,error_tol);
    STATUS;
    adapt_equal_insert(grid,error_tol);
    STATUS;
    DUMP_TEC;
  }

  if (!gridRightHandedBoundary(grid)) 
    printf("ERROR: modifed grid does not have right handed boundaries\n");

  gridExportFAST( grid, "output.fgrid" );
  
  printf("writing output ref %s\n",ref_output);
  gridExportRef( grid, ref_output );

  printf("Done.\n");
  
  return 0;
}

