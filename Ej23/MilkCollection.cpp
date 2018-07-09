#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include<fstream>
#include <sstream>
#include <math.h>

using namespace std;



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



int main(int argc, char** argv){
    map<int,Farm> farms = getFarmInfo();
    vector< vector<float> > distances = createDistancesMatrix(farms);
    
    return 0;
}
