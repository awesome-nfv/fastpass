/*
 * pim.h
 *
 *  Created on: Apr 27, 2014
 *      Author: yonch
 */

#ifndef PIM_H_
#define PIM_H_

#include "edgelist.h"
#include "grant-accept.h"
#include "../graph-algo/admitted.h"
#include "../graph-algo/backlog.h"
#include "../graph-algo/bin.h"
#include "../graph-algo/fp_ring.h"
#include "../graph-algo/platform.h"

#define UNALLOCATED 0
#define ALLOCATED   1

/**
 * A structure for the state of a grant partition
 */
struct pim_state {
        struct ga_adj requests_by_src[N_PARTITIONS]; /* per src partition */
        struct ga_partd_edgelist grants;
        struct ga_adj grants_by_dst[N_PARTITIONS]; /* per dst partition */
        struct ga_partd_edgelist accepts;
        uint8_t grant_adj_index[MAX_NODES]; /* per src adj index of grant */
        uint8_t src_status[MAX_NODES]; /* TODO: use 1 bit per src instead of 8 */
        uint8_t dst_status[MAX_NODES]; /* TODO: use 1 bit per dst instead of 8 */
        struct backlog backlog;
        struct bin *new_demands[N_PARTITIONS]; /* per src partition */
        struct fp_ring *q_admitted_out;
        struct fp_mempool *admitted_traffic_mempool;
        struct admission_statistics stat;
};

/**
 * Prepare data structures so they are ready to allocate the next timeslot
 */
void pim_prepare(struct pim_state *state, uint16_t partition_index);

/**
 * For all source (left-hand) nodes in partition 'partition_index',
 *    selects edges to grant. These are added to 'grants'.
 */
void pim_do_grant(struct pim_state *state, uint16_t partition_index);

/**
 * For every destination (right-hand) node in partition 'partition_index',
 *    select among its granted edges which edge to accept. These edges are
 *    added to 'accepts'
 */
void pim_do_accept(struct pim_state *state, uint16_t partition_index);

/**
 * Process all of the accepts, after a timeslot is done being allocated
 */
void pim_process_accepts(struct pim_state *state, uint16_t partition_index);

/**
 * Initialize all demands to zero
 */
static inline
void pim_reset_state(struct pim_state *state)
{
        uint16_t src_partition;
        for (src_partition = 0; src_partition < N_PARTITIONS; src_partition++) {
                ga_reset_adj(&state->requests_by_src[src_partition]);
        }
        backlog_init(&state->backlog);
}

/**
 * Initialize pim state
 */
static inline
void pim_init_state(struct pim_state *state, struct fp_ring *q_admitted_out,
                    struct fp_mempool *admitted_traffic_mempool)
{
        pim_reset_state(state);
        uint16_t partition;
        for (partition = 0; partition < N_PARTITIONS; partition++) {
                /* TODO: use bins instead */
                state->new_demands[partition] = create_bin(MAX_NODES);
        }

        state->q_admitted_out = q_admitted_out;
        state->admitted_traffic_mempool = admitted_traffic_mempool;
}

#endif /* PIM_H_ */
