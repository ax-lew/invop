int NGeneratorTypes = ...;
int NHoras = ...;

range GeneratorTypes = 1..NGeneratorTypes;
range Hours = 1..NHoras;
range HoursFrom2 = 2..NHoras;

int Demand [Hours] = ...;

int AmountGeneratorsPerType[GeneratorTypes] = ...;
int MinimumLevel[GeneratorTypes] = ...;
int MaximumLevel[GeneratorTypes] = ...;
float CostPerHourAtMin[GeneratorTypes] = ...;
float CostPerHourPerExtra[GeneratorTypes] = ...;
float StartUpCost[GeneratorTypes] = ...;

dvar int+ UsedPerTypeXHour[GeneratorTypes][Hours];
dvar int+ StartedPerTypeXHour[GeneratorTypes][Hours];
dvar int+ ShutedDownPerTypeXHour[GeneratorTypes][HoursFrom2];
dvar float+ ExtraEnergyPerTypeXHour[GeneratorTypes][Hours];

minimize
  	sum(h in Hours)
  	  (sum(t in GeneratorTypes)
  	  	(StartedPerTypeXHour[t][h] * StartUpCost[t]
  	  	+ UsedPerTypeXHour[t][h] * CostPerHourAtMin[t]
  	  	+ ExtraEnergyPerTypeXHour[t][h] * CostPerHourPerExtra[t]));
subject to {
	forall(t in GeneratorTypes)
  	{
		ct2:
			UsedPerTypeXHour[t][1] - StartedPerTypeXHour[t][1] == 0;
		ct8:
	  		StartedPerTypeXHour[t][1] <= AmountGeneratorsPerType[t];
	}
	forall(t in GeneratorTypes)
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
	  		sum(t in GeneratorTypes) (UsedPerTypeXHour[t][h] * MinimumLevel[t] + ExtraEnergyPerTypeXHour[t][h]) >= Demand[h];
	  		
	  	ct6:
	  		sum(t in GeneratorTypes) (UsedPerTypeXHour[t][h] * MaximumLevel[t]) >= Demand[h] * 1.15;	  
	  }
	forall(t in GeneratorTypes)
	forall(h in Hours)
	  {
	  	ct7:
	  		ExtraEnergyPerTypeXHour[t][h] <= UsedPerTypeXHour[t][h] * (MaximumLevel[t] - MinimumLevel[t]);	
	  }
}