int NGeneratorTypes = ...;
int NHoras = ...;
int NGTWTypes = ...;

range AllGeneratorTypes = 1..NGeneratorTypes+NGTWTypes;
range GTW = 1..NGTWTypes;
range GeneratorTypes = 1..NGeneratorTypes;
range Hours = 1..NHoras;
range HoursPlusOne = 1..NHoras+1;
range HoursFrom2 = 2..NHoras;

int Demand [Hours] = ...;
int AmountGeneratorsPerType[AllGeneratorTypes] = ...;
int MinimumLevel[GeneratorTypes] = ...;
int MaximumLevel[GeneratorTypes] = ...;
float CostPerHourAtMin[GeneratorTypes] = ...;
float CostPerHourPerExtra[GeneratorTypes] = ...;
float StartUpCost[GeneratorTypes] = ...;

float ConstantCost [GTW] = ...;
float ConstantMW[GTW] = ...;
float DepthCost[GTW] = ...;
float RStartUpCost[GTW] = ...;
int MWPerMeterLoaded = ...;

dvar int+ UsedPerTypeXHour[AllGeneratorTypes][Hours];
dvar int+ StartedPerTypeXHour[AllGeneratorTypes][Hours];
dvar int+ ShutedDownPerTypeXHour[AllGeneratorTypes][HoursFrom2];
dvar float+ ExtraEnergyPerTypeXHour[GeneratorTypes][Hours];
dvar float+ ExtraRPerTypeXHour[GeneratorTypes][Hours];
dvar float+ MinimumRPerTypeXHour[GeneratorTypes][Hours];
dvar float+ Depth[HoursPlusOne];

minimize
  	sum(h in Hours)
  	  (sum(t in GeneratorTypes)
  	  	(StartedPerTypeXHour[t][h] * StartUpCost[t]
  	  	+ UsedPerTypeXHour[t][h] * CostPerHourAtMin[t]
  	  	+ (ExtraEnergyPerTypeXHour[t][h] + ExtraRPerTypeXHour[t][h]) * CostPerHourPerExtra[t])
  	  +
  	  (sum(w in GTW)
  	  	(StartedPerTypeXHour[NGeneratorTypes+w][h] * RStartUpCost[w]
  	  	+ UsedPerTypeXHour[NGeneratorTypes+w][h] * ConstantCost[w])
  	  ));
subject to {
	forall(t in AllGeneratorTypes)
  	{
		ct2:
			UsedPerTypeXHour[t][1] - StartedPerTypeXHour[t][1] == 0;
		ct8:
	  		StartedPerTypeXHour[t][1] <= AmountGeneratorsPerType[t];
	}
	forall(t in AllGeneratorTypes)
	forall(h in 1..NHoras-1)
	  {
	  	ct3:
	  		UsedPerTypeXHour[t][h+1] - UsedPerTypeXHour[t][h] - StartedPerTypeXHour[t][h+1] + ShutedDownPerTypeXHour[t][h+1] == 0;
	  	ct4:
	  		UsedPerTypeXHour[t][h] + StartedPerTypeXHour[t][h+1] <= AmountGeneratorsPerType[t];
	  }
	forall(h in Hours)
	  {
	  	ct5:
	  		(sum(t in GeneratorTypes) 
	  			(UsedPerTypeXHour[t][h] * MinimumLevel[t] 
	  			- MinimumRPerTypeXHour[t][h] 
	  			+ ExtraEnergyPerTypeXHour[t][h])
	  		+ 
	  		sum(w in GTW)
	  		  	(UsedPerTypeXHour[w+NGeneratorTypes][h] * ConstantMW[w])
	  		) >= Demand[h];	  
	  }
	forall(h in HoursFrom2)
	  {
	  	ct6:
	  		(sum(t in GeneratorTypes)
	  			((UsedPerTypeXHour[t][h]- StartedPerTypeXHour[t][h]) * MaximumLevel[t])
	  		+ sum(w in GTW) ConstantMW[w]
	  		)
	  		>= Demand[h] * 1.15;	  
	  }
	ct15:
  		(sum(t in GeneratorTypes) (UsedPerTypeXHour[t][1] * MaximumLevel[t]) + sum(w in GTW) ConstantMW[w])
  		>= Demand[1] * 1.15;
	forall(t in GeneratorTypes)
	forall(h in Hours)
	  {
	  	ct7:
	  		ExtraEnergyPerTypeXHour[t][h] + ExtraRPerTypeXHour[t][h] <= UsedPerTypeXHour[t][h] * (MaximumLevel[t] - MinimumLevel[t]);
	  	ct9:
	  		MinimumRPerTypeXHour[t][h] <= UsedPerTypeXHour[t][h] * MinimumLevel[t];	
	  }
	ct10:
  		Depth[1] == 16;
  	ct11:
  		Depth[25] - Depth[1] == 0;
	forall(h in HoursPlusOne)
	  {
	  	ct12:
	  	 	Depth[h] >=15;
  	 	ct13:
	  	 	Depth[h] <=20; 
	  }
	forall(h in Hours)
	  {
	  	ct14:
	  		(
	  			Depth[h+1] - Depth[h] - 
	  			(sum(g in GeneratorTypes) (ExtraRPerTypeXHour[g][h] + MinimumRPerTypeXHour[g][h])/MWPerMeterLoaded)
	  			+ sum(w in GTW) (UsedPerTypeXHour[w+NGeneratorTypes][h] * DepthCost[w])
	  		)
	  		== 0;
	  }
}