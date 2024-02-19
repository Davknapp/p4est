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

#include "gmt_models.h"

static void
model_set_geom (p4est_gmt_model_t * model,
                const char *name, p4est_geometry_X_t X)
{
  model->sgeom.name = name;
  model->sgeom.user = model;
  model->sgeom.X = X;
  model->sgeom.destroy = NULL;
  model->model_geom = &model->sgeom;
}

typedef struct p4est_gmt_model_synth
{
  int                 synthno;
  int                 resolution;
  size_t              num_points;
  double             *points;
}
p4est_gmt_model_synth_t;

static void
model_synth_destroy_data (void *vmodel_data)
{
  p4est_gmt_model_synth_t *sdata = (p4est_gmt_model_synth_t *) vmodel_data;
  P4EST_FREE (sdata->points);
  P4EST_FREE (sdata);
}

static int
model_synth_intersect (p4est_topidx_t which_tree, const double coord[4],
                       size_t m, void *vmodel)
{
  p4est_gmt_model_t  *model = (p4est_gmt_model_t *) vmodel;
  p4est_gmt_model_synth_t *sdata;
  const double       *pco;
  double              hx, hy;

  P4EST_ASSERT (model != NULL);
  P4EST_ASSERT (m < model->M);
  sdata = (p4est_gmt_model_synth_t *) model->model_data;
  P4EST_ASSERT (sdata != NULL && sdata->points != NULL);
  pco = sdata->points + 2 * m;
  P4EST_ASSERT (sdata->resolution >= 0);

  /* In this model we have only one tree, the unit square. */
  P4EST_ASSERT (which_tree == 0);

  /* Rectangle coordinates are in [0, 1] for the numbered reference tree and
   * stored as { lower left x, lower left y, upper right x, upper right y }. */

  /* We do not refine if target resolution is reached. */
  hx = coord[2] - coord[0];
  hy = coord[3] - coord[1];
  if (SC_MAX (hx, hy) <= pow (.5, sdata->resolution)) {
    return 0;
  }

  /* In this synthetic example the point IS the object.  There are no lines. */
  if ((coord[0] <= pco[0] && pco[0] <= coord[2]) &&
      (coord[1] <= pco[1] && pco[1] <= coord[3])) {
    return 1;
  }

  /* We have exhausted the refinement criteria. */
  return 0;
}

static void
model_synth_geom_X (p4est_geometry_t * geom, p4est_topidx_t which_tree,
                    const double abc[3], double xyz[3])
{
  /* In this model we have only one tree, the unit square. */
  P4EST_ASSERT (which_tree == 0);

  /* We work with the unit square as physical space. */
  memcpy (xyz, abc, 3 * sizeof (double));
}

p4est_gmt_model_t  *
p4est_gmt_model_synth_new (int synthno, int resolution)
{
  p4est_gmt_model_t  *model = P4EST_ALLOC_ZERO (p4est_gmt_model_t, 1);
  p4est_gmt_model_synth_t *sdata = NULL;
  double             *p;

  /* initalize model */
  switch (synthno) {
  case 0:
    model->output_prefix = "triangle";
    model->conn = p4est_connectivity_new_unitsquare ();
    model->model_data = sdata = P4EST_ALLOC (p4est_gmt_model_synth_t, 1);
    sdata->synthno = synthno;
    sdata->resolution = resolution;
    sdata->num_points = model->M = 3;
    p = sdata->points = P4EST_ALLOC (double, 6);
    p[0] = 0.2;
    p[1] = 0.1;
    p[2] = 0.7;
    p[3] = 0.4;
    p[4] = 0.5;
    p[5] = 0.8;
    model->destroy_data = model_synth_destroy_data;
    model->intersect = model_synth_intersect;
    model_set_geom (model, model->output_prefix, model_synth_geom_X);
    break;
    /* possibly add more cases that work with polygon segments */
  default:
    SC_ABORT_NOT_REACHED ();
  }

  /* return initialized model */
  P4EST_ASSERT (sdata != NULL);
  sdata->synthno = synthno;
  return model;
}

static int
model_latlong_intersect (p4est_topidx_t which_tree, const double coord[4],
                         size_t m, void *vmodel)
{
#ifdef P4EST_ENABLE_DEBUG
  p4est_gmt_model_t  *model = (p4est_gmt_model_t *) vmodel;
#endif

  P4EST_ASSERT (model != NULL);
  P4EST_ASSERT (m < model->M);

  /* Rectangle coordinates are in [0, 1] for the numbered reference tree and
   * stored as { lower left x, lower left y, upper right x, upper right y }. */

  return 0;
}

static void
model_latlong_geom_X (p4est_geometry_t * geom, p4est_topidx_t which_tree,
                      const double abc[3], double xyz[3])
{
#if LATLONG_DATA_HAS_BEEN_PROGRAMMED
  p4est_gmt_model_t  *model = (p4est_gmt_model_t *) geom->user;

  /* put the parameters latitude, longitude into the model data */
  longitude =
    ((typecast into gmt lanlong model data *) model->model_data)->longitude;
  latitude = ...;

  xyz[0] = longitude[0] + (longitude->[1] - longitude[0]) * abc[0];
  xyz[1] = latitude[0] + (latitude->[1] - latitude[0]) * abc[1];
#else
  xyz[0] = abc[0];
  xyz[1] = abc[1];
#endif
  xyz[2] = 0.;
}

static void
p4est_gmt_model_latlong_destroy (void *model_data)
{
  coastline_polygon_list_t *coast_poly_data =
    (coastline_polygon_list_t *) model_data;
  for (int i = 0; i < coast_poly_data->num_polygons; i++) {
    free (coast_poly_data->polygon_headers[i].pts);
  }
  free (coast_poly_data->polygon_headers);
  free (coast_poly_data);
}

p4est_gmt_model_t  *
p4est_gmt_model_latlong_new (p4est_gmt_model_latlong_params_t * params)
{
  p4est_gmt_model_t  *model = P4EST_ALLOC_ZERO (p4est_gmt_model_t, 1);

  /* the latlong models live on the unit square as reference domain */
  model->conn = p4est_connectivity_new_unitsquare ();

  /* load model properties */
  coastline_polygon_list_t *coast_poly =
    read_land_polygons_bin (params->load_filename, params->longitude,
                            params->latitude);
  model->model_data = coast_poly;

  /* set virtual functions */
  model->intersect = model_latlong_intersect;
  model->destroy_data = p4est_gmt_model_latlong_destroy;        /* <- needs to free whatever is in model_data */

  /* setup input/output parameters */
  model->output_prefix = params->output_prefix; /*< Prefix defaults to NULL */
  model_set_geom (model, params->output_prefix, model_latlong_geom_X);

  /* the model is ready */
  model->M = coast_poly->num_line_segments;
  return model;
}

/** are two bounding boxes overlapping ? */
int
is_overlapping (double x1min,
                double x1max,
                double y1min,
                double y1max,
                double x2min, double x2max, double y2min, double y2max)
{
  return ((x1min < x2max) && (x2min < x1max) && (y1min < y2max)
          && (y2min < y1max));
}

/* convert endianess from big to little */
int
to_little_end (int i)
{
  unsigned char      *data = (unsigned char *) &(i);
  int                 j =
    (data[3] << 0) | (data[2] << 8) | (data[1] << 16) | ((unsigned) data[0] <<
                                                         24);
  return j;
}

coastline_polygon_list_t *
read_land_polygons_bin (const char *filename, double lon[2], double lat[2])
{
  printf ("Reading land poygons in BIN format from %s\n", filename);

  FILE               *infile = fopen (filename, "r");
  int                 num_polygons = 0;
  int                 num_line_segments = 0;
  int                 global_line_segment_index = 0;

  gshhg_header_t     *all_used =
    (gshhg_header_t *) malloc (500000 * sizeof (gshhg_header_t));
  while (!feof (infile)) {
    gshhg_header_t      poly_header;
    int                 h[11];
    int                 s = fread (h, 11, sizeof (int), infile);
    if (s > 0) {
      poly_header.id = to_little_end (h[0]);
      poly_header.n = to_little_end (h[1]);
      poly_header.flag = to_little_end (h[2]);
      poly_header.west = to_little_end (h[3]) / 1.0e6;
      poly_header.east = to_little_end (h[4]) / 1.0e6;
      poly_header.south = to_little_end (h[5]) / 1.0e6;
      poly_header.north = to_little_end (h[6]) / 1.0e6;
      poly_header.area = to_little_end (h[7]);
      poly_header.area_full = to_little_end (h[8]);
      poly_header.container = to_little_end (h[9]);
      poly_header.ancestor = to_little_end (h[10]);
      poly_header.global_line_segment_index = -1;

      // printf("Id %d with %d pts\n", poly_header.id, poly_header.n);

      int                *pts =
        (int *) malloc (2 * poly_header.n * sizeof (int));
      double             *coord_list =
        (double *) malloc (2 * poly_header.n * sizeof (double));
      fread (pts, 2 * poly_header.n, sizeof (int), infile);

      for (int i = 0; i < poly_header.n; i++) {
        coord_list[2 * i] = to_little_end (pts[2 * i]) / 1.0e6;
        if (coord_list[2 * i] > 180.0) {
          coord_list[2 * i] -= 360.0;
        }
        coord_list[2 * i + 1] = to_little_end (pts[2 * i + 1]) / 1.0e6;
      }
      poly_header.pts = coord_list;
      int                 level = poly_header.flag & 255;
      if ((level == 1) && (poly_header.container == -1)) {
        // ceck if bbox of polygon overlaps with region of intrest
        if (is_overlapping
            (poly_header.west, poly_header.east, poly_header.south,
             poly_header.north, lon[0], lon[1], lat[0], lat[1])) {
          poly_header.global_line_segment_index = global_line_segment_index;
          all_used[num_polygons] = poly_header;
          num_polygons++;
          // polygons are closed, i.e. line segemnts are number of points - 1
          num_line_segments += poly_header.n - 1;
          // start index of global indexed segements
          global_line_segment_index += poly_header.n - 1;
        }
        else {
          free (coord_list);
        }
      }
      else {
        // printf("Level: %d und cont %d\n", level, poly_header.container);
        free (coord_list);
      }

      free (pts);
    }
  }
  printf ("We have %d polygons meeting the requests\n", num_polygons);
  coastline_polygon_list_t *pl_ptr =
    (coastline_polygon_list_t *) malloc (1 *
                                         sizeof (coastline_polygon_list_t));
  pl_ptr->polygon_headers = all_used;
  pl_ptr->num_polygons = num_polygons;
  pl_ptr->num_line_segments = num_line_segments;
  pl_ptr->west = lon[0];
  pl_ptr->east = lon[1];
  pl_ptr->south = lat[0];
  pl_ptr->north = lat[1];
  fclose (infile);
  return pl_ptr;
}

typedef struct p4est_gmt_model_sphere
{
  int                 resolution;
  size_t              num_geodesics;
  p4est_gmt_sphere_geoseg_t *geodesics;
} p4est_gmt_model_sphere_t;

static void
model_sphere_destroy_data (void *vmodel_data)
{
  p4est_gmt_model_sphere_t *sdata = (p4est_gmt_model_sphere_t *) vmodel_data;
  P4EST_FREE (sdata->geodesics);
  P4EST_FREE (sdata);
}

/** Returns 1 if the line segments (p0 to p1) and (p2 to p3) intersect, otherwise 0 */
static int
lines_intersect (double p0_x, double p0_y, double p1_x, double p1_y,
                 double p2_x, double p2_y, double p3_x, double p3_y)
{
  /* We solve the matrix equation (p1-p0, p2-p3) (s, t)^T = (p2-p0),
   * by inverting the matrix (p1-p0, p2-p3). */

  /* Precompute reused values for efficiency */
  double              s1_x, s1_y, s2_x, s2_y;
  s1_x = p1_x - p0_x;
  s1_y = p1_y - p0_y;
  s2_x = p3_x - p2_x;
  s2_y = p3_y - p2_y;

  /* Compute line intersection */
  double              s, t;
  s =
    (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / (-s2_x * s1_y +
                                                      s1_x * s2_y);
  t =
    (s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y +
                                                     s1_x * s2_y);

  /* Check intersection lies on relevant segment */
  if (s >= 0 && s <= 1 && t >= 0 && t <= 1) {
    return 1;
  }

  return 0;
}

/** Returns 1 if the given geodesic intersects the given rectangle and 0 otherwise.
 *
 * \param[in] which_tree  tree id inside forest
 * \param[in] coord       rectangle for intersection checking. Rectangle coordinates
 *                        are in [0, 1] for the numbered reference tree and stored as
 *                        { lower left x, lower left y, upper right x, upper right y }.
 * \param[in] m           index of the geodesic we are checking
 * \param[in] vmodel      spherical model
 *
 */
static int
model_sphere_intersect (p4est_topidx_t which_tree, const double coord[4],
                        size_t m, void *vmodel)
{
  p4est_gmt_model_t  *model = (p4est_gmt_model_t *) vmodel;
  p4est_gmt_model_sphere_t *sdata;
  const p4est_gmt_sphere_geoseg_t *pco; /* mth geodesic segment */
  double              hx, hy;   /* width, height */

  P4EST_ASSERT (model != NULL);
  P4EST_ASSERT (m < model->M);
  sdata = (p4est_gmt_model_sphere_t *) model->model_data;
  P4EST_ASSERT (sdata != NULL && sdata->geodesics != NULL);
  pco = sdata->geodesics + m;
  P4EST_ASSERT (sdata->resolution >= 0);

  /* In this model we have 6 trees */
  P4EST_ASSERT (which_tree >= 0 && which_tree <= 5);

  /* Check the segment is on the relevant tree */
  if (pco->which_tree != which_tree) {
    return 0;
  }

  /* We do not refine if target resolution is reached. */
  hx = coord[2] - coord[0];
  hy = coord[3] - coord[1];
  if (SC_MAX (hx, hy) <= pow (.5, sdata->resolution)) {
    return 0;
  }

  /* Check if the line segment L between p1 and p2 intersects the edges of
   * the rectangle.
   */

  /* Check if L intersects the bottom edge of rectangle */
  if (lines_intersect
      (pco->p1x, pco->p1y, pco->p2x, pco->p2y, coord[0], coord[1], coord[2],
       coord[1])) {
    return 1;
  }
  /* Check if L intersects the top edge of rectangle */
  if (lines_intersect
      (pco->p1x, pco->p1y, pco->p2x, pco->p2y, coord[0], coord[3], coord[2],
       coord[3])) {
    return 1;
  }
  /* Check if L intersects the left edge of rectangle */
  if (lines_intersect
      (pco->p1x, pco->p1y, pco->p2x, pco->p2y, coord[0], coord[1], coord[0],
       coord[3])) {
    return 1;
  }
  /* Check if L intersects the right edge of rectangle */
  if (lines_intersect
      (pco->p1x, pco->p1y, pco->p2x, pco->p2y, coord[2], coord[1], coord[2],
       coord[3])) {
    return 1;
  }

  /* Check if L is contained in the interior of rectangle.
   * Since we have already ruled out intersections it suffices
   * to check if one of the endpoints of L is in the interior.
   */
  if (pco->p1x >= coord[0] && pco->p1x <= coord[2] && pco->p1y >= coord[1]
      && pco->p1y <= coord[3]) {
    return 1;
  }

  /* We have exhausted the refinement criteria. */
  return 0;
}

p4est_gmt_model_t  *
p4est_gmt_model_sphere_new (int resolution, const char *input,
                            const char *output_prefix, sc_MPI_Comm mpicomm)
{
  sc_MPI_File         file_handle;
  p4est_gmt_model_t  *model;
  p4est_gmt_model_sphere_t *sdata = NULL;
  size_t              global_num_points = 0;
  size_t              local_num_points = 0;
  int                 local_int_bytes;
  int                 rank, num_procs;
  int                 mpiret;
  int                 mpival;
  int                 mpiall;
  int                 ocount;
  int                 mpireslen;
  char                mpierrstr[sc_MPI_MAX_ERROR_STRING];
  sc_MPI_Offset       mpi_offset;
  sc_MPI_Status       status;
  const char         *count_mismatch_message =
    "This should only occur when attempting to read "
    "beyond the bounds of the input file. "
    "If you correctly specified your input as the "
    "output of the preprocessing script then we "
    "expect that this error should never occur.\n";

  /* Get rank and number of processes */
  mpiret = sc_MPI_Comm_size (mpicomm, &num_procs);
  SC_CHECK_MPI (mpiret);
  mpiret = sc_MPI_Comm_rank (mpicomm, &rank);
  SC_CHECK_MPI (mpiret);

  /* clean initialization */
  mpival = sc_MPI_SUCCESS;
  mpiall = sc_MPI_SUCCESS;
  file_handle = sc_MPI_FILE_NULL;
  model = NULL;

  /* check for required parameters */
  if (input == NULL) {
    P4EST_GLOBAL_LERROR ("Sphere model expects non-NULL input filename.\n");
    P4EST_GLOBAL_LERROR ("Use the -F flag to set a filename.\n");
    return NULL;
  }

  /* collectively open file of precomputed geodesic segments */
  mpiret = sc_io_open (mpicomm, input, SC_IO_READ, sc_MPI_INFO_NULL,
                       &file_handle);

  /* check file open errors */
  if (mpiret != sc_MPI_SUCCESS) {
    mpiret = sc_MPI_Error_string (mpiret, mpierrstr, &mpireslen);
    SC_CHECK_MPI (mpiret);
    P4EST_GLOBAL_LERRORF ("Could not open input file: %s\n", input);
    P4EST_GLOBAL_LERRORF ("Error Code: %s\n", mpierrstr);
    P4EST_GLOBAL_LERROR ("Check you have run the preprocessing script.\n");
    P4EST_GLOBAL_LERROR ("Check you specified the input path correctly\n");
    return NULL;
  }

  if (rank == 0) {
    /* read the global number of points from file */
    mpiall = sc_io_read_at (file_handle, 0, &global_num_points,
                            sizeof (size_t), sc_MPI_BYTE, &ocount);

    /* check we read the expected number of bytes */
    if (mpiall == sc_MPI_SUCCESS && ocount != (int) sizeof (size_t)) {
      P4EST_GLOBAL_LERROR ("Count mismatch: reading number of points\n");
      P4EST_GLOBAL_LERROR (count_mismatch_message);
      mpiall = sc_MPI_ERR_OTHER;
    }
  }

  /* broadcast possible read errors */
  mpiret = sc_MPI_Bcast (&mpiall, 1, sc_MPI_INT, 0, mpicomm);
  SC_CHECK_MPI (mpiret);

  /* check read errors */
  if (mpiall != sc_MPI_SUCCESS) {
    mpiret = sc_MPI_Error_string (mpiall, mpierrstr, &mpireslen);
    SC_CHECK_MPI (mpiret);
    P4EST_GLOBAL_LERROR ("Error reading number of global points\n");
    P4EST_GLOBAL_LERRORF ("Error Code: %s\n", mpierrstr);
    /* cleanup on error */
    (void) sc_io_close (&file_handle);
    return NULL;
  }

  /* broadcast the global number of points */
  mpiret = sc_MPI_Bcast (&global_num_points, sizeof (size_t),
                         sc_MPI_BYTE, 0, mpicomm);
  SC_CHECK_MPI (mpiret);

  /* Check that the number of bytes being read does not overflow int.
   * We do this because by convention we record data size with a size_t,
   * whereas MPIIO uses int.
   * TODO: we could define a custom MPI datatype rather than sending
   * bytes to increase this maximum
   */
  if (global_num_points * sizeof (p4est_gmt_sphere_geoseg_t) >
      (size_t) INT_MAX) {
    P4EST_GLOBAL_LERRORF ("Global number of points %lld is too big.\n",
                          (long long) global_num_points);
    /* cleanup on error */
    (void) sc_io_close (&file_handle);
    return NULL;
  }

  /* set read offsets */
  /* note: these will be more relevant in the distributed version */
  mpi_offset = 0;
  local_num_points = global_num_points;
  local_int_bytes =
    (int) (local_num_points * sizeof (p4est_gmt_sphere_geoseg_t));
  P4EST_ASSERT (local_int_bytes >= 0);

  /* allocate model */
  model = P4EST_ALLOC_ZERO (p4est_gmt_model_t, 1);
  model->model_data = sdata = P4EST_ALLOC (p4est_gmt_model_sphere_t, 1);
  sdata->geodesics =
    P4EST_ALLOC (p4est_gmt_sphere_geoseg_t, local_num_points);

  /* Set final geodesic count */
  sdata->num_geodesics = model->M = local_num_points;

  /* Assign resolution, intersector and destructor */
  sdata->resolution = resolution;
  model->destroy_data = model_sphere_destroy_data;
  model->intersect = model_sphere_intersect;

  /* Assign connectivity */
  model->conn = p4est_connectivity_new_cubed ();

  /* Assign geometry */
  /* Note: the following allocates memory externally, rather than in sgeom. */
  model->model_geom = p4est_geometry_new_sphere2d (model->conn, 1.0);
  model->geom_allocated = 1;

  /* set default output prefix */
  if (output_prefix == NULL) {
    model->output_prefix = "sphere";
  }
  else {
    model->output_prefix = output_prefix;
  }

  /* each mpi process reads its data for its own offset */
  mpival = sc_io_read_at_all (file_handle, mpi_offset + sizeof (size_t),
                              sdata->geodesics,
                              local_int_bytes, sc_MPI_BYTE, &ocount);

  /* check we read the expected number of bytes */
  if (mpival == sc_MPI_SUCCESS && ocount != local_int_bytes) {
    mpival = sc_MPI_ERR_OTHER;
  }

  /* communicate any read errors */
  /* receive errors from predecessor */
  if (rank != 0) {
    mpiret =
      sc_MPI_Recv (&mpiall, 1, sc_MPI_INT, rank - 1, 0, mpicomm, &status);
    SC_CHECK_MPI (mpiret);
  }
  /* we propagate the (rankwise) earliest error */
  if (mpiall != sc_MPI_SUCCESS) {
    mpival = mpiall;
  }
  /* send errors to successor */
  if (rank != num_procs - 1) {
    mpiret = sc_MPI_Send (&mpival, 1, sc_MPI_INT, rank + 1, 0, mpicomm);
    SC_CHECK_MPI (mpiret);
  }
  /* broadcast the read error from the last rank */
  mpiret = sc_MPI_Bcast (&mpiall, sizeof (size_t),
                         sc_MPI_INT, num_procs - 1, mpicomm);
  SC_CHECK_MPI (mpiret);

  /* check read error */
  if (mpiall != sc_MPI_SUCCESS) {
    mpiret = sc_MPI_Error_string (mpiall, mpierrstr, &mpireslen);
    SC_CHECK_MPI (mpiret);
    if (mpiall == sc_MPI_ERR_OTHER) {
      P4EST_GLOBAL_LERROR ("Count mismatch: reading geodesics\n");
      P4EST_GLOBAL_LERROR (count_mismatch_message);
    }
    P4EST_GLOBAL_LERROR ("Error reading geodesics from file\n");
    P4EST_GLOBAL_LERRORF ("Error Code: %s\n", mpierrstr);
    /* cleanup on error */
    (void) sc_io_close (&file_handle);
    p4est_gmt_model_destroy (model);
    return NULL;
  }

  /* close the file collectively */
  mpival = sc_io_close (&file_handle);

  /* check file close error */
  if (mpival != sc_MPI_SUCCESS) {
    mpiret = sc_MPI_Error_string (mpival, mpierrstr, &mpireslen);
    SC_CHECK_MPI (mpiret);
    P4EST_GLOBAL_LERROR ("Error closing file\n");
    P4EST_GLOBAL_LERRORF ("Error Code: %s\n", mpierrstr);
    /* cleanup on error */
    p4est_gmt_model_destroy (model);
    return NULL;
  }

  /* the model is ready */
  return model;
}

void
p4est_gmt_model_destroy (p4est_gmt_model_t * model)
{
  /* free geometry when dynamically allocated */
  if (model->geom_allocated) {
    p4est_geometry_destroy (model->model_geom);
  }

  /* free model-specific data */
  if (model->destroy_data != NULL) {
    /* only needed for non-trivial free code */
    model->destroy_data (model->model_data);
  }
  else {
    /* the default clears a standard allocation or respects NULL */
    P4EST_FREE (model->model_data);
  }

  /* destroy connectivity outside of the specific model */
  p4est_connectivity_destroy (model->conn);
  P4EST_FREE (model);
}
