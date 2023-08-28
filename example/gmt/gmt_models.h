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

typedef struct p4est_gmt_model
{
  int                 M;
  const char         *output_prefix;
  p4est_connectivity_t *conn;
  p4est_geometry_t   *model_geom;
  void               *model_data;

  /** Used only to free whatever is stored in model->model_data */
  void                (*destroy_data) (void *vmodel_data);
  int                 (*intersect) (int blockno, const double *coord, int m,
                                    void *vmodel);

  /* private data */
  p4est_geometry_t    sgeom;
}
p4est_gmt_model_t;

/** Create a specific synthetic model */
p4est_gmt_model_t  *p4est_gmt_model_synth_new (int synthno);

/** Parameter type for latitude-longitude model */
typedef struct p4est_gmt_model_latlong_params
{
  int                 latitude[2];
  int                 longitude[2];
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

#endif /* P4EST_GMT_MODELS_H */