int NMinoristas = ...;
int NRegion1 = ...;
int NRegion2 = ...;
int NRegion3 = ...;
float PercentageSector1 =...;
int NPropiedades = ...;

range Propiedades = 1..NPropiedades;
range Minoristas = 1..NMinoristas;
range MinoristasR1 = 1..NRegion1;
range MinoristasR2 = NRegion1+1..NRegion1+NRegion2;
range MinoristasR3 = NRegion1+NRegion2+1..NRegion1+NRegion2+NRegion3;

int Combustible[Minoristas] = ...;
int Delivery[Minoristas] = ...;
int Licores[Minoristas] = ...;
int EsCategoriaA[Minoristas] = ...;

int EsRegion1[Minoristas] = ...;
int EsRegion2[Minoristas] = ...;
int EsRegion3[Minoristas] = ...;

dvar boolean D[Minoristas];
dvar float+ Delta[Propiedades];
dvar float+ Delta_prima[Propiedades];



maximize
  	sum(i in Minoristas) (1- D[i]);
subject to {
	ct1:
		sum(i in MinoristasR1)
		  (Combustible[i] * (PercentageSector1 + Delta[1] - Delta_prima[1])) 
		  == sum(i in MinoristasR1) (Combustible[i] * D[i]);
	ct2:
		sum(i in MinoristasR2)
		  (Combustible[i] * (PercentageSector1 + Delta[2] - Delta_prima[2]))
		  == sum(i in MinoristasR2) (Combustible[i] * D[i]);
	ct3:
		sum(i in MinoristasR3)
		  (Combustible[i] * (PercentageSector1 + Delta[3] - Delta_prima[3]))
		  == sum(i in MinoristasR3) (Combustible[i] * D[i]);
	ct4:
		sum(i in Minoristas)
		  (Delivery[i] * (PercentageSector1 + Delta[4] - Delta_prima[4]))
		  == sum(i in Minoristas) (Delivery[i] * D[i]);
	ct5:
		sum(i in Minoristas)
		  (Licores[i] * (PercentageSector1 + Delta[5] - Delta_prima[5]))
		  == sum(i in Minoristas) (Licores[i] * D[i]);
	ct6:
		sum(i in Minoristas)
		  (EsCategoriaA[i] * (PercentageSector1 + Delta[6] - Delta_prima[6]))
		  == sum(i in Minoristas) (EsCategoriaA[i] * D[i]);
	ct7:
		sum(i in Minoristas)
		  ((1-EsCategoriaA[i]) * (PercentageSector1 + Delta[7] - Delta_prima[7]))
		  == sum(i in Minoristas) ((1-EsCategoriaA[i]) * D[i]);
	forall(i in Propiedades)
  	{
		ct8:
			Delta[i] <= 0.05;
		ct9:
			Delta_prima[i] <= 0.05; 	  
  	}	
}