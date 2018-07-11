#include <iostream>
#include <string>
#include <vector>
#include <ilcplex/cplex.h>

struct Aceite{
	float dureza;
	string nombre;
}

struct Problema{
	int cantidad_aceites_vegetales; // siempre los primeros en cada fila de precios o en aceites
	vector<Aceite> aceites;
	vector<vector<int>> costo_aceite_por_mes;

	Problema(int cantidad_aceites_vegetales, int cantidad_aceites_no_vegetales, cantidad_meses){
		int c_aceites = cantidad_aceites_vegetales+cantidad_aceites_no_vegetales;
		cantidad_aceites_vegetales = cantidad_aceites_vegetales;
		aceites = vector<Aceite>(c_aceites);
		costo_aceite_por_mes = vector< vector<int> >(c_meses, vector<int>(c_aceites));
	}
}

void Solve(Problema * problema){
	CPXENVptr env = NULL;
    CPXLPptr lp = NULL;
    
    int status;
    
    // Variable para indicar cuantas variables va a tener nuestro modelo
    // La cantidad de colores al definir el modelo esta dada por la mejor solucion que nos dio la heuristica inicial
    int cant_subindices = problema->aceites.size()*costo_aceite_por_mes.size();
    int cantvar = cant_subindices*3; //stock00,..,stock|Aceites||Meses|,compra00,...,uso|Aceites||Meses|
    

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

    env = CPXopenCPLEX(&status);
    if(env==NULL)exit(-1);
    
    status = CPXsetintparam(env, CPX_PARAM_DATACHECK, CPX_ON);
    if(status)exit(-1);
    
    status = CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON);
    if(status)exit(-1);
    
    status = CPXsetintparam (env, CPX_PARAM_MIPCBREDLP, CPX_OFF);
    if(status)exit(-1);
    
    status = CPXsetdblparam(env, CPX_PARAM_TILIM, 3600.0);
    if(status)exit(-1);
    
    status = CPXsetintparam(env, CPX_PARAM_THREADS, 8);
    if(status)exit(-1);

    status = CPXsetintparam(env, CPX_PARAM_NODESEL, CPX_NODESEL_BESTBOUND);
    if(status)exit(-1);
    
    status = CPXsetintparam(env, CPX_PARAM_MIPSEARCH, CPX_MIPSEARCH_TRADITIONAL);
    if(status)exit(-1);
    
    // Creamos el problema en si en CPLEX
    lp = CPXcreateprob(env, &status, "food_manufacture");
    if(lp==NULL)exit(-1);
    
    // Definimos la funcion objetivo como una minimizacion
    CPXchgobjsen(env, lp, CPX_MAX);

    // Definimos la variables del problema con su valor en la funcion objetivo, lower bound, upper bound y coltype que indica el tipo de variable
    // CPLEX maneja un arreglo de variables. Para esto consideramos las variables en el orden X_11, X_12, X_13, .... , X_21, X_22,..., XN1, .... , Y_1 , Y_2...

    int i = 0;
    
    int aceites = problema->aceites.size();
    int meses = problema->costo_aceite_por_mes.size();
    //Stock i j
	for(int i_mes = 0; i_mes < problema->costo_aceite_por_mes.size(); i_mes++){
    	for(int i_aceite = 0; i_aceite < problema->aceites.size(); i_aceite++){
    		obj[i_mes*meses+i_aceite] = -5.;
	        lb[i] = 0.;
	        ub[i] = 1000000.;
	        coltype[i] = 'R';
    	}
    }
        
    //Compra i j
    for(int i_mes = 0; i_mes < problema->costo_aceite_por_mes.size(); i_mes++){
    	for(int i_aceite = 0; i_aceite < problema->aceites.size(); i_aceite++){
    		obj[cant_subindices+i_mes*meses+i_aceite] = -problema->costo_aceite_por_mes[i_mes][i_aceite];
	        lb[i] = 0.;
	        ub[i] = 1000000.;
	        coltype[i] = 'R';
    	}
    }

    //Uso i j
    for(int i_mes = 0; i_mes < problema->costo_aceite_por_mes.size(); i_mes++){
    	for(int i_aceite = 0; i_aceite < problema->aceites.size(); i_aceite++){
    		obj[cant_subindices*2+i_mes*meses+i_aceite] = 150;
	        lb[i] = 0.;
	        ub[i] = 1000000.;
	        coltype[i] = 'R';
    	}
    }
    
    // Con esta llamada se crean todas las columnas de nuestro problema
    status = CPXnewcols(env, lp, cantvar, obj, lb, ub, coltype, NULL);
    if(status)exit(-1);
    
	
	for(int i_mes = 0; i_mes < problema->costo_aceite_por_mes.size(); i_mes++){
    	for(int i_aceite = 0; i_aceite < problema->aceites.size(); i_aceite++){
    		
    	}
    }

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
        for(int i=0; i<prob->N; ++i){
            for(int j=0; j<prob->bestObj; ++j){
                if(x[i*prob->bestObj+j]>0.001){
                    prob->colores[i]=j;
                }
            }
        }
    return;
}

Problema * read_input(istream &cin){
	int amount_vegetables, amount_non_vegetables, amount_months, precio;
	string name;
	float dureza;
	
	cin >> amount_vegetables >> amount_non_vegetables >> amount_months;
	Problema * problema = new Problema(amount_vegetables, amount_non_vegetables, amount_months);

	for(int i = 0; i < amount_vegetables+amount_non_vegetables; i++){
		cin >> name >> dureza;
		problema->aceites[i] = Aceite(name, dureza);
	}

	for(int i = 0; i < amount_months; i++){
		for(int j = 0; j < amount_non_vegetables+amount_vegetables; j++){
			cin >> precio;
			problema->aceites[i][j] = precio;
		}
	}

	return problema;
}

int main(){
	Problema * problema = read_input();

	return 0;
}