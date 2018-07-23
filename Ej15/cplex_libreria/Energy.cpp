#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <fstream>
#include <sstream>
#include <math.h>
#include <ilcplex/cplex.h>

using namespace std;

const int ExtraMW_offset = 0;
const int Used_offset = 1;
const int Started_offset = 2;
const int ShuttedDown_offset = 3;

struct GeneratorType{
    int available;
    int min_mw;
    int max_mw;
    float cost_min_mw;
    float cost_extra_mw;
    float cost_start;
    GeneratorType(int _available, int _minMw,int _maxMw,float _costMin,float _extraCos,float _costStart) : available(_available), min_mw(_minMw), max_mw(_maxMw), cost_min_mw(_costMin), cost_extra_mw(_extraCos), cost_start(_costStart) {};
};

std::ostream& operator<<(std::ostream& os, GeneratorType &gt)
    {
        os << "available: " << gt.available << "\n";
        os << "min_mw: " << gt.min_mw << "\n";
        os << "max_mw: " << gt.max_mw << "\n";
        os << "cost_min_mw: " << gt.cost_min_mw << "\n";
        os << "cost_extra_mw: " << gt.cost_extra_mw << "\n";
        os << "cost_start: " << gt.cost_start << "\n";
        return os;
    }

struct Problem{
    vector<float> demands_per_hour;
	vector<GeneratorType> generators;
	Problem(){}
};

void printProblem(Problem *problem){
    for(int i = 0; i < problem->generators.size(); i++){
        cout << "GeneratorType " << i << "\n";
        cout << problem->generators[i];
        cout << "\n";
    }

    cout << "Demands\n" << "[";
    for(auto x : problem->demands_per_hour){
        cout << x << ", ";
    }
    cout << "]\n";
}

void fillProblem(Problem *problem){
    int count_generator, count_hours;
    cin >> count_generator >> count_hours;

    int hours, demand_per_hour_in_mw;
    while(count_hours--){
        cin >> hours >> demand_per_hour_in_mw;
        while(hours--) problem->demands_per_hour.push_back(demand_per_hour_in_mw);
    }

    int available, min_mw, max_mw;
    float cost_min_mw, cost_extra_mw, cost_start;
    while(count_generator--){
        cin >> available >> min_mw >> max_mw >> cost_min_mw >> cost_extra_mw >> cost_start;
        problem->generators.push_back(GeneratorType(available, min_mw, max_mw, cost_min_mw, cost_extra_mw, cost_start));
    }
}

/*
1 <= hour <= 24
1 <= generator_type <= problem->generators.size() 
*/
int GetIndexForVariable(Problem * prob, int generator_type, int hour, int offset_set_of_variables){
    int amountGeneratorTypes = prob->generators.size();
    int hours = prob->demands_per_hour.size();
    int res =  amountGeneratorTypes*hours*offset_set_of_variables + generator_type * hours + hour; // 24 hours per type
    //cout << res << endl;
    return res;
}

float GetFuncObjCoefForVariable(Problem * prob, int id_generator, int hour, int offset_set_of_variables){
    float objFuncCoef;
    switch(offset_set_of_variables){
        case 0:
            //ExtraMw ij
            objFuncCoef = prob->generators[id_generator].cost_extra_mw;
            break;
        case 1:
            //Used ij
            objFuncCoef = prob->generators[id_generator].cost_min_mw;
            break;
        case 2:
            //Started ij
            objFuncCoef = prob->generators[id_generator].cost_start;
            break;
        case 3:
            //ShuttedDown ij
            objFuncCoef = 0;
            break;
        default:
            //should not execute this;
            cout << "Cant handle offset_set_of_variables "<< offset_set_of_variables << endl;
            throw(1);
    }

    return objFuncCoef;
}

void solveMIP(Problem * prob){
    ////////////////////////////
    //Vars
    
    CPXENVptr env = NULL;
    CPXLPptr lp = NULL;
    
    int status;
    
    // (forall i in GeneratorTypes, j in Hours) define in order ExtraMW_{i j}, Used_{i j}, Started_{i j}, ShuttedDown_{i j}
    int cantvar = prob->generators.size()*prob->demands_per_hour.size()*4; //24 hours, 4 times
    
    double *obj=(double*)malloc(sizeof(double)*cantvar);
    double *lb=(double*)malloc(sizeof(double)*cantvar);
    double *ub=(double*)malloc(sizeof(double)*cantvar);
    char *coltype=(char*)malloc(sizeof(char)*cantvar);
    
    int *rmatbeg=(int*)malloc(sizeof(int));
    int *rmatind=(int*)malloc(sizeof(int)*cantvar);
    double *rmatval=(double*)malloc(sizeof(double)*cantvar);
    double *rhs=(double*)malloc(sizeof(double)*1);
    char *sense=(char*)malloc(sizeof(char)*1);
    
    double objval;
    double *x = NULL;
    double *y = NULL;
    double *pi = NULL;
    double *slack = NULL;
    double *dj = NULL;
    
    env = CPXopenCPLEX(&status);
    if(env==NULL)exit(-1);
    
    status = CPXsetintparam(env, CPX_PARAM_DATACHECK, CPX_ON);
    if(status)exit(-1);
    
    status = CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON);
    if(status)exit(-1);

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
       for(int hour=0; hour < prob->demands_per_hour.size(); hour++){
			int index = GetIndexForVariable(prob, generator, hour, ExtraMW_offset);
			obj[index] = GetFuncObjCoefForVariable(prob, generator, hour, ExtraMW_offset);
    		lb[index] = 0.;
	        ub[index] = (prob->generators[generator].max_mw - prob->generators[generator].min_mw) * prob->generators[generator].available;
	        coltype[index] = 'S';
	    }
    }

    for(int var_offset = 1; var_offset < 4; var_offset++){
        for (int generator=0; generator< prob->generators.size(); generator++){
            for(int hour=0; hour < prob->demands_per_hour.size(); hour++){
                int index = GetIndexForVariable(prob, generator, hour, var_offset);
                obj[index] = GetFuncObjCoefForVariable(prob, generator, hour, var_offset);
                lb[index] = 0.;
                ub[index] = prob->generators[generator].available + 10;
                coltype[index] = 'I';
            }
        }    
    }


    // Con esta llamada se crean todas las columnas de nuestro problema
    status = CPXnewcols(env, lp, cantvar, obj, lb, ub, coltype, NULL);
    if(status)exit(-1);

    /* Constrains */

    // Demand is satisfied in each hour
    for(int hour = 0; hour < prob->demands_per_hour.size(); hour++){
        int count = 0;
        rmatbeg[0] = 0;
        rhs[0] = prob->demands_per_hour[hour];
        sense[0] = 'G';

        for(int generator = 0; generator < prob->generators.size(); generator++ ){
            //Add the amount of Used ij times the min_mw of each generator type 
            int index = GetIndexForVariable(prob, generator, hour, Used_offset);
            rmatval[count] = prob->generators[generator].min_mw;
            rmatind[count] = index;
            count++;

            //add ExtraMw ij
            index = GetIndexForVariable(prob, generator, hour, ExtraMW_offset);
            rmatval[count] = 1;
            rmatind[count] = index;
            count++;   
        }

        status = CPXaddrows(env, lp, 0, 1, count, rhs, sense, rmatbeg, rmatind, rmatval, NULL, NULL);
        if(status)exit(-1);
    }

    //A possible overload of demand (Demand * 1.15) in each hour could be attended
    for(int hour = 0; hour < prob->demands_per_hour.size(); hour++){
        int count = 0;
        rmatbeg[0] = 0;
        rhs[0] = prob->demands_per_hour[hour] * 1.15;
        sense[0] = 'G';

        for(int generator = 0; generator < prob->generators.size(); generator++ ){
            int index = GetIndexForVariable(prob, generator, hour, Used_offset);
            rmatval[count] = prob->generators[generator].max_mw;
            rmatind[count] = index;
            count++;
        }

        status = CPXaddrows(env, lp, 0, 1, count, rhs, sense, rmatbeg, rmatind, rmatval, NULL, NULL);
        if(status)exit(-1);
    }

    // ExtraMW ij <= Used ij * (MaxMw i - MinMw i)
    for(int hour = 0; hour < prob->demands_per_hour.size(); hour++){
        for(int generator = 0; generator < prob->generators.size(); generator++ ){
            rmatbeg[0] = 0;
            rhs[0] = 0;
            sense[0] = 'L';

            //Add Used ij substracting
            rmatval[0] = prob->generators[generator].min_mw - prob->generators[generator].max_mw;
            rmatind[0] = GetIndexForVariable(prob, generator, hour, Used_offset);

            //add ExtraMW ij adding
            rmatval[1] = 1;
            rmatind[1] = GetIndexForVariable(prob, generator, hour, ExtraMW_offset);

            status = CPXaddrows(env, lp, 0, 1, 2, rhs, sense, rmatbeg, rmatind, rmatval, NULL, NULL);
            if(status)exit(-1);
        }
    }

    // Add relation between PRENDIDO, USO and APAGADO
    for(int hour = 0; hour < prob->demands_per_hour.size()-1; hour++){
        for(int generator = 0; generator < prob->generators.size(); generator++ ){
            rmatbeg[0] = 0;
            rhs[0] = 0;
            sense[0] = 'E';

            //Add Used i j+1
            rmatval[0] = 1;
            rmatind[0] = GetIndexForVariable(prob, generator, hour+1, Used_offset);

            //Subtract Started i j+1
            rmatval[1] = -1;
            rmatind[1] = GetIndexForVariable(prob, generator, hour+1, Started_offset);

            //Add ShuttedDown ij
            rmatval[2] = 1;
            rmatind[2] = GetIndexForVariable(prob, generator, hour+1, ShuttedDown_offset);

            //Add previous Used
            rmatval[3] = -1;
            rmatind[3] = GetIndexForVariable(prob, generator, hour, Used_offset);
            
            status = CPXaddrows(env, lp, 0, 1, 4, rhs, sense, rmatbeg, rmatind, rmatval, NULL, NULL);
            if(status)exit(-1);
        }
    }

    // Used ij + Started i j+1 <= available i
    for(int hour = 0; hour < prob->demands_per_hour.size()-1; hour++){
        for(int generator = 0; generator < prob->generators.size(); generator++ ){
            rmatbeg[0] = 0;
            rhs[0] = prob->generators[generator].available;
            sense[0] = 'L';

            //Subtract Started i j+1
            rmatval[0] = 1;
            rmatind[0] = GetIndexForVariable(prob, generator, hour+1, Started_offset);

            //Add previous Used
            rmatval[1] = 1;
            rmatind[1] = GetIndexForVariable(prob, generator, hour, Used_offset);
            
            status = CPXaddrows(env, lp, 0, 1, 2, rhs, sense, rmatbeg, rmatind, rmatval, NULL, NULL);
            if(status)exit(-1);
        }
    }

    // Used i 0 == Started i 0
    for(int generator = 0; generator < prob->generators.size(); generator++ ){
        rmatbeg[0] = 0;
        rhs[0] = 0;
        sense[0] = 'E';

        //Add USO ij
        rmatval[0] = 1;
        rmatind[0] = GetIndexForVariable(prob, generator, 0, Used_offset);

        //Subtract PRENDIDO ij
        rmatval[1] = -1;
        rmatind[1] = GetIndexForVariable(prob, generator, 0, Started_offset);
        
        status = CPXaddrows(env, lp, 0, 1, 2, rhs, sense, rmatbeg, rmatind, rmatval, NULL, NULL);
        if(status)exit(-1);
    }

    // Used i 0 <= available i
    for(int hour = 0; hour < prob->demands_per_hour.size(); hour++){
        for(int generator = 0; generator < prob->generators.size(); generator++ ){
            rmatbeg[0] = 0;
            rhs[0] = prob->generators[generator].available;
            sense[0] = 'L';

            //Add USO ij
            rmatval[0] = 1;
            rmatind[0] = GetIndexForVariable(prob, generator, hour, Used_offset);
            
            status = CPXaddrows(env, lp, 0, 1, 1, rhs, sense, rmatbeg, rmatind, rmatval, NULL, NULL);
            if(status)exit(-1);
        }
    }

    // // All >= 0
    // for(int offset = 0; offset < 4; offset++){
    //     for(int hour = 0; hour < prob->demands_per_hour.size(); hour++){
    //         for(int generator = 0; generator < prob->generators.size(); generator++ ){
    //             rmatbeg[0] = 0;
    //             rhs[0] = 0;
    //             sense[0] = 'G';
    //             rmatval[0] = 1;
    //             rmatind[0] = GetIndexForVariable(prob, generator, hour, offset);
    //             status = CPXaddrows(env, lp, 0, 1, 1, rhs, sense, rmatbeg, rmatind, rmatval, NULL, NULL);
    //             if(status)exit(-1);
    //         }
    //     }
    // }


    /////////////////////////////
    // Una vez armado todo, se hace la siguiente llamada para darle el control a CPLEX y que resuelva todo el modelo
    status = CPXmipopt(env,lp);
    if(status)exit(-1);
    /////////////////////////////
    cout << "resuelto" << endl;
    
    status = CPXgetobjval (env, lp, &objval); // Para traer el mejor valor obtenido
    if(status)exit(-1);
    cout << "valores" << endl;
    
    // Pido memoria para traer la solucion
    int cur_numrows = CPXgetnumrows(env,lp);
    int cur_numcols = CPXgetnumcols(env,lp);
    x = (double *) malloc(cur_numcols * sizeof(double));
    
    
    // Pedimos los valores de las variables en la solucion
    status = CPXgetx (env, lp, x, 0, cur_numcols-1);
    if(status)exit(-1);
    cout << "variables" << endl;
    
    // Armamos la asignacion de colores en nuestra estructura con la informacion que nos trajimos de CPLEX
    cout << "objval" << objval << endl;

    for (int hour=0; hour < prob->demands_per_hour.size(); hour++){
        cout << "Hour "<< hour << ": ";
        float acumMW = 0;
        //if a generator is used, it is printed with the extra
        for (int generator=0; generator< prob->generators.size(); generator++){
            int index = GetIndexForVariable(prob, generator, hour, Used_offset);
        	if(x[index]){
                //cout << "GeneratorType: " << generator << "\n";
                cout << x[index] << " ";// << x[GetIndexForVariable(prob, generator, hour, ExtraMW_offset)] << " ";
                acumMW += x[index] * prob->generators[generator].min_mw;
                //index = x[GetIndexForVariable(prob, generator, hour, Started_offset)];
                //cout << "Started: " << x[index] << "\n";
                acumMW += x[GetIndexForVariable(prob, generator, hour, ExtraMW_offset)];
            }
        }
        cout << "\n" << "TotalMW: "<< acumMW << "\n";
    }
    
    return;
}



int main(int argc, char** argv){
    Problem * prob = new Problem();
    fillProblem(prob);
    //printProblem(prob);
    solveMIP(prob);
    return 0;
}
