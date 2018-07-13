#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <fstream>
#include <sstream>
#include <math.h>
#include <ilcplex/cplex.h>

using namespace std;


struct Farm{
    int east;
    int north;
    bool everyDay;
    int toCollect;
};

struct Problema{
	vector< vector<float> > distances;
	map<int,Farm> farmsInfo;
	int farmsQty;
	Problema(){}
};

vector< vector<int> > getPowerSet(int *set, int set_size)
{
    /*set_size of power set of a set with set_size
      n is (2**n -1)*/
    unsigned int pow_set_size = pow(2, set_size);
    int counter, j;
 
    /*Run from counter 000..0 to 111..1*/
    vector< vector<int> > power_set;
    for(counter = 0; counter < pow_set_size; counter++)
    {
    	vector<int> s;
      for(j = 0; j < set_size; j++)
       {
          /* Check if jth bit in the counter is set
             If set then pront jth element from set */
          if(counter & (1<<j))
          	s.push_back(set[j]);
       }
       power_set.push_back(s);
    }
    return power_set;
}
 

int getIndexFromEdge(int from, int to, int day, int length){
	int count = 0;
	for (int i = 0; i < length; i++){
    	for(int j=0; j<length; j++){
    		if (i != j) {
    			if (i == from && j == to){
    				return count+day*length*(length-1);
	    		}
	    		count++;
    		}
    	}
	}
	return -1;
}

bool getBooleanValue(string s) {
    if (s == "true") {
        return true;
    }
    return false;
}

int getIntValue(string s) {
    return stoi(s);
}

float getDistances(Farm f1, Farm f2) {
    return sqrt(pow(f1.east - f2.east,2) + pow(f1.north - f2.north,2));
}

map<int,Farm> getFarmInfo() {
    ifstream file ( "SmallFarms.csv" );
    string line;
    string value;

    map<int, Farm> m;
    while ( file.good() ){
        getline( file, line, '\n' );
        stringstream ss(line);

        vector<string> farmValues;
        while(getline( ss, value, ',' )){
            farmValues.push_back(value);   
        }

        Farm f;
        f.east = getIntValue(farmValues[1]);
        f.north = getIntValue(farmValues[2]);
        f.everyDay = getBooleanValue(farmValues[3]);
        
        f.toCollect = getIntValue(farmValues[4]);

        m[getIntValue(farmValues[0])-1] = f;
    }

    return m;
}

vector< vector<float> > createDistancesMatrix(map<int,Farm> farmsInfo){
    vector< vector<float> > distances(farmsInfo.size(), vector<float> (farmsInfo.size(), 0));
    for (map<int,Farm>::iterator it=farmsInfo.begin(); it!=farmsInfo.end(); ++it){
        for (map<int,Farm>::iterator it2=farmsInfo.begin(); it2!=farmsInfo.end(); ++it2){
            distances[it->first][it2->first] = getDistances(it->second, it2->second);
        }
    }
    
    return distances;
}


void solveMIP(Problema * prob){
    ////////////////////////////
    //Variables
    
    // Variables necesarias para manejar el ambiente y el problema en CPLEX
    CPXENVptr env = NULL;
    CPXLPptr lp = NULL;
    
    // Variable para guardar los codigos de error cada vez que se utiliza una rutina de CPLEX
    int status;
    
    // Variable para indicar cuantas variables va a tener nuestro modelo
    int cantvar = prob->farmsQty*(prob->farmsQty-1)*2;
    
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
    lp = CPXcreateprob(env, &status, "milkCollection");
    if(lp==NULL)exit(-1);
    
    
    // Definimos la funcion objetivo como una minimizacion
    CPXchgobjsen(env, lp, CPX_MIN);

    // Definimos la variables del problema con su valor en la funcion objetivo, lower bound, upper bound y coltype que indica el tipo de variable
    // CPLEX maneja un arreglo de variables. Para esto consideramos las variables en el orden X_11, X_12, X_13, .... , X_21, X_22,..., XN1, .... , Y_1 , Y_2...
    for (int days=0; days<2; days++){
    	for(int i=0;i<prob->farmsQty;i++){
	    	for(int j=0;j<prob->farmsQty;j++){
	    		if (i != j) {
	    			int index = getIndexFromEdge(i,j,days,prob->farmsQty);
	    			obj[index] = prob->distances[i][j];
		    		lb[index] = 0.;
			        ub[index] = 1.;
			        coltype[index] = 'B';
	    		}
	    	}
	    }
    }

 
    
    // Con esta llamada se crean todas las columnas de nuestro problema
    status = CPXnewcols(env, lp, cantvar, obj, lb, ub, coltype, NULL);
    if(status)exit(-1);

    
    ////////////////////////////
    // Una vez armadas todas las variables  y la funcion objetivo, faltan las restricciones
    
    // capacidad maxima
    for (int days=0; days<2; days++){
    	int count = 0;
	    rmatbeg[0] = 0;
        rhs[0] = 80000.0;
        sense[0] = 'L';
	    for(int i=0;i<prob->farmsQty;i++){
	    	for(int j=0;j<prob->farmsQty;j++){
	    		if(i!=j){
	    			int index = getIndexFromEdge(i,j,days,prob->farmsQty);
		    		rmatval[count] = prob->farmsInfo[i].toCollect;
		    		rmatind[count] = index;
		    		count++;
	    		}
	    	}
	    }

	    status = CPXaddrows(env, lp, 0, 1, count, rhs, sense, rmatbeg, rmatind, rmatval, NULL, NULL);
        if(status)exit(-1);
    }
	
    /// every day - salen de una granja
    for (int days=0; days<2; days++){
		for(int i=0;i<prob->farmsQty;i++){
			if (!prob->farmsInfo[i].everyDay) {
				continue;
			}
			int count = 0;
		    rmatbeg[0] = 0;
		    sense[0] = 'E';
			rhs[0] = 1;
			for(int j=0;j<prob->farmsQty;j++){
				if (i!=j){
					int index = getIndexFromEdge(i,j, days, prob->farmsQty);
		    		rmatval[count] = 1.0;
		    		rmatind[count] = index;
		    		count++;
				}
			}
			status = CPXaddrows(env, lp, 0, 1, count, rhs, sense, rmatbeg, rmatind, rmatval, NULL, NULL);
			if(status)exit(-1);	
		}
	}

	/// every day - entran una granja
    for (int days=0; days<2; days++){
		for(int j=0;j<prob->farmsQty;j++){
			if (!prob->farmsInfo[j].everyDay) {
				continue;
			}
			int count = 0;
		    rmatbeg[0] = 0;
		    sense[0] = 'E';
			rhs[0] = 1;
			for(int i=0;i<prob->farmsQty;i++){
				if (i!=j){
					int index = getIndexFromEdge(i,j, days, prob->farmsQty);
		    		rmatval[count] = 1.0;
		    		rmatind[count] = index;
		    		count++;
				}
			}
			status = CPXaddrows(env, lp, 0, 1, count, rhs, sense, rmatbeg, rmatind, rmatval, NULL, NULL);
			if(status)exit(-1);	
		}
	}
/*
	// someday - salen de una granja
	for(int i=0;i<prob->farmsQty;i++){
		if (prob->farmsInfo[i].everyDay) {
			continue;
		}
		int count = 0;
	    rmatbeg[0] = 0;
	    sense[0] = 'E';
		rhs[0] = 1;
		for(int j=0;j<prob->farmsQty;j++){
			if (i!=j){
				int index = getIndexFromEdge(i,j, 0, prob->farmsQty);
				int indexDay2 = getIndexFromEdge(i,j, 1, prob->farmsQty);
	    		rmatval[count] = 1;
	    		rmatind[count] = index;
	    		count++;
	    		rmatval[count] = 1;
	    		rmatind[count] = indexDay2;
	    		count++;
			}
		}
		status = CPXaddrows(env, lp, 0, 1, count, rhs, sense, rmatbeg, rmatind, rmatval, NULL, NULL);
		if(status)exit(-1);	
	}
    */
/*
	// someday - entran de una granja
	for(int j=0;j<prob->farmsQty;j++){
		if (prob->farmsInfo[j].everyDay) {
			continue;
		}
		int count = 0;
	    rmatbeg[0] = 0;
	    sense[0] = 'E';
		rhs[0] = 1;
		for(int i=0;i<prob->farmsQty;i++){
			if (i!=j){
				int index = getIndexFromEdge(i,j, 0, prob->farmsQty);
				int indexDay2 = getIndexFromEdge(i,j, 1, prob->farmsQty);
	    		rmatval[count] = 1.0;
	    		rmatind[count] = index;
	    		count++;
	    		rmatval[count] = 1.0;
	    		rmatind[count] = indexDay2;
	    		count++;
			}
		}
		status = CPXaddrows(env, lp, 0, 1, count, rhs, sense, rmatbeg, rmatind, rmatval, NULL, NULL);
		if(status)exit(-1);	
	}
*/
	// subtour
	int farms[prob->farmsQty];
	for (int i = 0; i < prob->farmsQty; i++){
		farms[i] = i;
	}


	vector< vector<int> > powerSet = getPowerSet(farms, prob->farmsQty);

	for (int days=0; days<2; days++){
		for(int s = 0; s<powerSet.size(); s++){
			if(powerSet[s].size() <= 1 || powerSet[s].size() == prob->farmsQty){
				continue;
			}
			int count = 0;
		    rmatbeg[0] = 0;
	        rhs[0] = powerSet[s].size()-1;
	        sense[0] = 'L';
		    for(int i=0;i<powerSet[s].size();i++){
		    	for(int j=0;j<powerSet[s].size();j++){
		    		int from = powerSet[s][i];
		    		int to = powerSet[s][j];
		    		if(from!=to){
		    			int index = getIndexFromEdge(from,to,days,prob->farmsQty);
			    		rmatval[count] = 1.0;
			    		rmatind[count] = index;
			    		count++;
			    		cout << from << "-" << to << "+";
			    		
		    		}
		    	}
		    }
		    status = CPXaddrows(env, lp, 0, 1, count, rhs, sense, rmatbeg, rmatind, rmatval, NULL, NULL);
        	if(status)exit(-1);

        	cout << "<=" << powerSet[s].size()-1 << endl;
		}

	}

    /////////////////////////////
    // Una vez armado todo, se hace la siguiente llamada para darle el control a CPLEX y que resuelva todo el modelo
    status = CPXmipopt(env,lp);
    if(status)exit(-1);
    /////////////////////////////
    
    
    status = CPXgetobjval (env, lp, &objval); // Para traer el mejor valor obtenido
    if(status)exit(-1);
    
    
    
    // Pido memoria para traer la solucion
    int cur_numrows=CPXgetnumrows(env,lp);
    int cur_numcols=CPXgetnumcols(env,lp);
    x = (double *) malloc(cur_numcols * sizeof(double));
    
    
    // Pedimos los valores de las variables en la solucion
    status = CPXgetx (env, lp, x, 0, cur_numcols-1);
    if(status)exit(-1);

    
    // Armamos la asignacion de colores en nuestra estructura con la informacion que nos trajimos de CPLEX
    cout << "objval" << objval << endl;

    for (int days=0; days<2; days++){
    	cout << "dia " << days << endl;
    	for(int i=0;i<prob->farmsQty;i++){
	    	for(int j=0;j<prob->farmsQty;j++){
	    		if (i != j){
	    			int index = getIndexFromEdge(i,j,days,prob->farmsQty);
	    			if (x[index]) {
	    				cout << "(" << i << "," << j << ")" << endl;
	    			}
	    			
	    		}	    		
	    	}
	    }
    }
    
    return;
}

int main(int argc, char** argv){
    Problema * prob = new Problema();
    map<int,Farm> farms = getFarmInfo();
    vector< vector<float> > distances = createDistancesMatrix(farms);
    prob->distances = distances;
    prob->farmsInfo = farms;
	prob->farmsQty = farms.size();
    solveMIP(prob);
    return 0;
}
