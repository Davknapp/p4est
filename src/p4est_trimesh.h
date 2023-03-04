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

/** \file p4est_trimesh.h
 *
 * Generate a conforming triangle mesh from a 2:1 balanced p4est.
 * This mesh is represented by augmenting the \ref p4est_lnodes structure.
 */

#ifndef P4EST_TRIMESH_H
#define P4EST_TRIMESH_H

#include <p4est_lnodes.h>

SC_EXTERN_C_BEGIN;

#if 0
typedef struct p4est_tnode_t
{
  p4est_qcoord_t      x;
  p4est_qcoord_t      y;
  p4est_topidx_t      which_tree;
}
p4est_tnode_t;
#endif

/** Lookup table structure defining a conforming triangle mesh.
 *
 * The \a lnodes member encodes the process-relavent corners and edges.
 * The structure can be created with or without including edges as nodes.
 * The members of \a lnodes are reinterpreted.
 *  - degree is 0.
 *  - vnodes is the maxium number of nodes per element, 9 or 25 (with edges).
 *  - face_code as defined in \ref \p4est_lnodes.h encodes hanging neighbors.
 *    Each valid face_code determines one possible node layout.
 *  - According to the node layout, the nodes of the elemnt are encoded.
 */
typedef struct p4est_trimesh
{
#if 0
  sc_array_t         *onodes;   /**< owned nodes: p4est_tnode_t */
  sc_array_t         *snodes;   /**< shared nodes: p4est_tnode_t */
#endif
  p4est_lnodes_t     *lnodes;   /**< mesh metadata */
}
p4est_trimesh_t;

p4est_trimesh_t    *p4est_trimesh_new (p4est_t * p4est,
                                       p4est_ghost_t * ghost, int with_edge);

void                p4est_trimesh_destroy (p4est_trimesh_t * trimesh);

SC_EXTERN_C_END;

#endif /* !P4EST_TRIMESH_H */
