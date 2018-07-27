CCC = g++
CPLEXDIR = /opt/ibm/ILOG/CPLEX_Studio126/cplex
CFLAGS = -std=c++11 -DIL_STD -O -DNDEBUG -I$(CPLEXDIR)/include -fPIC -m64 -DILOUSEMT -D_REENTRANT -DILM_REENTRANT
COMMONSFLAGS = -D commons
CUTSFLAGS = -D cuts_experiments
SELECTIONFLAGS = -D node_selection_experiments
HEURFLAGS = -D heuristics_experiments
LDFLAGS = -L$(CPLEXDIR)/lib/x86-64_linux/static_pic -lcplex -lpthread 


clean:
	rm -f CommonOil SelectionOil CutsOil HeuristicOil

common:
	$(CCC) -o CommonOil $(CFLAGS) $(COMMONSFLAGS) Oil.cpp $(LDFLAGS)

cuts_experiments:
	$(CCC) -o CutsOil $(CFLAGS) $(CUTSFLAGS) Oil.cpp $(LDFLAGS)

node_selection_experiments:
	$(CCC) -o SelectionOil $(CFLAGS) $(SELECTIONFLAGS) Oil.cpp $(LDFLAGS)

heuristics_experiments:
	$(CCC) -o HeuristicOil $(CFLAGS) $(HEURFLAGS) Oil.cpp $(LDFLAGS)