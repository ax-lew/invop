#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <ilcplex/cplex.h>
#include <iomanip>
#include <ctime>
using namespace std;

int status=0;
int presolve_bit_mask = 0; // presolve all but CPX_PARAM_MIPCBREDLP
int heuristics_bit_mask = 0; //all the heuristics
int cut_bit_mask = 0; // all the cuts available
int cut_pass = -1; //until 100 plane cuts in the initial node
int node_selection_index = 0;

struct Problem {
    vector<float> hardness;
    vector< vector<int> > prices;
    int vegetable_oils;
    float limit_use_vegetables_per_month;
    float limit_use_non_vegetables_per_month;
    float price_stock_per_ton;
    float benefit_per_ton;
    float initial_stock;
    
    Problem(int vegetable_oils, int count_oils, int count_month, float limit_use_vegetables_per_month, float limit_use_non_vegetables_per_month, 
        float benefit_per_ton, float price_stock_per_ton, float initial_stock) {
        this->vegetable_oils = vegetable_oils;
        this->price_stock_per_ton = price_stock_per_ton;
        this->benefit_per_ton = benefit_per_ton;
        this->initial_stock = initial_stock;
        this->limit_use_vegetables_per_month = limit_use_vegetables_per_month;
        this->limit_use_non_vegetables_per_month = limit_use_non_vegetables_per_month;
        this->hardness = vector<float>(count_oils);
        this->prices = vector< vector<int> > (count_month, vector<int>(count_oils));
    }
};

std::ostream& operator<<(std::ostream& os, Problem &pr)
    {
        os << pr.vegetable_oils << " " << pr.prices[0].size() << " " << pr.prices.size() << " " << pr.limit_use_vegetables_per_month << " ";
        os << pr.limit_use_non_vegetables_per_month << " " << pr.benefit_per_ton << " " << pr.price_stock_per_ton << " " << pr.initial_stock << "\n";
        os << pr.hardness[0];
        for(int i= 1; i < pr.hardness.size(); i++){
            os<< " " << pr.hardness[i];
        }
        os << "\n";

        for(int m = 0; m < pr.prices.size(); m++){
            os << pr.prices[m][0];
            for(int i= 1; i < pr.prices[m].size(); i++){
                os << " " << pr.prices[m][i];
            }
            os << "\n";
        }
        return os;
    }


void load_input_from_istream(Problem &problem, istream &stream){
    for(int i = 0 ; i < problem.hardness.size(); i++){
        stream >> problem.hardness[i];
    }
    for(int m = 0; m < problem.prices.size(); m++){
        for(int i = 0 ; i < problem.prices[m].size(); i++){
            stream >> problem.prices[m][i];
        }
    }
}

int GetIndexForStock(int oils, int month, int oil){
    return month*oils + oil;
}

int GetIndexForUse(int months, int oils, int month, int oil){
    return months*oils + month*oils + oil;
}

int GetIndexForBought(int months, int oils, int month, int oil){
    return months*oils*2 + month*oils + oil;
}

void output_conf(CPXENVptr &env){
    /////////
    // Output
    status = CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON);
    if(status)exit(-1);
    
    //~ status = CPXsetintparam(env, CPX_PARAM_MIPDISPLAY, 5);
    //~ if(status)exit(-1);
    
    //~ status = CPXsetintparam(env, CPX_PARAM_MIPINTERVAL, 1 );
    //~ if(status)exit(-1);
    /////////

    // Para setear un archivo de log
    //~ CPXFILEptr logfile = NULL;
    //~ logfile = CPXfopen ("BPGC.log", "a");
    //~ CPXsetlogfile (env, logfile);

    return;
}

void cut_conf(CPXENVptr &env){
    /////////
    // CPLEX cuts
    status = CPXsetintparam(env, CPX_PARAM_CUTPASS, cut_pass);
    if(status)exit(-1);
    
    vector<int> avaiable_cuts = {CPX_PARAM_CLIQUES, CPX_PARAM_COVERS, CPX_PARAM_DISJCUTS, CPX_PARAM_FLOWCOVERS, CPX_PARAM_FLOWPATHS, CPX_PARAM_FRACCUTS, CPX_PARAM_GUBCOVERS, CPX_PARAM_MCFCUTS, CPX_PARAM_IMPLBD, CPX_PARAM_MIRCUTS, CPX_PARAM_ZEROHALFCUTS};
    for(int i = 0; i < avaiable_cuts.size(); i++) {
        status = CPXsetintparam(env, avaiable_cuts[i], ( (cut_bit_mask&(1<<i) != 0)? 1 : -1) );
        if(status)exit(-1);
    }
    /////////

    return;
}

void heuristic_conf(CPXENVptr &env){
    /////////
    // CPLEX heuristics
    vector<int> avaiable_heurs = {CPX_PARAM_HEURFREQ, CPX_PARAM_RINSHEUR, CPX_PARAM_FPHEUR};
    for(int i = 0; i < avaiable_heurs.size(); i++){
        status = CPXsetintparam(env, avaiable_heurs[i], ( ((heuristics_bit_mask&(1<<i)) != 0)? 1 : -1) );
        if(status)exit(-1);
    }
    
    status = CPXsetintparam(env, CPX_PARAM_LBHEUR, ( ((heuristics_bit_mask & (1<<3)) != 0) ? 1 : 0));
    if(status)exit(-1);
    /////////

    return;
}

void presolve_conf(CPXENVptr &env){
    /////////
    // Presolve
    status = CPXsetintparam (env, CPX_PARAM_MIPCBREDLP, ( (presolve_bit_mask & (1<<0) != 0)? CPX_ON : CPX_OFF));
    if(status)exit(-1);
    
    // LINEAR REDUCTION OF VARIABLES
    status = CPXsetintparam (env, CPX_PARAM_PRELINEAR, ( (presolve_bit_mask & (1<<1) != 0)? 1 : 0));
    if(status)exit(-1);
    
    // PRIMAL AND DUAL REDUCTIONS (3), NONE (0)
    status = CPXsetintparam (env, CPX_PARAM_REDUCE, ( (presolve_bit_mask & (1<<2) != 0)? 3 : 0));
    if(status)exit(-1);

    // -1 let CPLEX choose; default | 0 Turn off represolve
    status = CPXsetintparam (env, CPX_PARAM_REPEATPRESOLVE, ( (presolve_bit_mask & (1<<3) != 0)? -1 : 0));
    if(status)exit(-1);

    // THERE ARE MORE LEVELS 1-3 OF AGGRESSIVE PROBING ON VARIABLES
    status = CPXsetintparam (env, CPX_PARAM_PROBE, ( (presolve_bit_mask & (1<<4) != 0)? 0 : -1));
    if(status)exit(-1);    
    /////////

    return;
}

void execution_conf(CPXENVptr &env){
    /////////
    // Execution decisions
    status = CPXsetdblparam(env, CPX_PARAM_TILIM, 900.0);
    if(status)exit(-1);
    
    // AMOUNT OF NODES TO SOLVE, DEFAULT MORE THAN 9 MILLIONS
    //~ status = CPXsetintparam(env, CPX_PARAM_NODELIM, 1);
    //~ if(status)exit(-1);
    
    // 1 DETERMINISTIC, 0 CPLEX DECIDES, -1 OPPORTUNISTIC
    status = CPXsetintparam(env, CPX_PARAM_PARALLELMODE, 1);
    if(status)exit(-1);

    //SINGLE THREAD
    status = CPXsetintparam(env, CPX_PARAM_THREADS, 1);
    if(status)exit(-1);

    // check input for possible mistakes
    status = CPXsetintparam(env, CPX_PARAM_DATACHECK, CPX_ON);
    if(status)exit(-1);
    /////////

    return;
}

void branch_conf(CPXENVptr &env){
    //~ status = CPXsetintparam(env, CPX_PARAM_NODESEL, CPX_NODESEL_DFS);
    vector<int> available_node_selection = {CPX_NODESEL_DFS, CPX_NODESEL_BESTBOUND, CPX_NODESEL_BESTEST, CPX_NODESEL_BESTEST_ALT};

    status = CPXsetintparam(env, CPX_PARAM_NODESEL, available_node_selection[node_selection_index]);
    if(status)exit(-1);
    
    status = CPXsetintparam(env, CPX_PARAM_MIPSEARCH, CPX_MIPSEARCH_TRADITIONAL);
    if(status)exit(-1);

    return;
}

void print_result(Problem &problem, double *x, double objval, ostream &out){
    int months = problem.prices.size();
    int oils = problem.prices[0].size();
        
    out << "Result " << objval << endl;

    out << endl << setw(15);
    for (int _month = 0; _month < months; _month++){
        out << _month << setw(15);
    }
    out << endl;

    for (int _oil=0; _oil< oils; _oil++){
        for (int _month = 0; _month < months; _month++){
            out << x[GetIndexForStock(oils, _month, _oil)] << setw(15);
        }
        out << endl;
    }

    out << endl;

    for (int _oil=0; _oil< oils; _oil++){
        for (int _month = 0; _month < months; _month++){
            out << x[GetIndexForUse(months, oils, _month, _oil)] << setw(15);
        }
        out << endl;
    }

    out << endl;

    for (int _oil=0; _oil< oils; _oil++){
        for (int _month = 0; _month < months; _month++){
            out << x[GetIndexForBought(months, oils, _month, _oil)] << setw(15);
        }
        out << endl;
    }

    return;
}

float Solve(Problem &problem){
    float time_solver;

    double *obj=nullptr,*lb=nullptr,*ub=nullptr,*rmatval=nullptr,*rhs=nullptr,*x=nullptr;
    char *coltype=nullptr, *sense=nullptr;
    int *rmatbeg=nullptr, *rmatind=nullptr;
    CPXENVptr env = NULL;
    CPXLPptr lp = NULL;

    try{
    clock_t t1,t2;

#pragma region YOU_DONT_WANT_TO_SEE_THIS    
    int months = problem.prices.size(), oils = problem.hardness.size(); 
    int cantvar = months*oils*3;

    obj=(double*)malloc(sizeof(double)*cantvar);
    lb=(double*)malloc(sizeof(double)*cantvar);
    ub=(double*)malloc(sizeof(double)*cantvar);
    coltype=(char*)malloc(sizeof(char)*cantvar);
    
    rmatbeg=(int*)malloc(sizeof(int));
    rmatind=(int*)malloc(sizeof(int)*cantvar);
    rmatval=(double*)malloc(sizeof(double)*cantvar);
    rhs=(double*)malloc(sizeof(double)*1);
    sense=(char*)malloc(sizeof(char)*1);
    double objval;


    env = CPXopenCPLEX(&status);
    if(env==NULL)exit(-1);
    
    output_conf(env);
    cut_conf(env);
    heuristic_conf(env);
    presolve_conf(env);
    execution_conf(env);
    branch_conf(env);
#pragma endregion YOU_DONT_WANT_TO_SEE_THIS    
    
    // Creamos el problema en si en CPLEX
    lp = CPXcreateprob(env, &status, "oil");
    if(lp==NULL)exit(-1);
    
    
    // Definimos la funcion objetivo como una minimizacion
    CPXchgobjsen(env, lp, CPX_MAX);

#pragma region VARIABLES  
    //Stock ith jth : ith month, jth oil
    for(int _month = 0; _month < months; _month++){
        for(int _oil = 0; _oil < oils; _oil++){
            int index = GetIndexForStock(oils, _month, _oil);
            obj[index] = -problem.price_stock_per_ton;
            lb[index] = 0.;
            ub[index] = 1000.;
            coltype[index] = 'C';    
        }
    }

    //Use ith jth : ith month, jth oil
    for(int _month = 0; _month < months; _month++){
        for(int _oil = 0; _oil < oils; _oil++){
            int index = GetIndexForUse(months, oils, _month, _oil);
            obj[index] = problem.benefit_per_ton;
            lb[index] = 0.;
            ub[index] = 1000000000.;
            coltype[index] = 'C';    
        }
    }

    //Bought ith jth : ith month, jth oil
    for(int _month = 0; _month < months; _month++){
        for(int _oil = 0; _oil < oils; _oil++){
            int index = GetIndexForBought(months, oils, _month, _oil);
            obj[index] = -problem.prices[_month][_oil];
            lb[index] = 0.;
            ub[index] = 1000000000.;
            coltype[index] = 'C';   
        }
    }

    status = CPXnewcols(env, lp, cantvar, obj, lb, ub, coltype, NULL);
    if(status)exit(-1);
#pragma endregion VARIABLES

#pragma region CONSTRAINS
    //limit on use of vegetable oils
    for (int _month=0; _month < problem.prices.size(); _month++){
        int count = 0;
        rmatbeg[0] = 0;
        rhs[0] = problem.limit_use_vegetables_per_month;
        sense[0] = 'L';
        for(int _veg_oil=0; _veg_oil < problem.vegetable_oils; _veg_oil++){

            int index = GetIndexForUse(months, oils, _month, _veg_oil);
            
            rmatval[count] = 1;
            rmatind[count] = index;

            count++;
        }

        status = CPXaddrows(env, lp, 0, 1, count, rhs, sense, rmatbeg, rmatind, rmatval, NULL, NULL);
        if(status)exit(-1);
    }

    //limit on use of non vegetable oils
    for (int _month=0; _month < problem.prices.size(); _month++){
        int count = 0;
        rmatbeg[0] = 0;
        rhs[0] = problem.limit_use_non_vegetables_per_month;
        sense[0] = 'L';
        for(int _nonv_oil = problem.vegetable_oils; _nonv_oil < oils; _nonv_oil++){
            
            int index = GetIndexForUse(months, oils, _month, _nonv_oil);
            
            rmatval[count] = 1;
            rmatind[count] = index;

            count++;
        }

        status = CPXaddrows(env, lp, 0, 1, count, rhs, sense, rmatbeg, rmatind, rmatval, NULL, NULL);
        if(status)exit(-1);
    }

    float lower_bound_hardness = 3;
    float upper_bound_hardness = 6;
    //lower bound on hardness
    for (int _month=0; _month < problem.prices.size(); _month++){
        int count = 0;
        rmatbeg[0] = 0;
        rhs[0] = 0;
        sense[0] = 'G';
        for(int _oil = 0; _oil < oils; _oil++){
            
            int index = GetIndexForUse(months, oils, _month, _oil);
            
            rmatval[count] = (problem.hardness[_oil] - lower_bound_hardness);
            rmatind[count] = index;

            count++;
        }

        status = CPXaddrows(env, lp, 0, 1, count, rhs, sense, rmatbeg, rmatind, rmatval, NULL, NULL);
        if(status)exit(-1);
    }

    //upper bound on hardness
    for (int _month=0; _month < problem.prices.size(); _month++){
        int count = 0;
        rmatbeg[0] = 0;
        rhs[0] = 0;
        sense[0] = 'G';
        for(int _oil = 0; _oil < oils; _oil++){
            
            int index = GetIndexForUse(months, oils, _month, _oil);
            
            rmatval[count] = (upper_bound_hardness - problem.hardness[_oil]);
            rmatind[count] = index;

            count++;
        }

        status = CPXaddrows(env, lp, 0, 1, count, rhs, sense, rmatbeg, rmatind, rmatval, NULL, NULL);
        if(status)exit(-1);
    }

    //compute stock
    for (int _month=0; _month < problem.prices.size()-1; _month++){
        for(int _oil = 0; _oil < oils; _oil++){
            int count = 0, index;
            rmatbeg[0] = 0;
            rhs[0] = 0;
            
            sense[0] = 'E';    

            index = GetIndexForStock(oils, _month+1, _oil);
            rmatval[count] = 1;
            rmatind[count] = index;
            count++;

            index = GetIndexForStock(oils, _month, _oil);
            rmatval[count] = -1;
            rmatind[count] = index;
            count++;
            
            index = GetIndexForBought(months, oils, _month+1, _oil);
            rmatval[count] = -1;
            rmatind[count] = index;
            count++;

            index = GetIndexForUse(months, oils, _month+1, _oil);
            rmatval[count] = +1;
            rmatind[count] = index;
            count++;

            status = CPXaddrows(env, lp, 0, 1, count, rhs, sense, rmatbeg, rmatind, rmatval, NULL, NULL);
            if(status)exit(-1);
        }
    }

    // same custom for first month
    for(int _oil = 0; _oil < oils; _oil++){
        int count = 0, index;
        rmatbeg[0] = 0;
        rhs[0] = problem.initial_stock;
        
        sense[0] = 'E';

        index = GetIndexForStock(oils, 0, _oil);
        rmatval[count] = 1;
        rmatind[count] = index;
        count++;
        
        index = GetIndexForBought(months, oils, 0, _oil);
        rmatval[count] = -1;
        rmatind[count] = index;
        count++;

        index = GetIndexForUse(months, oils, 0, _oil);
        rmatval[count] = 1;
        rmatind[count] = index;
        count++;

        status = CPXaddrows(env, lp, 0, 1, count, rhs, sense, rmatbeg, rmatind, rmatval, NULL, NULL);
        if(status)exit(-1);
    }

    // at the end of the last month there should be the same initial stock of each oil
    for(int _oil = 0; _oil < oils; _oil++){
        int count = 0, index;
        rmatbeg[0] = 0;
        rhs[0] = problem.initial_stock;
        sense[0] = 'E';

        index = GetIndexForStock(oils, months-1, _oil);
        rmatval[count] = 1;
        rmatind[count] = index;
        count++;

        status = CPXaddrows(env, lp, 0, 1, count, rhs, sense, rmatbeg, rmatind, rmatval, NULL, NULL);
        if(status)exit(-1);
    }
#pragma endregion CONSTRAINS    

    t1=clock();
    status = CPXmipopt(env,lp);
    if(status) return -1;
    t2=clock();
    time_solver = ((float)t2-(float)t1) / float(CLOCKS_PER_SEC);

#ifdef commons
    //in case of error during experiments
    status = CPXgetobjval (env, lp, &objval); // Para traer el mejor valor obtenido
    if(status)exit(-1);
    
    int cur_numrows=CPXgetnumrows(env,lp);
    int cur_numcols=CPXgetnumcols(env,lp);
    x = (double *) malloc(cur_numcols * sizeof(double));
    
    status = CPXgetx (env, lp, x, 0, cur_numcols-1);
    if(status)exit(-1);
    
    print_result(problem, x, objval, cout);
    delete x;
#endif
    }
    catch(int e){
        time_solver = -1;
        if(x != nullptr) delete x;
    }

    if(obj != nullptr) delete obj;
    if(lb != nullptr) delete lb;
    if(ub != nullptr) delete ub;
    if(coltype != nullptr) delete coltype;
    if(rmatbeg != nullptr) delete rmatbeg;
    if(rmatind != nullptr) delete rmatind;
    if(rmatval != nullptr) delete rmatval;
    if(rhs != nullptr) delete rhs;
    if(sense != nullptr) delete sense;
    if(lp != nullptr) CPXfreeprob(env, &lp);
    if(env != nullptr) CPXcloseCPLEX(&env);
    
    return time_solver;
}

double prune(vector<float> &times){
    
    int from = 0, to = times.size();
    double res = 0;

    if(times.size() > 10){
        sort(times.begin(), times.end());
        int from = floor(times.size() / 5);
        int to = ceil(4 * times.size() / 5);
    }
    for(int i = from; i < to; i++){
        res += times[i];
    }
    
    return res/=(to-from);
}


int main(int argc, char** argv){ 
    int count_inputs, repeats;
    cin >> count_inputs >> repeats;
    vector<Problem> inputs = vector<Problem>();
    
    int count_vegetable_oils, count_oils, count_month;
    float price_stock_per_ton, initial_stock, limit_use_vegetables_per_month, limit_use_non_vegetables_per_month, benefit_per_ton;

    for(int i = 0; i < count_inputs; i++){
        cin >> count_vegetable_oils >> count_oils >> count_month >> limit_use_vegetables_per_month >> limit_use_non_vegetables_per_month >> benefit_per_ton >> price_stock_per_ton >> initial_stock;
        inputs.push_back(
            Problem(count_vegetable_oils, count_oils, count_month, limit_use_vegetables_per_month, limit_use_non_vegetables_per_month, benefit_per_ton, price_stock_per_ton, initial_stock));
        load_input_from_istream(inputs.back(), cin);
    }

    vector<float> times;

#ifdef normal
    for(int i = 0; i < inputs.size(); i++){
        times = vector<float>();
        for(int r = 0; r < repeats; r++){
            float solving_time = Solve(inputs[i]);
            if (solving_time < 0){
                cerr << "Input " << i << " Repeat " << r << " cut_pass " << cut_pass << " cut_bit_mask " << cut_bit_mask << " Ups!" << endl;
            }
            else{
                times.push_back(solving_time);
            }
        }
        cout << prune(times) << endl;
    }
#endif

#ifdef cuts_experiments
    ofstream output;
    output.open ("cuts_results.csv");

    vector<int> possible_cut_pass = {-1, 0, 100};
    
    for(int i = 0; i < inputs.size(); i++){
        times = vector<float>();
        for(auto _cut_pass : possible_cut_pass){
            cut_pass = _cut_pass;
            for(int cut_bit_mask=0; cut_bit_mask < 4; cut_bit_mask++){
                for(int r = 0; r < repeats; r++){
                    float solving_time = Solve(inputs[i]);
                    if (solving_time < 0){
                        cerr << "Input " << i << " Repeat " << r << " cut_pass " << cut_pass << " cut_bit_mask " << cut_bit_mask << " Ups!" << endl;
                    }
                    else{
                        times.push_back(solving_time);
                    }
                }
                output << prune(times) << ",";
            }
        }
        output << endl;
    }
#endif

#ifdef node_selection_experiments
    ofstream output;
    output.open ("node_selection_results.csv");

    for(int i = 0; i < inputs.size(); i++){
        times = vector<float>();
        for(node_selection_index = 0; node_selection_index < 4; node_selection_index++){
            for(int r = 0; r < repeats; r++){
                float solving_time = Solve(inputs[i]);
                if (solving_time < 0){
                    cerr << "Input " << i << " Repeat " << r << " node_selection_index " << node_selection_index << " Ups!" << endl;
                }
                else{
                    times.push_back(solving_time);
                }
            }
            output << prune(times) << ",";
        }
        output << endl;
    }
#endif

#ifdef heuristics_experiments
    ofstream output;
    output.open ("heuristics_results.csv");

    for(int i = 0; i < inputs.size(); i++){
        times = vector<float>();
        for(int bit = 0; bit < 4;bit ++){
            heuristics_bit_mask = (1<<bit);
            for(int r = 0; r < repeats; r++){
                float solving_time = Solve(inputs[i]);
                if (solving_time < 0){
                    cerr << "Input " << i << " Repeat " << r << " heuristics_bit_mask " << heuristics_bit_mask << " Ups!" << endl;
                }
                else{
                    times.push_back(solving_time);
                }
            }
            output << prune(times) << ",";
        }
        output << endl;
    }
#endif

    return 0;
}