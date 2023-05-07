#ifndef CLIQUE_YUAN_PENG_H_INCLUDED
#define CLIQUE_YUAN_PENG_H_INCLUDED

#include "../algorithms/core.h"
#include "../collections/graph.h"
#include "../collections/ordered_set.h"
#include "../collections/queue.h"
#include "../utilities/array_util.h"

OrderedSet** find_k_cliques_yuan_peng(Graph* graph, int k);

#endif