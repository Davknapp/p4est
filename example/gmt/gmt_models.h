/*
  This file is part of p4est.
  p4est is a C library to manage a collection (a forest) of multiple
  connected adaptive quadtrees or octrees in parallel.

  Copyright (C) 2010 The University of Texas System
  Additional copyright (C) 2011 individual authors
  Written by Carsten Burstedde, Lucas C. Wilcox, and Tobin Isaac

  p4est is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  p4est is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with p4est; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef P4EST_GMT_MODELS_H
#define P4EST_GMT_MODELS_H

#include <p4est_geometry.h>

/** Used to free private model data. */
typedef void        (*p4est_gmt_destroy_data_t) (void *vmodel_data);

/** Check intersection of a quadrant with an object. */
typedef int         (*p4est_gmt_intersect_t) (p4est_topidx_t which_tree,
                                              const double coord[4],
                                              size_t m, void *vmodel);

/** General, application specific model data */
typedef struct p4est_gmt_model
{
  size_t              M;
  const char         *output_prefix;
  p4est_connectivity_t *conn;
  p4est_geometry_t   *model_geom;
  void               *model_data;

  /** When not NULL, free whatever is stored in model->model_data. */
  p4est_gmt_destroy_data_t destroy_data;

  /** Intersect a given rectangle with a model object. */
  p4est_gmt_intersect_t intersect;

  /** Private geometry data. */
  p4est_geometry_t    sgeom;
}
p4est_gmt_model_t;

/** Create a specific synthetic model */
p4est_gmt_model_t  *p4est_gmt_model_synth_new (int synthno, int resolution);

/** Parameter type for latitude-longitude model */
typedef struct p4est_gmt_model_latlong_params
{
  double              latitude[2];
  double              longitude[2];
  int                 resolution;
  const char         *load_filename;
  const char         *output_prefix;
}
p4est_gmt_model_latlong_params_t;

/** Create a specific latlong model */
p4est_gmt_model_t  *p4est_gmt_model_latlong_new
  (p4est_gmt_model_latlong_params_t * params);

/** Destroy model */
void                p4est_gmt_model_destroy (p4est_gmt_model_t * model);

/** representation of the GSHHG coastline product **/

// header for the gshhg binary (*.b) file 
// (taken from/see http://www.soest.hawaii.edu/pwessel/gshhg/ and README.txt for details)
typedef struct gshhg_header
{           /* Global Self-consistent Hierarchical High-resolution Shorelines */
  int id;   /* Unique polygon id number, starting at 0 */
  int n;    /* Number of points in this polygon */
  int flag; /* = level + version << 8 + greenwich << 16 + source << 24 + river << 25 */
  /* flag contains 5 items, as follows:
   * low byte:    level = flag & 255: Values: 1 land, 2 lake, 3 island_in_lake, 4
   * pond_in_island_in_lake 2nd byte:    version = (flag >> 8) & 255: Values: Should be 12 for GSHHG
   * release 12 (i.e., version 2.2) 3rd byte:    greenwich = (flag >> 16) & 1: Values: Greenwich is
   * 1 if Greenwich is crossed 4th byte:    source = (flag >> 24) & 1: Values: 0 = CIA WDBII, 1 =
   * WVS 4th byte:    river = (flag >> 25) & 1: Values: 0 = not set, 1 = river-lake and level = 2
   */
  double west, east, south, north; /* min/max extent in micro-degrees */
  int area;                        /* Area of polygon in 1/10 km^2 */
  int area_full;                   /* Area of original full-resolution polygon in 1/10 km^2 */
  int container; /* Id of container polygon that encloses this polygon (-1 if none) */
  int ancestor;  /* Id of ancestor polygon in the full resolution set that was the source of this
                    polygon (-1 if none) */
  int global_line_segment_index;
  double* pts;
}
gshhg_header_t;

typedef struct coastline_polygon_list
{
  gshhg_header_t* polygon_headers;
  int num_polygons;
  int num_line_segments;
  /** bounding box used to extract polygons */
  /** NOTE: this is not the bounding box of the included polygons, */ 
  /** but the bounding box of all included polygons intersects with/is inside this bounding box */
  double west, east, south, north; 
}
coastline_polygon_list_t;

/** reads the binary GSHHG data file (*.b) */
/** polygons for which their bounding box does not intersect with the bounding box lon[2] = {lon_min lon_max}, */
/** lat[2] = {lat_min lat_max} are discarded.  */
/** NOTE: only the bounding box is tested, not the polygon (there might by false positves)! */
coastline_polygon_list_t* read_land_polygons_bin(const char* filename,
                                                 double lon[2],
                                                 double lat[2]);

#endif /* P4EST_GMT_MODELS_H */
