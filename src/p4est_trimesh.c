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

#include <p4est_iterate.h>
#include <p4est_trimesh.h>

typedef struct trimesh_meta
{
  int                 with_faces;
  p4est_locidx_t      lenum;
  p4est_locidx_t      num_owned;
  p4est_locidx_t      num_shared;
  p4est_locidx_t      szero[25];
  p4est_t            *p4est;
  p4est_ghost_t      *ghost;
  p4est_trimesh_t    *tm;
}
trimesh_meta_t;

static const int    node_seq[9] = { 4, 3, 5, 1, 7, 0, 2, 6, 8 };
static const int    node_dim[3] = { 0, 1, 5 };

static void
iter_volume1 (p4est_iter_volume_info_t * vi, void *user_data)
{
  trimesh_meta_t     *me = (trimesh_meta_t *) user_data;
  p4est_lnodes_t     *ln = me->tm->lnodes;
  p4est_locidx_t      le;
#ifdef P4EST_ENABLE_DEBUG
  p4est_tree_t       *tree;

  /* initial checks  */
  P4EST_ASSERT (vi->p4est == me->p4est);
  tree = p4est_tree_array_index (vi->p4est->trees, vi->treeid);
  P4EST_ASSERT (tree->quadrants_offset + vi->quadid == me->lenum);

#endif
  /* create owned node */
  le = me->lenum++;
  P4EST_ASSERT (ln->face_code[le] == 0);
  P4EST_ASSERT (!memcmp (ln->element_nodes + ln->vnodes * le,
                         me->szero, sizeof (p4est_locidx_t) * ln->vnodes));

  /* place owned node at quadrant midpoint */
  ln->element_nodes[ln->vnodes * le + node_seq[0]] = me->num_owned++;
}

static void
iter_face1 (p4est_iter_face_info_t * fi, void *user_data)
{
  trimesh_meta_t     *me = (trimesh_meta_t *) user_data;
  int                 i, j;
  /* each face connection produces at most 3 nodes: 1 corner, 2 face */
  int                 codim[3];         /**< codimension of a node */
  int                 is_owned[3];      /**< is that node locally owned */
  int                 num_refs[3];      /**< number references to a node */
  int                 indowned[3][3];   /**< for a node, which position in
                                             each containing quadrant */
  p4est_iter_face_side_t *fs, *fss[2];

  /* initial checks  */
  P4EST_ASSERT (fi->p4est == me->p4est);

  /* find ownership of all nodes on this face connection */
  for (i = 0; i < 3; ++i) {
    codim[i] = -1;
    is_owned[i] = num_refs[i] = 0;
    for (j = 0; j < 3; ++j) {
      indowned[i][j] = -1;
    }
  }
  if (fi->sides.elem_count == 1) {
    P4EST_ASSERT (fi->orientation == 0);
    P4EST_ASSERT (fi->tree_boundary == P4EST_CONNECT_FACE);
    fs = (p4est_iter_face_side_t *) sc_array_index_int (&fi->sides, 0);
    P4EST_ASSERT (!fs->is_hanging);
    P4EST_ASSERT (!fs->is.full.is_ghost);
    if (me->with_faces) {
      /* produce one face node */
      codim[0] = 1;
      num_refs[0] = 1;
      indowned[0][0] = node_seq[node_dim[1] + fs->face];

      /* ownership is trivial */
      is_owned[0] = 1;
    }
  }
  else {
    P4EST_ASSERT (fi->sides.elem_count == 2);
    fss[0] = (p4est_iter_face_side_t *) sc_array_index_int (&fi->sides, 0);
    fss[1] = (p4est_iter_face_side_t *) sc_array_index_int (&fi->sides, 1);
    P4EST_ASSERT (!fss[0]->is_hanging || !fss[1]->is_hanging);
    if (!fss[0]->is_hanging && !fss[1]->is_hanging) {
      /* conforming (same-size) face connection */
      if (me->with_faces) {
        codim[0] = 1;
        num_refs[0] = 2;
        indowned[0][0] = node_seq[node_dim[1] + fss[0]->face];
        indowned[0][1] = node_seq[node_dim[1] + fss[1]->face];

        /* examine ownership situation */

      }
    }
    else {

    }

  }
}

static void
iter_corner1 (p4est_iter_corner_info_t * ci, void *user_data)
{
}

p4est_trimesh_t    *
p4est_trimesh_new (p4est_t * p4est, p4est_ghost_t * ghost, int with_faces)
{
  int                 mpiret;
  int                 p, q, s;
  int                 vn;
  p4est_locidx_t      le;
  p4est_gloidx_t      gc;
  p4est_trimesh_t    *tm;
  p4est_lnodes_t     *ln;
  trimesh_meta_t      tmeta, *me = &tmeta;

  P4EST_ASSERT (p4est_is_balanced (p4est, P4EST_CONNECT_FACE));

  memset (me, 0, sizeof (trimesh_meta_t));
  me->p4est = p4est;
  me->ghost = ghost;
  me->with_faces = with_faces;
  tm = me->tm = P4EST_ALLOC_ZERO (p4est_trimesh_t, 1);
  ln = tm->lnodes = P4EST_ALLOC_ZERO (p4est_lnodes_t, 1);

  p = p4est->mpirank;
  s = p4est->mpisize;
  ln->mpicomm = p4est->mpicomm;
  ln->sharers = sc_array_new (sizeof (p4est_lnodes_rank_t));
  ln->degree = 0;
  vn = ln->vnodes = 9 + (with_faces ? 16 : 0);
  le = ln->num_local_elements = p4est->local_num_quadrants;
  ln->face_code = P4EST_ALLOC_ZERO (p4est_lnodes_code_t, le);
  ln->element_nodes = P4EST_ALLOC_ZERO (p4est_locidx_t, le * vn);

  /* determine the face_code for each element */
  me->lenum = 0;
  p4est_iterate (p4est, ghost, me, iter_volume1, iter_face1, iter_corner1);
  P4EST_ASSERT (me->lenum == le);
  P4EST_INFOF ("p4est_trimesh_new: owned %ld shared %ld\n",
               (long) me->num_owned, (long) me->num_shared);

  /* send messages */

  /* share owned count */
  ln->global_owned_count = P4EST_ALLOC (p4est_locidx_t, s);
  mpiret = sc_MPI_Allgather (&me->num_owned, 1, P4EST_MPI_LOCIDX,
                             ln->global_owned_count, 1, P4EST_MPI_LOCIDX,
                             p4est->mpicomm);
  SC_CHECK_MPI (mpiret);
  ln->global_offset = 0;
  for (q = 0; q < p; ++q) {
    ln->global_offset += ln->global_owned_count[q];
  }
  for (gc = ln->global_offset; q < s; ++q) {
    gc += ln->global_owned_count[q];
  }
  P4EST_GLOBAL_PRODUCTIONF ("p4est_trimesh_new: global owned %lld\n",
                            (long long) gc);

  /* receive messages */
  /* populate nflags */
  /* finalize lnodes */

  return tm;
}

void
p4est_trimesh_destroy (p4est_trimesh_t * tm)
{
  P4EST_ASSERT (tm != NULL);
  P4EST_ASSERT (tm->lnodes != NULL);

  p4est_lnodes_destroy (tm->lnodes);
  P4EST_FREE (tm->nflags);
  P4EST_FREE (tm);
}