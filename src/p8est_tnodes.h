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

#include <p8est_geometry.h>

SC_EXTERN_C_BEGIN;

/** Flag values for tnodes construction. */
typedef enum        p8est_tnodes_flags
{
  /** The default flags have no bits set. */
  P8EST_TNODES_FLAGS_NONE = 0,

  /** Generate geometric coordinates for nodes on the tree boundary.
   * Since the \ref p4est_connectivity may be periodic, the same lnode
   * entry (see \ref p4est_lnodes) may be referenced from more than one
   * coordinate location.  If periodicity is not expected, this flag is not
   * needed.  Otherwise, setting it disambiguates the coordinates between
   * multiple instances for the same lnode entry.  This enables for example
   * the visualization of the periodic unit square as a factual square. */
  P8EST_TNODES_COORDS_SEPARATE = 0x01
}
p8est_tnodes_flags_t;

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
  p4est_gloidx_t      global_toffset;   /**< Global tetrahedron offset
                                             for the current process. */
  p4est_gloidx_t      global_tcount;    /**< Global tetrahedron count. */
  p4est_locidx_t     *local_tcount;     /**< Tetrahedron count per process
                                             (has mpisize entries). */

  /** Offsets into local triangles per element and one beyond. */
  p4est_locidx_t     *local_element_offset;
  p4est_topidx_t      local_first_tree; /**< First local tree on process,
                                             -1 if process has no elements. */
  p4est_topidx_t      local_last_tree;  /**< Last local tree on process,
                                             -2 if process has no elements. */
  /** Offsets into local triangles, zero indexed from local_first_tree
   * to local_last_tree + 1 inclusive.  Length 1 on empty processes. */
  p4est_topidx_t     *local_tree_offset;

  sc_array_t         *simplices;        /**< Vertex indices of local
                                             simplices.  Each array entry
                                             holds 4 \ref p4est_locidx_t. */
  sc_array_t         *coord_to_lnode;   /**< This pointer may be NULL, in
                                             which case \c simplices indexes
                                             into both the local nodes from
                                             \ref p4est_lnodes and the \c
                                             coordinates below.  Otherwise,
                                             the simplex array indexes into \c
                                             coordinates, and this array maps
                                             a coordinate to its local node. */
  sc_array_t         *coordinates;      /**< Each entry is a double 3-tuple. */

  /* deprecated members below */
  p8est_tnodes_config_t *configuration; /**< One entry per element. */
  p8est_lnodes_t     *lnodes;   /**< Element and tetrahedron node data. */
  int                 lnodes_owned;     /**< Boolean: ownership of \a lnodes. */
  struct p8est_tnodes_private *pri;  /**< Private member not to access. */
}
p8est_tnodes_t;

/** Generate a conforming tetrahedron mesh from a 2:1 balance forest.
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
 * \return              Valid conforming tetrahedron mesh structure.
 */
p8est_tnodes_t     *p8est_tnodes_new (p8est_t * p4est,
                                      p8est_ghost_t * ghost, int full_style,
                                      int with_faces, int with_edges);

/** Generate a conforming tetrahedron mesh from a Q2 nodes structure.
 * \param [in] p4est                    Forest underlying the mesh.
 * \param [in] geom                     If NULL, we create tree relative
 *                                      reference coordinates in [0, 1]^3.
 *                                      Otherwise we apply \c geom.
 *                                      Any geometry should either be passed
 *                                      here, or to the VTK output routine,
 *                                      but not given in both places.
 * \param [in] lnodes                   Valid node structure of degree 2.
 *                                      Must be derived from \c p4est.
 * \param [in] lnodes_take_ownership    Boolean: we will own \c lnodes.
 * \param [in] construction_flags       Currently must be 0.
 * \return                              Valid conforming tetrahedron mesh.
 *                     Each tetrahedron is strictly contained in one element
 *                     of the p8est hexahedral mesh underlying \c lnodes.
 *                     Each element contains from 4 to 48 tetrahedra.
 *                     The tetrahedra are right-handed with respect to the
 *                     tree coordinate system containing their element.
 */
p8est_tnodes_t     *p8est_tnodes_new_Q2_P1 (p8est_t *p4est,
                                            p8est_geometry_t *geom,
                                            p8est_lnodes_t *lnodes,
                                            int lnodes_take_ownership,
                                            int construction_flags);

/** Free the memory in a conforming tetrahedron mesh structure.
 * \param [in] tnodes      Memory is deallocated.  Do not use after return.
 */
void                p8est_tnodes_destroy (p8est_tnodes_t * tnodes);

SC_EXTERN_C_END;

#endif /* !P8EST_TNODES_H */
