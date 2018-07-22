#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <fstream>
#include <sstream>
#include <math.h>
#include <ilcplex/cplex.h>

using namespace std;


const int MWij_var_set_index = 0;
const int USOij_var_set_index = 1;
const int PRENDIDOij_var_set_index = 2;
const int APAGADOij_var_set_index = 3;


struct Generator{
    int MinMw;
    int MaxMw;
    int CostMinMw;
    int CostExtraMw;
    int CostStart;
    Generator(int _minMw,int _maxMw,int _costMin,int _extraCos,int _costStart) : MinMw(_minMw), MaxMw(MaxMw), CostMinMw(_costMin), CostExtraMw(_extraCos), CostStart(_costStart) {};
};

struct Range{
    int Init;
    int End;
    int DemandMw;
    Range(int _init, int _end, int _demandMw) : Init(_init), End(_end), DemandMw(_demandMw) {};
};


struct Problem{
	vector< Range > ranges;
	vector<Generator> generators;
	Problem(){}
};

void fillProblem(Problem *problem){
    int count_generator, count_ranges;
    cin >> count_generator >> count_ranges;

    int MinMw, MaxMw, CostMinMw, CostExtraMw, CostStart;
    for(int i = 0; i < count_generator; i++){
        cin >> MinMw >> MaxMw >> CostMinMw >> CostExtraMw >> CostStart;
        problem->generators.push_back(Generator(MinMw, MaxMw, CostMinMw, CostExtraMw, CostStart));
    }

    int Init, End, DemandMw;
    for(int i = 0; i < count_ranges; i++){
        cin >> Init >> End >> DemandMw;
        problem->ranges.push_back(Range(Init, End, DemandMw));
    }
}

int GetIndexForVariable(Problem * prob, int id_generator, int id_range, int id_set){
    int amountGenerators = prob->generators.size(), amountRanges = prob->ranges.size();
    int res =  amountGenerators*amountRanges*id_set + id_generator * amountRanges + id_range;
    cout << res << endl;
    return res;
}

int GetFuncObjCoefForVariable(Problem * prob, int id_generator, int id_range, int id_set){
    int objFuncCoef;
    switch(id_set){
        case 0:
            //Mwij
            objFuncCoef = prob->generators[id_generator].CostExtraMw;
            break;
        case 1:
            //Usoij
            objFuncCoef = prob->generators[id_generator].CostMinMw;
            break;
        case 2:
            //Prendidoij
            objFuncCoef = prob->generators[id_generator].CostStart;
            break;
        case 3:
            //Apagadoij
            objFuncCoef = 0;
            break;
        default:
            //should not execute this;
            cout << "Cant handle id_set of vars "<< id_set << endl;
            throw(1);
    }

    return objFuncCoef;
}

void solveMIP(Problem * prob){
    ////////////////////////////
    //Variables
    
    // Variables necesarias para manejar el ambiente y el problema en CPLEX
    CPXENVptr env = NULL;
    CPXLPptr lp = NULL;
    
    // Variable para guardar los codigos de error cada vez que se utiliza una rutina de CPLEX
    int status;
    
    // Variable para indicar cuantas variables va a tener nuestro modelo
    int cantvar = prob->ranges.size()*prob->generators.size()*4; // MWij, Usoij, Prendidoij, Apagadoij
    
    // Variables para definir las variables del modelo y la funcion objetivo
    double *obj=(double*)malloc(sizeof(double)*cantvar);
    double *lb=(double*)malloc(sizeof(double)*cantvar);
    double *ub=(double*)malloc(sizeof(double)*cantvar);
    char *coltype=(char*)malloc(sizeof(char)*cantvar);
    
    // Variables para definir las constraints del modelo
    int *rmatbeg=(int*)malloc(sizeof(int));
    int *rmatind=(int*)malloc(sizeof(int)*cantvar);
    double *rmatval=(double*)malloc(sizeof(double)*cantvar);
    double *rhs=(double*)malloc(sizeof(double)*1);
    char *sense=(char*)malloc(sizeof(char)*1);
    
    // Variables para traer los resultados
    double objval;
    double *x = NULL;
    double *y = NULL;
    double *pi = NULL;
    double *slack = NULL;
    double *dj = NULL;
    ////////////////////////////
    
    ////////////////////////////
    // Antes de hacer cualquier cosa, inicializamos cplex
    env = CPXopenCPLEX(&status);
    if(env==NULL)exit(-1);
    ////////////////////////////
    
    
    ////////////////////////////////////////////////////////////////////
    // Lista de varios parametros de CPLEX para personalizar su ejecucion
    
    status = CPXsetintparam(env, CPX_PARAM_DATACHECK, CPX_ON);
    if(status)exit(-1);
    
    /////////
    // Output
    status = CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON);
    if(status)exit(-1);

        
    /////////
    // Presolve
    status = CPXsetintparam (env, CPX_PARAM_MIPCBREDLP, CPX_OFF);
    if(status)exit(-1);
    
   
   

    /////////
    // Execution decisions
    
    status = CPXsetdblparam(env, CPX_PARAM_TILIM, 3600.0);
    if(status)exit(-1);

    status = CPXsetintparam(env, CPX_PARAM_THREADS, 8);
    if(status)exit(-1);

    status = CPXsetintparam(env, CPX_PARAM_NODESEL, CPX_NODESEL_BESTBOUND);
    if(status)exit(-1);
    
    
    status = CPXsetintparam(env, CPX_PARAM_MIPSEARCH, CPX_MIPSEARCH_TRADITIONAL);
    if(status)exit(-1);
   
    
    
    // Para setear un archivo de log
    //~ CPXFILEptr logfile = NULL;
   	//~ logfile = CPXfopen ("BPGC.log", "a");
    //~ CPXsetlogfile (env, logfile);
    
    

    // Creamos el problema en si en CPLEX
    lp = CPXcreateprob(env, &status, "energy");
    if(lp==NULL)exit(-1);
    
    
    // Definimos la funcion objetivo como una minimizacion
    CPXchgobjsen(env, lp, CPX_MIN);

    // Definimos la variables del problema con su valor en la funcion objetivo, lower bound, upper bound y coltype que indica el tipo de variable
    // CPLEX maneja un arreglo de variables. Para esto consideramos las variables en el orden X_11, X_12, X_13, .... , X_21, X_22,..., XN1, .... , Y_1 , Y_2...
    for (int generator=0; generator< prob->generators.size(); generator++){
       for(int range=0; range < prob->ranges.size(); range++){
			int index = GetIndexForVariable(prob, generator, range, 0);
            cout << index << endl;
			obj[index] = GetFuncObjCoefForVariable(prob, generator, range, 0);
    		lb[index] = 0.;
	        ub[index] = prob->generators[generator].MaxMw - prob->generators[generator].MinMw + 10;
	        coltype[index] = 'N';
	    }
    }

    for(int var_set = 1; var_set < 4; var_set++){
        for (int generator=0; generator< prob->generators.size(); generator++){
            for(int range=0; range < prob->ranges.size(); range++){
                int index = GetIndexForVariable(prob, generator, range, var_set);
                obj[index] = GetFuncObjCoefForVariable(prob, generator, range, var_set);
                lb[index] = 0.;
                ub[index] = 1.;
                coltype[index] = 'B';
            }
        }    
    }


    // Con esta llamada se crean todas las columnas de nuestro problema
    status = CPXnewcols(env, lp, cantvar, obj, lb, ub, coltype, NULL);
    if(status)exit(-1);

    // Demand Mw is satisfied in each range
    for(int range = 0; range < prob->ranges.size(); range++){
        int count = 0;
        rmatbeg[0] = 0;
        rhs[0] = prob->ranges[range].DemandMw;
        sense[0] = 'G';

        for(int generator = 0; generator < prob->generators.size(); generator++ ){

            //Add USO ij
            int index = GetIndexForVariable(prob, generator, range, USOij_var_set_index);
            rmatval[count] = prob->generators[generator].MinMw;
            rmatind[count] = index;
            count++;

            //add MW ij
            index = GetIndexForVariable(prob, generator, range, MWij_var_set_index);
            rmatval[count] = 1;
            rmatind[count] = index;
            count++;   
        }

        status = CPXaddrows(env, lp, 0, 1, count, rhs, sense, rmatbeg, rmatind, rmatval, NULL, NULL);
        if(status)exit(-1);
    }

    //A possible overload of demand could be attended
    for(int range = 0; range < prob->ranges.size(); range++){
        int count = 0;
        rmatbeg[0] = 0;
        rhs[0] = prob->ranges[range].DemandMw * 1.15;
        sense[0] = 'G';

        for(int generator = 0; generator < prob->generators.size(); generator++ ){
            //Add USO ij
            int index = GetIndexForVariable(prob, generator, range, USOij_var_set_index);
            rmatval[count] = prob->generators[generator].MaxMw * (prob->ranges[range].End - prob->ranges[range].Init);
            rmatind[count] = index;
            count++;
        }

        status = CPXaddrows(env, lp, 0, 1, count, rhs, sense, rmatbeg, rmatind, rmatval, NULL, NULL);
        if(status)exit(-1);
    }

    // MW ij is 0 when USO ij is 0
    for(int range = 0; range < prob->ranges.size(); range++){
        for(int generator = 0; generator < prob->generators.size(); generator++ ){
            rmatbeg[0] = 0;
            rhs[0] = 0;
            sense[0] = 'L';

            //Add USO ij
            rmatval[0] = prob->generators[generator].MinMw - prob->generators[generator].MaxMw;
            rmatind[0] = GetIndexForVariable(prob, generator, range, USOij_var_set_index);

            //add MW ij
            rmatval[1] = 1;
            rmatind[1] = GetIndexForVariable(prob, generator, range, MWij_var_set_index);

            status = CPXaddrows(env, lp, 0, 1, 2, rhs, sense, rmatbeg, rmatind, rmatval, NULL, NULL);
            if(status)exit(-1);
        }
    }

    // Add relation between PRENDIDO, USO and APAGADO
    for(int range = 1; range < prob->ranges.size(); range++){
        for(int generator = 0; generator < prob->generators.size(); generator++ ){
            rmatbeg[0] = 0;
            rhs[0] = 0;
            sense[0] = 'E';

            //Add USO ij
            rmatval[0] = 1;
            rmatind[0] = GetIndexForVariable(prob, generator, range, USOij_var_set_index);

            //Subtract PRENDIDO ij
            rmatval[1] = -1;
            rmatind[1] = GetIndexForVariable(prob, generator, range, PRENDIDOij_var_set_index);

            //Add APAGADO ij
            rmatval[2] = 1;
            rmatind[2] = GetIndexForVariable(prob, generator, range, APAGADOij_var_set_index);

            //Add previous USO
            rmatval[3] = -1;
            rmatind[3] = GetIndexForVariable(prob, generator, range-1, USOij_var_set_index);
            
            status = CPXaddrows(env, lp, 0, 1, 4, rhs, sense, rmatbeg, rmatind, rmatval, NULL, NULL);
            if(status)exit(-1);
        }
    }

    // First relation between USO and PRENDIDO
    for(int generator = 0; generator < prob->generators.size(); generator++ ){
        rmatbeg[0] = 0;
        rhs[0] = 0;
        sense[0] = 'E';

        //Add USO ij
        rmatval[0] = 1;
        rmatind[0] = GetIndexForVariable(prob, generator, 0, USOij_var_set_index);

        //Subtract PRENDIDO ij
        rmatval[1] = -1;
        rmatind[1] = GetIndexForVariable(prob, generator, 0, PRENDIDOij_var_set_index);
        
        status = CPXaddrows(env, lp, 0, 1, 2, rhs, sense, rmatbeg, rmatind, rmatval, NULL, NULL);
        if(status)exit(-1);
    }


    /////////////////////////////
    // Una vez armado todo, se hace la siguiente llamada para darle el control a CPLEX y que resuelva todo el modelo
    status = CPXmipopt(env,lp);
    if(status)exit(-1);
    /////////////////////////////
    
    
    status = CPXgetobjval (env, lp, &objval); // Para traer el mejor valor obtenido
    if(status)exit(-1);
    
    
    // Pido memoria para traer la solucion
    int cur_numrows = CPXgetnumrows(env,lp);
    int cur_numcols = CPXgetnumcols(env,lp);
    x = (double *) malloc(cur_numcols * sizeof(double));
    
    
    // Pedimos los valores de las variables en la solucion
    status = CPXgetx (env, lp, x, 0, cur_numcols-1);
    if(status)exit(-1);

    
    // Armamos la asignacion de colores en nuestra estructura con la informacion que nos trajimos de CPLEX
    cout << "objval" << objval << endl;

    for (int range=0; range < prob->ranges.size(); range++){
        cout << "Range "<< range << "(" << prob->ranges[range].Init << ", " << prob->ranges[range].End << "): \n";

        //if a generator is used, it is printed with the extra
        for (int generator=0; generator< prob->generators.size(); generator++){
            int index = GetIndexForVariable(prob, generator, range, USOij_var_set_index);
        	if(x[index]){
                cout << "Generator " << generator << " : " << x[GetIndexForVariable(prob, generator, range, MWij_var_set_index)];
                if(x[GetIndexForVariable(prob, generator, range, PRENDIDOij_var_set_index)] == 1){
                    cout << " *";
                }
                cout << "\n";
            }
        }
    }
    
    return;
}

int main(int argc, char** argv){
    Problem * prob = new Problem();
    fillProblem(prob);
    solveMIP(prob);
    return 0;
}
