CCC = g++
CPLEXDIR = /opt/ibm/ILOG/CPLEX_Studio126/cplex
CFLAGS = -std=c++11 -DIL_STD -O -DNDEBUG -I$(CPLEXDIR)/include -fPIC -m64 -DILOUSEMT -D_REENTRANT -DILM_REENTRANT
LDFLAGS = -L$(CPLEXDIR)/lib/x86-64_linux/static_pic -lcplex -lpthread 

solve:
	$(CCC) -o Energy $(CFLAGS) Energy.cpp $(LDFLAGS)

clean:
	rm -f Energy