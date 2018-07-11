CCC = g++
CPLEXDIR = /media/libre/cplex
CFLAGS = -std=c++11 -DIL_STD -O -DNDEBUG -I$(CPLEXDIR)/include -fPIC -m64 -DILOUSEMT -D_REENTRANT -DILM_REENTRANT
LDFLAGS = -L$(CPLEXDIR)/lib/x86-64_linux/static_pic -lcplex -lpthread 


clean:
	rm -f food_manufacure

solve:
	$(CCC) -o food_manufacure $(CFLAGS) food_manufacure.cpp $(LDFLAGS)



