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

/** \file p8est_tnodes.h
 *
 * Generate a conforming tetrahedron mesh from a 2:1 balanced p8est.
 * This mesh is represented by augmenting the \ref p8est_lnodes structure.
 */

#ifndef P8EST_TNODES_H
#define P8EST_TNODES_H

#include <p8est_lnodes.h>

SC_EXTERN_C_BEGIN;

/** Integer type to store the bits of an element configuration. */
typedef uint32_t    p8est_tnodes_config_t;

/** Lookup table structure defining a conforming tetrahedral mesh.
 *
 * The \a lnodes member encodes process-relavent corners, edges and faces.
 * Tetrahedron-shaped volume and corner entities are always included.
 * Can be created with or without including faces and/or edges as entities.
 * The members of \a lnodes are reinterpreted; cf. \ref p8est_lnodes.h :
 *  - degree is set to 0.
 *  - vnodes is the maxium number of nodes per element.
 */
typedef struct p8est_tnodes
{
  int                 full_style;       /**< Full style subdivision? */
  int                 with_faces;       /**< Include tetrahedron faces? */
  int                 with_edges;       /**< Include tetrahedron edges? */
  p8est_tnodes_config_t *configuration; /**< One entry per element. */
  p4est_gloidx_t      global_toffset;   /**< Global triangle offset
                                             for the current process. */
  p4est_locidx_t     *global_tcount;    /**< Triangle count per process
                                             (has mpisize entries). */
  p4est_locidx_t     *local_toffset;    /**< Triangle offsets per local
                                             element and one beyond. */
  p8est_lnodes_t     *lnodes;   /**< Element and triangle node data. */
  struct p8est_tnodes_private *pri;  /**< Private member not to access. */
}
p8est_tnodes_t;

/** Generate a conforming triangle mesh from a 2:1 balance forest.
 * \param [in] p4est    Valid forest after 2:1 (at least face) balance.
 * \param [in] ghost    Ghost layer created from \b p4est.  Even with MPI,
 *                      it may be NULL to number the nodes purely locally.
 *                      In this case, nodes on a parallel boundary will be
 *                      considered as local for each touching process.
 *                      No shared nodes will be created.
 * \param [in] full_style   Half or full subdivision for unrefined elements.
 * \param [in] with_faces   If true, include each face of the tetrahedral
 *                          mesh as a node, otherwise ignore all faces.
 * \param [in] with_edges   If true, include each edge of the tetrahedral
 *                          mesh as a node, otherwise ignore all edges.
 * \return              Valid conforming triangle mesh structure.
 */
p8est_tnodes_t     *p8est_tnodes_new (p8est_t * p4est,
                                      p8est_ghost_t * ghost, int full_style,
                                      int with_faces, int with_edges);

/** Free the memory in a conforming triangle mesh structure.
 * \param [in] tnodes      Memory is deallocated.  Do not use after return.
 */
void                p8est_tnodes_destroy (p8est_tnodes_t * tnodes);

SC_EXTERN_C_END;

#endif /* !P8EST_TNODES_H */
