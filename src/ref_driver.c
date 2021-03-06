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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ref_adapt.h"
#include "ref_adj.h"
#include "ref_cavity.h"
#include "ref_cell.h"
#include "ref_collapse.h"
#include "ref_dict.h"
#include "ref_edge.h"
#include "ref_egads.h"
#include "ref_export.h"
#include "ref_face.h"
#include "ref_fixture.h"
#include "ref_gather.h"
#include "ref_grid.h"
#include "ref_histogram.h"
#include "ref_import.h"
#include "ref_list.h"
#include "ref_malloc.h"
#include "ref_math.h"
#include "ref_matrix.h"
#include "ref_metric.h"
#include "ref_migrate.h"
#include "ref_mpi.h"
#include "ref_node.h"
#include "ref_part.h"
#include "ref_smooth.h"
#include "ref_sort.h"
#include "ref_split.h"
#include "ref_subdiv.h"
#include "ref_validation.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define VERSION "not available"
#endif

static void echo_argv(int argc, char *argv[]) {
  int pos;
  printf("\n");
  for (pos = 0; pos < argc; pos++) printf(" %s", argv[pos]);
  printf("\n\n");
}

int main(int argc, char *argv[]) {
  REF_MPI ref_mpi;
  REF_GRID ref_grid = NULL;
  int opt;
  int passes = 15, pass;
  REF_BOOL tecplot_movie = REF_FALSE;
  REF_BOOL curvature_metric = REF_TRUE;
  REF_BOOL curvature_constraint = REF_FALSE;
  REF_BOOL debug_verbose = REF_FALSE;
  char output_project[1004];
  char output_filename[1024];
  REF_INT ngeom;
  REF_BOOL all_done = REF_FALSE;
  REF_BOOL all_done0 = REF_FALSE;
  REF_BOOL all_done1 = REF_FALSE;

  RSS(ref_mpi_start(argc, argv), "start");
  RSS(ref_mpi_create(&ref_mpi), "make mpi");
  ref_mpi_stopwatch_start(ref_mpi);

  output_project[0] = '\0';

  if (ref_mpi_once(ref_mpi)) {
    printf("refine %s, on or after 1.9.2\n", VERSION);
    echo_argv(argc, argv);
  }

  while ((opt = getopt(argc, argv, "i:m:g:r:o:p:x:f:s:tdc")) != -1) {
    switch (opt) {
      case 'i':
        if (ref_mpi_para(ref_mpi)) {
          RSS(ref_part_by_extension(&ref_grid, ref_mpi, optarg), "part");
        } else {
          RSS(ref_import_by_extension(&ref_grid, ref_mpi, optarg), "import");
        }
        ref_mpi_stopwatch_stop(ref_mpi, "read grid");
        break;
      case 'g':
        RNS(ref_grid, "input grid must be loaded before geom");
        RSS(ref_egads_load(ref_grid_geom(ref_grid), optarg), "ld e");
        ref_mpi_stopwatch_stop(ref_mpi, "load egads");
        break;
      case 'r':
        RNS(ref_grid, "input grid must be loaded before geom");
        ref_geom_segments_per_radian_of_curvature(ref_grid_geom(ref_grid)) =
            atof(optarg);
        break;
      case 'm':
        RSS(ref_part_metric(ref_grid_node(ref_grid), optarg), "part m");
        curvature_metric = REF_FALSE;
        ref_mpi_stopwatch_stop(ref_mpi, "load metric");
        break;
      case 'o':
        snprintf(output_project, 1004, "%s", optarg);
        break;
      case 'p':
        RNS(ref_grid, "input grid must be loaded before geom");
        {
          REF_INT part_int = atoi(optarg);
          ref_grid_partitioner(ref_grid) = (REF_MIGRATE_PARTIONER)part_int;
        }
        break;
      case 'x':
        break;
      case 'f':
        break;
      case 's':
        passes = atoi(optarg);
        break;
      case 't':
        tecplot_movie = REF_TRUE;
        break;
      case 'd':
        debug_verbose = REF_TRUE;
        ref_mpi->debug = REF_TRUE;
        break;
      case '?':
      default:
        printf("parse error -%c\n", optopt);
        printf("usage: \n %s\n", argv[0]);
        printf("       [-i input_grid.ext]\n");
        printf("       [-g geometry.egads]\n");
        printf("       [-r segments_per_curvature_radian]\n");
        printf(
            "       [-m input_project.{solb,sol,metric}] (curvature metric "
            "when missing)\n");
        printf("       [-s number_of_adaptation_sweeps] default is 15\n");
        printf("       [-o output_project]\n");
        printf("       [-x export_grid.ext]\n");
        printf("       [-p partitioner_id]\n");
        printf("       [-t] tecplot movie\n");
        printf("       [-d] debug verbose\n");
        return 1;
    }
  }

  RNS(ref_grid, "input grid required");

  RSS(ref_gather_ngeom(ref_grid_node(ref_grid), ref_grid_geom(ref_grid),
                       REF_GEOM_FACE, &ngeom),
      "count ngeom");
  if (ngeom > 0) {
    if (!ref_geom_model_loaded(ref_grid_geom(ref_grid))) {
      if (ref_mpi_once(ref_mpi)) {
        printf("geometry association found, but EGADS not loaded.\n");
        printf("please provide the '-g project.egads' argument.\n");
      }
      THROW("geometry association without EGADS");
    }
    if (!ref_geom_manifold(ref_grid_geom(ref_grid))) {
      ref_grid_twod(ref_grid) = REF_TRUE;
      ref_grid_surf(ref_grid) = REF_TRUE;
    }
    curvature_constraint = REF_TRUE;
    if (ref_geom_model_loaded(ref_grid_geom(ref_grid))) {
      RSS(ref_egads_mark_jump_degen(ref_grid), "T and UV jumps; UV degen");
    }
    RSS(ref_geom_verify_topo(ref_grid), "geom topo");
    RSS(ref_geom_verify_param(ref_grid), "geom param");
    ref_mpi_stopwatch_stop(ref_mpi, "geom assoc");
  }

  if (ref_mpi_once(ref_mpi)) {
    printf("twod %d surf %d manifold %d\n", ref_grid_twod(ref_grid),
           ref_grid_surf(ref_grid), ref_geom_manifold(ref_grid_geom(ref_grid)));
  }

  if (ref_geom_curvature_unlimited(ref_grid_geom(ref_grid))) {
    RAS(!curvature_metric,
        "[-r seg_per_radian] must be greater than 0.1 or -m provided");
    curvature_constraint = REF_FALSE;
  }

  if (curvature_metric) {
    RSS(ref_metric_interpolated_curvature(ref_grid), "interp curve");
    ref_mpi_stopwatch_stop(ref_mpi, "curvature metric");
  } else {
    if (curvature_constraint) {
      RSS(ref_metric_constrain_curvature(ref_grid), "crv const");
      RSS(ref_validation_cell_volume(ref_grid), "vol");
      ref_mpi_stopwatch_stop(ref_mpi, "crv const");
    }
    RSS(ref_grid_cache_background(ref_grid), "cache");
    ref_mpi_stopwatch_stop(ref_mpi, "cache metric");
  }

  RSS(ref_gather_tec_movie_record_button(ref_grid_gather(ref_grid),
                                         tecplot_movie),
      "show time");

  RSS(ref_validation_cell_volume(ref_grid), "vol");
  RSS(ref_histogram_quality(ref_grid), "gram");
  RSS(ref_histogram_ratio(ref_grid), "gram");
  ref_mpi_stopwatch_stop(ref_mpi, "histogram");

  RSS(ref_migrate_to_balance(ref_grid), "balance");
  RSS(ref_grid_pack(ref_grid), "pack");
  ref_mpi_stopwatch_stop(ref_mpi, "pack");

  for (pass = 0; !all_done && pass < passes; pass++) {
    if (ref_mpi_once(ref_mpi))
      printf("\n pass %d of %d with %d ranks\n", pass + 1, passes,
             ref_mpi_n(ref_mpi));
    all_done1 = all_done0;
    RSS(ref_adapt_pass(ref_grid, &all_done0), "pass");
    all_done = all_done0 && all_done1 &&
               (!ref_grid_twod(ref_grid) || pass > MIN(5, passes));
    ref_mpi_stopwatch_stop(ref_mpi, "pass");
    if (curvature_metric) {
      RSS(ref_metric_interpolated_curvature(ref_grid), "interp curve");
      ref_mpi_stopwatch_stop(ref_mpi, "curvature metric");
    } else {
      RSS(ref_metric_synchronize(ref_grid), "sync with background");
      ref_mpi_stopwatch_stop(ref_mpi, "metric sync");
    }
    RSS(ref_validation_cell_volume(ref_grid), "vol");
    RSS(ref_histogram_quality(ref_grid), "gram");
    RSS(ref_histogram_ratio(ref_grid), "gram");
    ref_mpi_stopwatch_stop(ref_mpi, "histogram");
    RSS(ref_adapt_tattle_faces(ref_grid), "tattle");
    ref_mpi_stopwatch_stop(ref_grid_mpi(ref_grid), "tattle faces");
    RSS(ref_migrate_to_balance(ref_grid), "balance");
    RSS(ref_grid_pack(ref_grid), "pack");
    ref_mpi_stopwatch_stop(ref_mpi, "pack");
  }

  RSS(ref_node_implicit_global_from_local(ref_grid_node(ref_grid)),
      "implicit global");
  ref_mpi_stopwatch_stop(ref_mpi, "implicit global");

  RSS(ref_geom_verify_param(ref_grid), "final params");
  ref_mpi_stopwatch_stop(ref_mpi, "verify final params");

  if (strcmp(output_project, "") != 0) {
    snprintf(output_filename, 1024, "%s.meshb", output_project);
    if (ref_mpi_once(ref_mpi))
      printf("gather " REF_GLOB_FMT " nodes to %s\n",
             ref_node_n_global(ref_grid_node(ref_grid)), output_filename);
    RSS(ref_gather_by_extension(ref_grid, output_filename), "export");
    ref_mpi_stopwatch_stop(ref_mpi, "gather meshb");

    snprintf(output_filename, 1024, "%s.b8.ugrid", output_project);
    if (ref_mpi_once(ref_mpi)) printf("write/gather %s\n", output_filename);
    RSS(ref_gather_by_extension(ref_grid, output_filename), "b8.ugrid");
    ref_mpi_stopwatch_stop(ref_mpi, "gather b8.ugrid");

    snprintf(output_filename, 1024, "%s-final-metric.solb", output_project);
    if (ref_mpi_once(ref_mpi))
      printf("write/gather metric %s\n", output_filename);
    RSS(ref_gather_metric(ref_grid, output_filename), "met met");
    if (!ref_mpi_para(ref_mpi)) {
      snprintf(output_filename, 1024, "%s_surf.tec", output_project);
      if (ref_mpi_once(ref_mpi)) printf("export surf %s\n", output_filename);
      RSS(ref_export_tec_surf(ref_grid, output_filename), "surf tec");
      snprintf(output_filename, 1024, "%s_geom.tec", output_project);
      if (ref_mpi_once(ref_mpi)) printf("export geom %s\n", output_filename);
      RSS(ref_geom_tec(ref_grid, output_filename), "geom tec");
      ref_mpi_stopwatch_stop(ref_mpi, "tec");
    }
    if (debug_verbose && !ref_grid_twod(ref_grid) && !ref_mpi_para(ref_mpi)) {
      if (ref_mpi_once(ref_mpi))
        printf("export tec metric ellipse %s\n", output_project);
      RSS(ref_export_tec_metric_ellipse(ref_grid, output_project), "al");
      ref_mpi_stopwatch_stop(ref_mpi, "ellipse");
    }
  }

  /* export via -x grid.ext and -f final-surf.tec*/
  for (opt = 0; opt < argc - 1; opt++) {
    if (strcmp(argv[opt], "-x") == 0) {
      if (ref_mpi_para(ref_mpi)) {
        if (ref_mpi_once(ref_mpi))
          printf("gather " REF_GLOB_FMT " nodes to %s\n",
                 ref_node_n_global(ref_grid_node(ref_grid)), argv[opt + 1]);
        RSS(ref_gather_by_extension(ref_grid, argv[opt + 1]), "gather -x");
      } else {
        if (ref_mpi_once(ref_mpi))
          printf("export " REF_GLOB_FMT " nodes to %s\n",
                 ref_node_n_global(ref_grid_node(ref_grid)), argv[opt + 1]);
        RSS(ref_export_by_extension(ref_grid, argv[opt + 1]), "export -x");
      }
    }
    if (strcmp(argv[opt], "-f") == 0) {
      if (ref_mpi_once(ref_mpi))
        printf("gather final surface status %s\n", argv[opt + 1]);
      RSS(ref_gather_surf_status_tec(ref_grid, argv[opt + 1]), "gather -f");
    }
  }

  if (NULL != ref_grid) RSS(ref_grid_free(ref_grid), "free");

  ref_mpi_stopwatch_stop(ref_mpi, "done.");

  RSS(ref_mpi_free(ref_mpi), "mpi free");
  RSS(ref_mpi_stop(), "stop");

  return 0;
}
