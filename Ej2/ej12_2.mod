int NAceitesVegetales = ...;
int NAceitesNoVegetales = ...;
int NAceites = ...;
int NMeses = ...;
float PrecioProducto = ...;
int MinimoUsoDeUnAceite = ...;
int CotaSuperiorDeUsoDeUnAceite = ...;
int MaximoAceitesAUsarEnUnMes = ...;

range Aceites = 1..NAceites;
range Meses = 1..NMeses;
range MesesMasUno = 1..NMeses+1;
range AceitesNoVegetales = 1..NAceitesNoVegetales;
range AceitesVegetales = 1..NAceitesVegetales;

float Precio [Meses][Aceites] = ...;
float Dureza [Aceites] = ...;

dvar float+    Stock[MesesMasUno][Aceites];
dvar float+    Uso[Meses][Aceites];
dvar float+    Compra[Meses][Aceites];
dvar boolean    SeUsa[Meses][Aceites];

maximize
  sum(i in Meses)
  sum(j in Aceites)
  (Uso[i][j] * PrecioProducto 
  - Compra[i][j] * Precio[i][j]
  - 5 * Stock[i+1][j]);
subject to {
	forall( i in Meses)
	  {
	  	ct1:
	      sum( v in AceitesVegetales) Uso[i][v] <= 200;
     	ct2:
	      sum( v in AceitesNoVegetales) Uso[i][NAceitesVegetales + v] <= 250;
		ct3:
			sum( j in Aceites) Uso[i][j] * Dureza[j] <= sum( j in Aceites) Uso[i][j] * 6;
		ct4:
			sum( j in Aceites) Uso[i][j] * Dureza[j] >= sum( j in Aceites) Uso[i][j] * 3;
	      
	  }

  	forall( j in Aceites )
  	  {
  	    ct5:
			Stock[1][j] - Stock[NMeses+1][j] == 0;
		ct6:
			Stock[1][j] == 500;	  
  	  }  	
	
	forall( i in Meses )
	forall( j in Aceites)
	  	ct7:
			Stock[i+1][j] <= 1000;
	
	forall(i in Meses)
  	forall(j in Aceites)
		ct8:
			Stock[i+1][j] - Stock[i][j] - Compra[i][j] + Uso[i][j] == 0;
	
	forall(i in Meses)
  	forall(j in Aceites)
		ct9:
			Uso[i][j] <= CotaSuperiorDeUsoDeUnAceite * SeUsa[i][j];
			
	forall(i in Meses)
  	forall(j in Aceites)
		ct10:
			Uso[i][j] >= MinimoUsoDeUnAceite * SeUsa[i][j];
			
	forall(i in Meses)
		ct11:
			sum(j in Aceites) SeUsa[i][j] <= MaximoAceitesAUsarEnUnMes;
	
	forall(i in Meses)
	forall(j in AceitesNoVegetales)
		ct12:
			SeUsa[i][j] <= SeUsa[i][5];
			
	forall(i in Meses)
	forall(j in Aceites)
	{
		ct13:
			Uso[i][j] >= 0;
		ct14:
			Compra[i][j] >= 0;
		ct15:
			Stock[i+1][j] >= 0;
	}
}