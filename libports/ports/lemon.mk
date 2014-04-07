LEMON_VERSION = 1.3
LEMON         = lemon-$(LEMON_VERSION)
LEMON_TGZ     = $(LEMON).tar.gz
LEMON_URL     = http://lemon.cs.elte.hu/pub/sources/$(LEMON_TGZ)
LEMON_MD5     = 814d97757c157c5b46a413cc39aad625

#
# Interface to top-level prepare Makefile
#
PORTS += $(LEMON)

prepare-lemon: $(CONTRIB_DIR)/$(LEMON) include/lemon

$(CONTRIB_DIR)/$(LEMON):clean-lemon

#
# Port-specific local rules
#
$(DOWNLOAD_DIR)/$(LEMON_TGZ):
	$(VERBOSE)wget -c -P $(DOWNLOAD_DIR) $(LEMON_URL) && touch $@

$(DOWNLOAD_DIR)/$(LEMON_TGZ).verified: $(DOWNLOAD_DIR)/$(LEMON_TGZ)
	$(VERBOSE)$(HASHVERIFIER) $(DOWNLOAD_DIR)/$(LEMON_TGZ) $(LEMON_MD5) md5
	$(VERBOSE)touch $@

$(CONTRIB_DIR)/$(LEMON): $(DOWNLOAD_DIR)/$(LEMON_TGZ).verified
	$(VERBOSE)tar xfz $(<:.verified=) -C $(CONTRIB_DIR) && touch $@

LEMON_INCLUDES = concepts \
					  bits \
                 adaptors.h\
                 arg_parser.h\
                 assert.h\
                 bellman_ford.h\
                 bfs.h\
                 bin_heap.h\
                 binomial_heap.h\
                 bucket_heap.h\
                 capacity_scaling.h\
                 cbc.h\
                 christofides_tsp.h\
                 circulation.h\
                 clp.h\
                 color.h\
                 concept_check.h\
                 connectivity.h\
                 core.h\
                 cost_scaling.h\
                 counter.h\
                 cplex.h\
                 cycle_canceling.h\
                 dfs.h\
                 dheap.h\
                 dijkstra.h\
                 dim2.h\
                 dimacs.h\
                 edge_set.h\
                 edmonds_karp.h\
                 elevator.h\
                 error.h\
                 euler.h\
                 fib_heap.h\
                 fractional_matching.h\
                 full_graph.h\
                 glpk.h\
                 gomory_hu.h\
                 graph_to_eps.h\
                 greedy_tsp.h\
                 grid_graph.h\
                 grosso_locatelli_pullan_mc.h\
                 hao_orlin.h\
                 hartmann_orlin_mmc.h\
                 howard_mmc.h\
                 hypercube_graph.h\
                 insertion_tsp.h\
                 karp_mmc.h\
                 kruskal.h\
                 lgf_reader.h\
                 lgf_writer.h\
                 list_graph.h\
                 lp_base.h\
                 lp.h\
                 lp_skeleton.h\
                 maps.h\
                 matching.h\
                 math.h\
                 max_cardinality_search.h\
                 min_cost_arborescence.h\
                 nagamochi_ibaraki.h\
                 nauty_reader.h\
                 nearest_neighbor_tsp.h\
                 network_simplex.h\
                 opt2_tsp.h\
                 pairing_heap.h\
                 path.h\
                 planarity.h\
                 preflow.h\
                 quad_heap.h\
                 radix_heap.h\
                 radix_sort.h\
                 random.h\
                 smart_graph.h\
                 soplex.h\
                 static_graph.h\
                 suurballe.h\
                 time_measure.h\
                 tolerance.h\
                 unionfind.h

include/lemon:
	$(VERBOSE)mkdir -p $@
	$(VERBOSE)for i in $(LEMON_INCLUDES); do \
		ln -sf ../../$(CONTRIB_DIR)/$(LEMON)/lemon/$$i $@; done
	$(VERBOSE)ln -sf ../../src/lib/lemon/config.h $@

clean-lemon:
	$(VERBOSE)rm -rf include/lemon
	$(VERBOSE)rm -rf $(CONTRIB_DIR)/$(LEMON)
