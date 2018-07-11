CCC = g++
CPLEXDIR = /media/libre/cplex/cplex
CFLAGS = -std=c++11 -DIL_STD -O -DNDEBUG -I$(CPLEXDIR)/include -fPIC -m64 -DILOUSEMT -D_REENTRANT -DILM_REENTRANT
LDFLAGS = -L$(CPLEXDIR)/lib/x86-64_linux/static_pic -lcplex -lpthread 


clean:
	rm -f MilkCollection

solve:
	$(CCC) -o MilkCollection $(CFLAGS) MilkCollection.cpp $(LDFLAGS)



