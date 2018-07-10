#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <fstream>
#include <sstream>
#include <math.h>
#include <ilcplex/cplex.h>

using namespace std;


struct Problema{
	vector< vector<float> > distances;
	map<int,Farm> farmsInfo;
	int farmsQty;
	Problema(){}
};


struct Farm{
    int east;
    int north;
    bool everyDay;
    int toCollect;
};

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
    ifstream file ( "Farms.csv" );
    string line;
    string value;

    map<int, Farm> m;
    while ( file.good() ){
        getline( file, line, '\n' );
        stringstream ss(line);

        vector<string> farmValues;
        while(getline( ss, value, ',' )){
            farmValues.push_back(value);
            cout << value << endl;    
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

vector< vector<float> > createDistancesMatrix(map<int,Farm> farmData){
    vector< vector<float> > distances(farmData.size(), vector<float> (farmData.size(), 0));
    for (map<int,Farm>::iterator it=farmData.begin(); it!=farmData.end(); ++it){
        for (map<int,Farm>::iterator it2=farmData.begin(); it!=farmData.end(); ++it){
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
    int cantvar = prob->farmsQty*(farmsQty-1);
    
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
    for(int i=0;i<cantvar;i++){
        // Las variables Y tiene un 1 en la funcion objetivo y las X un 0
        obj[i] = (i>=prob->N*prob->bestObj)?1.:0.;
        
        // Todas las variables son binarias con cotas 0 y 1.
        lb[i] = 0.;
        ub[i] = 1.;
        coltype[i] = 'B';
    }
    
    
    // Con esta llamada se crean todas las columnas de nuestro problema
    status = CPXnewcols(env, lp, cantvar, obj, lb, ub, coltype, NULL);
    if(status)exit(-1);
    
    
    ////////////////////////////
    // Una vez armadas todas las variables  y la funcion objetivo, faltan las restricciones
    
    
    //Primera restriccion: Dos items que son vecinos no pueden tener un mismo color k
    // X_ik + X_jk <= Y_k
    for(int i=0;i<prob->N;i++){//Recorro los vertices
        for(int j=0;j<(int)prob->ejes[i].size();j++){ // Por cada vertice miro solo sus vecinos
            if(i>=prob->ejes[i][j])continue; // Esta linea esta para definir una sola vez la restriccion para cada par de vertices
            for(int k=0;k<prob->bestObj;k++){ // Recorro todos los colores
                
                // Como CPLEX deja meter muchas restricciones en una misma llamada, hace falta un arreglo rmatbeg que indique cada restriccion que ponemos donde comienza
                // En general siempre pondremos una sola restriccion en cada llamada por lo que este arreglo tendra una sola posicion con valor 0
                rmatbeg[0] = 0;
                
                rhs[0] = 0.0;// El valor del lado derecho de la restriccion
                sense[0] = 'L'; // El sentido de la restriccion (menor igual, igual, mayor igual)
                
                
                // Ahora solo falta indicar que variables estan en el lado izquierdo y con que valor.
                // rmatval tendra el valor de la variable
                // rmatind tendra el indice de la variable
                
                rmatval[0] = 1.0;
                rmatind[0] = i*prob->bestObj+k;
                rmatval[1] = 1.0;
                rmatind[1] = prob->ejes[i][j]*prob->bestObj+k;
                rmatval[2] = -1.0;
                rmatind[2] = prob->N*prob->bestObj+k;
                
                // Una vez armada toda la restriccion se hace la siguiente llamada para agregarla al modelo
                status = CPXaddrows(env, lp, 0, 1, 3, rhs, sense, rmatbeg, rmatind, rmatval, NULL, NULL);
                if(status)exit(-1);
            }
        }
    }
    
    
    //Restriccion de un color por nodo
    //Sum_k X_ik = 1 para todo i
    for(int i=0;i<prob->N;i++){
        rmatbeg[0] = 0;
        rhs[0] = 1.0;
        sense[0] = 'E';
        int count = 0;
        for(int j=0;j<prob->bestObj;j++){
            rmatval[count] = 1.0;
            rmatind[count++] = i*prob->bestObj+j;
        }
        status = CPXaddrows(env, lp, 0, 1, count, rhs, sense, rmatbeg, rmatind, rmatval, NULL, NULL);
        if(status)exit(-1);
    }
    
    // Restriccion que rompe simetria
    // No hace falta para que el modelo sea correcto, pero se la podemos agregar como restriccion para obtener poliedros mas chicos
    // Y_k >= Y_k+1
    for(int i=0;i<prob->bestObj-1;i++){
        rmatbeg[0] = 0;
        rhs[0] = 0.;
        sense[0] = 'G';
        rmatval[0] = 1.0;
        rmatind[0] = prob->N*prob->bestObj+i;
        rmatval[1] = -1.0;
        rmatind[1] = prob->N*prob->bestObj+i+1;
        status = CPXaddrows(env, lp, 0, 1, 2, rhs, sense, rmatbeg, rmatind, rmatval, NULL, NULL);
        if(status)exit(-1);
    }
    

    /////////////////////////////
    // Este sector de codigo permite proveerle a CPLEX una solucion factible de entrada
    status = CPXsetintparam (env, CPX_PARAM_ADVIND, 1);
    if(status)exit(-1);
    
    
    int beg[1] = {0};
    int nzcnt = prob->N+prob->bestObj;
    int* varindices = (int*)malloc(sizeof(int)*nzcnt);
    for(int i=0;i<prob->N;i++){
        varindices[i] = i*prob->bestObj+prob->colores[i];
    }
    for(int i=0;i<prob->bestObj;i++){
        varindices[i+prob->N] = i+prob->N*prob->bestObj;
    }
    
    
    double* values = (double*)malloc(sizeof(double)*nzcnt);
    for(int i=0;i<nzcnt;++i)values[i]=1.0;
    int effortlevel[1] = {CPX_MIPSTART_AUTO};
    status = CPXaddmipstarts(env, lp, 1, nzcnt, beg, varindices, values, effortlevel, NULL);

    
    free(varindices);
    free(values);
    /////////////////////////////
    
    
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
    if(prob->bestObj>=objval){
        for(int i=0; i<prob->N; ++i){
            for(int j=0; j<prob->bestObj; ++j){
                if(x[i*prob->bestObj+j]>0.001){
                    prob->colores[i]=j;
                }
            }
        }
        prob->bestObj = objval;
    }
    
    
    return;
}

int main(int argc, char** argv){
    Problema * prob = new Problema();
    map<int,Farm> farms = getFarmInfo();
    vector< vector<float> > distances = createDistancesMatrix(farms);
    prob->distances = distances;
    //prob->farmsInfo = farms;
	prob->farmsQty = farms.size();
    solveMIP(prob);
    return 0;
}