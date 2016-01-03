//#include <mpi.h>

#include <cstdio>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <vector>
#include <ctime>
using namespace std;

//Parameters
int map[100][100];
vector< vector<int> > gene, gene_child;
vector<int> temp, cost, cost_child;
vector<double> fitness, fitness_child;
int elite1, elite2;

//Functions
void readFile(char*, int);
void firstGeneration(int, int);
void findElite();
bool sameCost(int);


//Main function
int main(int argc, char const *argv[]) {
    char filename[512];
    int cities;
    int population;
    int generations;
    double crossover;
    double mutation;
    int immigration;

    //Read configure file
    FILE *conf;
    conf = fopen("config.conf", "r");
    if(conf != NULL){
        fscanf(conf, "%s", filename);       //File name
        fscanf(conf, "%d", &cities);        //Cites amount
        fscanf(conf, "%d", &population);    //Population amount
        fscanf(conf, "%d", &generations);   //Generation amount
        fscanf(conf, "%lf", &crossover);    //Crossover probability
        fscanf(conf, "%lf", &mutation);     //Mutation probability
        fscanf(conf, "%d", &immigration);   //Immigration amount
        fclose(conf);
    }else{
        printf("Configure file is not exist!\n");
    }

    //Initialize
    readFile(filename, cities);
    firstGeneration(cities, population);
    findElite();
    //for(int i=0; i<cost.size(); i++) printf("Cost: %d, fitness: %lf\n", cost[i], fitness[i]);
    for(int i=0; i<generations; i++){

    }

    return 0;
}

//Read file function
void readFile(char *_filename, int _cities) {
    FILE *fptr;
    fptr = fopen(_filename, "r");
    for(int i=0; i<_cities; i++){
        for(int j=0; j<_cities; j++){
            fscanf(fptr, "%d", &map[i][j]);
        }
    }
}

//First generation
void firstGeneration(int _cities, int _population) {
    srand(time(NULL));
    vector<int> randomnum;
    for(int i=0; i<_cities; i++)
        randomnum.push_back(i);
    for(int i=0; i<_population; i++){
        random_shuffle(randomnum.begin(), randomnum.end());
        for(int j=0; j<_cities; j++)
            temp.push_back(randomnum[j]);
        int cost_sum = 0;
        for(int j=0; j<_cities-1; j++)
            cost_sum += map[randomnum[j]][randomnum[j+1]];
        if(!sameCost(cost_sum)){    //If has same cost, we don't want this data.
            i--;
            continue;
        }
        cost.push_back(cost_sum);
        fitness.push_back((double)1/(double)cost_sum);
        gene.push_back(temp);
    }
}

//Find elite
void findElite(){
    int min = 2147483647, sec = 2147483647;
    for(int i=0; i<cost.size(); i++){
        if(cost[i] < min){
            sec = min;
            min = cost[i];
        }
    }
}

//Cost the same
bool sameCost(int _cost){
    for(int i=0; i<cost.size(); i++){
        if(_cost == cost[i])
            return true;
    }
    return false;
}
