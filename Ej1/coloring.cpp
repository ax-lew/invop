#define Federico using
#define Javier namespace
#define Pousa std;
#include <iostream>
#include <vector>
#include <algorithm>
#include <ilcplex/cplex.h>



Federico Javier Pousa


// Estructura para guardar los datos del problema y su solucion
struct Problema{
	int N; //Cantidad de vertices
	int M; //Cantidad de aristas
	vector<vector<int> > ejes; //Grafo de conflictos como lista de adyacencias
	vector<vector<int> > ady; //Grafo de conflictos como matriz de adyacencias
	int bestObj; // Mejor valor de la funcion objetivo global
	vector<int> colores; // Mejor solucion, tiene el color asignado a cada vertice
	Problema(){}
};

// Alguna heuristica inicial para tener una solucion factible razonable
void secuencial(Problema * prob){
    vector<int> nuevos(prob->N,-1);
    nuevos[0] = 0;
    vector<int> usados;
    int maximo = 0;
    for(int i=1;i<prob->N;i++){
        usados.clear();
        for(int j=0;j<(int)prob->ejes[i].size();j++){
            if(prob->ejes[i][j]>i)break;
            usados.push_back(nuevos[prob->ejes[i][j]]);
        }
        sort(usados.begin(),usados.end());
        int minimo = 0;
        for(int j=0;j<(int)usados.size();j++){
            if(usados[j]==minimo)minimo++;
        }
        nuevos[i]=minimo;
        maximo = max(maximo,minimo);
    }
    
    if(maximo<prob->bestObj){
        prob->bestObj = maximo+1;
        prob->colores = nuevos;
    }
    return;
}

void heuristicaInicial(Problema * prob){
    secuencial(prob);
    return;
}


// La siguiente funcion resuelve el problema de coloreo usando CPLEX para
// resolver un modelo ILP.
// El modelo tiene variables X_ik que indican si el item i tiene el color k
// y variables Y_k que indican si el color k esta siendo usado.
void solveMIP(Problema * prob){
    
    ////////////////////////////
    //Variables
    
    // Variables necesarias para manejar el ambiente y el problema en CPLEX
    CPXENVptr env = NULL;
    CPXLPptr lp = NULL;
    
    // Variable para guardar los codigos de error cada vez que se utiliza una rutina de CPLEX
    int status;
    
    // Variable para indicar cuantas variables va a tener nuestro modelo
    // La cantidad de colores al definir el modelo esta dada por la mejor solucion que nos dio la heuristica inicial
    int cantvar = prob->N*prob->bestObj+prob->bestObj;
    
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
    
    //~ status = CPXsetintparam(env, CPX_PARAM_MIPDISPLAY, 5);
    //~ if(status)exit(-1);
    
    //~ status = CPXsetintparam(env, CPX_PARAM_MIPINTERVAL, 1 );
    //~ if(status)exit(-1);
    /////////
    
    
    /////////
    // Presolve
    status = CPXsetintparam (env, CPX_PARAM_MIPCBREDLP, CPX_OFF);
    if(status)exit(-1);
    
    //~ status = CPXsetintparam(env, CPX_PARAM_PRELINEAR, 0);
    //~ if(status)exit(-1);
    //~ status = CPXsetintparam(env, CPX_PARAM_REDUCE, CPX_PREREDUCE_PRIMALONLY);
    //~ if(status)exit(-1);
    //~ status = CPXsetintparam(env, CPX_PARAM_REPEATPRESOLVE, 0); 
    //~ if(status)exit(-1);
    //~ status = CPXsetintparam(env, CPX_PARAM_PROBE, -1);
    //~ if(status)exit(-1);
    /////////

    /////////
    // CPLEX heuristics
    //~ status = CPXsetintparam(env, CPX_PARAM_HEURFREQ, -1);
    //~ if(status)exit(-1); 
    
    //~ status = CPXsetintparam(env, CPX_PARAM_RINSHEUR, -1);
    //~ if(status)exit(-1);

    //~ status = CPXsetintparam(env, CPX_PARAM_FPHEUR, -1);
    //~ if(status)exit(-1);
    
    //~ status = CPXsetintparam(env, CPX_PARAM_LBHEUR, -1);
    //~ if(status)exit(-1);
    /////////
    
    
    /////////
    // CPLEX cuts
    //~ status = CPXsetintparam(env, CPX_PARAM_CUTPASS, 100);
    //~ if(status)exit(-1);
    
    //~ status = CPXsetintparam(env, CPX_PARAM_CLIQUES, -1 );
    //~ if(status)exit(-1);
    //~ status = CPXsetintparam(env, CPX_PARAM_COVERS, -1 );
    //~ if(status)exit(-1);
    //~ status = CPXsetintparam(env, CPX_PARAM_DISJCUTS, -1 );
    //~ if(status)exit(-1);
    //~ status = CPXsetintparam(env, CPX_PARAM_FLOWCOVERS, -1 );
    //~ if(status)exit(-1);
    //~ status = CPXsetintparam(env, CPX_PARAM_FLOWPATHS, -1 );
    //~ if(status)exit(-1);
    //~ status = CPXsetintparam(env, CPX_PARAM_FRACCUTS, -1 );
    //~ if(status)exit(-1);
    //~ status = CPXsetintparam(env, CPX_PARAM_GUBCOVERS, -1 );
    //~ if(status)exit(-1);
    //~ status = CPXsetintparam(env, CPX_PARAM_MCFCUTS, -1 );
    //~ if(status)exit(-1);
    //~ status = CPXsetintparam(env, CPX_PARAM_IMPLBD, -1 );
    //~ if(status)exit(-1);
    //~ status = CPXsetintparam(env, CPX_PARAM_MIRCUTS, -1 );
    //~ if(status)exit(-1);
    //~ status = CPXsetintparam(env, CPX_PARAM_ZEROHALFCUTS, -1 );
    //~ if(status)exit(-1);
    
    /////////
    
    
    
    /////////
    // Execution decisions
    
    status = CPXsetdblparam(env, CPX_PARAM_TILIM, 3600.0);
    if(status)exit(-1);
    
    //~ status = CPXsetintparam(env, CPX_PARAM_NODELIM, 1);
    //~ if(status)exit(-1);
    
    //~ status = CPXsetintparam(env, CPX_PARAM_PARALLELMODE, 1);
    //~ if(status)exit(-1);

    status = CPXsetintparam(env, CPX_PARAM_THREADS, 8);
    if(status)exit(-1);

    //~ status = CPXsetintparam(env, CPX_PARAM_NODESEL, CPX_NODESEL_DFS);
    status = CPXsetintparam(env, CPX_PARAM_NODESEL, CPX_NODESEL_BESTBOUND);
    if(status)exit(-1);
    
    //~ status = CPXsetintparam(env, CPX_PARAM_MIPEMPHASIS, CPX_MIPEMPHASIS_OPTIMALITY);
    //~ if(status)exit(-1);
    
    //~ status = CPXsetintparam(env, CPX_PARAM_STARTALG, CPX_ALG_PRIMAL); CHECKSTATUS;
    //~ if(status)exit(-1);
    //~ status = CPXsetintparam(env, CPX_PARAM_BARCROSSALG, -1); CHECKSTATUS;
    //~ if(status)exit(-1);
    //~ status = CPXsetintparam(env, CPX_PARAM_LPMETHOD, CPX_ALG_DUAL ); CHECKSTATUS; 
    //~ if(status)exit(-1);
    
    //~ status = CPXsetintparam(env, CPX_PARAM_VARSEL, CPX_VARSEL_MININFEAS); CHECKSTATUS; // 
    //~ if(status)exit(-1);
    
    //~ status = CPXsetintparam(env, CPX_PARAM_MIPORDIND, CPX_ON); CHECKSTATUS;
    //~ if(status)exit(-1);
    
    
    status = CPXsetintparam(env, CPX_PARAM_MIPSEARCH, CPX_MIPSEARCH_TRADITIONAL);
    if(status)exit(-1);
    /////////
    
    
    
    // Para setear un archivo de log
    //~ CPXFILEptr logfile = NULL;
   	//~ logfile = CPXfopen ("BPGC.log", "a");
    //~ CPXsetlogfile (env, logfile);
    
    
    ////////////////////////////////////////////////////////////////////
    
    
    
    
    
    
    
    
    
    
    // Creamos el problema en si en CPLEX
    lp = CPXcreateprob(env, &status, "coloreo");
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



void solve(Problema * prob){
    heuristicaInicial(prob);
    solveMIP(prob);
    return;
}

int main(int argc, char** argv){
    // Pipeamos el archivo pasado por parametro a cin
    freopen(argv[1],"r",stdin);
    int aux1, aux2;
    
    
    // Lectura del problema y armado de la estructura
    Problema * prob = new Problema();
    cin >> prob->N >> prob->M;
    prob->bestObj = prob->N;
    prob->colores = vector<int>(prob->N,0);
    prob->ejes = vector<vector<int> >(prob->N,vector<int>(0));
    for(int i=0;i<prob->N;i++)prob->colores[i] = i;
    prob->ady = vector<vector<int> >(prob->N,vector<int>(prob->N,0));
    for(int i=0;i<prob->M;i++){
        cin >> aux1 >> aux2;
        prob->ejes[aux1].push_back(aux2);
        prob->ejes[aux2].push_back(aux1);
        prob->ady[aux1][aux2] = 1;
        prob->ady[aux2][aux1] = 1;
    }
    for(int i=0;i<prob->N;i++){
        sort(prob->ejes[i].begin(),prob->ejes[i].end());
    }
    
    
    // Llamamos a nuestra funcion que resuelve el problema
    solve(prob);
    
    
    // Imprimimos la solucion
    cout << prob->bestObj << endl;
    for(int i=0;i<prob->N;i++){
        cout << prob->colores[i] << " ";
    }
    cout << endl;
    
    
    delete prob;
    return 0;
}
