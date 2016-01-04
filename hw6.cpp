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
vector<int> temp, cost, cost_child, child1, child2;
vector<double> fitness, fitness_child;
vector<double> posibility;
int elite1, elite2;

//Constants
char filename[512];
int cities;
int population;
int generations;
double crossover_p;
double mutation_p;
int immigration;

//Functions
void readFile(char*, int);
void firstGeneration(int, int);
void findElite();
bool sameCost(int);
void calculatePosibility();
int whellSelection();


//Main function
int main(int argc, char const *argv[]) {

    //Read configure file
    FILE *conf;
    conf = fopen("config.conf", "r");
    if(conf != NULL){
        fscanf(conf, "%s", filename);       //File name
        fscanf(conf, "%d", &cities);        //Cites amount
        fscanf(conf, "%d", &population);    //Population amount
        fscanf(conf, "%d", &generations);   //Generation amount
        fscanf(conf, "%lf", &crossover_p);    //Crossover probability
        fscanf(conf, "%lf", &mutation_p);     //Mutation probability
        fscanf(conf, "%d", &immigration);   //Immigration amount
        fclose(conf);
    }else{
        printf("Configure file is not exist!\n");
    }

    //Initialize
    readFile(filename, cities);
    firstGeneration(cities, population);
    findElite();
    calculatePosibility();

    //Crossover
    srand(time(NULL));
    for(int i=0; i<population/2; i++){
        child1.clear();
        child2.clear();
        crossover();
        mutation();
    }

    //for(int i=0; i<cost.size(); i++) printf("Cost: %d, fitness: %lf\n", cost[i], fitness[i]);
    for(int i=0; i<generations; i++){

    }

    return 0;
}

//Read file function
void readFile() {
    FILE *fptr;
    fptr = fopen(filename, "r");
    for(int i=0; i<cities; i++){
        for(int j=0; j<cities; j++){
            fscanf(fptr, "%d", &map[i][j]);
        }
    }
}

//First generation
void firstGeneration() {
    srand(time(NULL));
    vector<int> randomnum;
    for(int i=0; i<cities; i++)
        randomnum.push_back(i);
    for(int i=0; i<population; i++){
        random_shuffle(randomnum.begin(), randomnum.end());
        for(int j=0; j<cities; j++)
            temp.push_back(randomnum[j]);
        int cost_sum = 0;
        for(int j=0; j<cities-1; j++)
            cost_sum += map[randomnum[j]][randomnum[j+1]];
        cost_sum += map[randomnum[0]][randomnum[cities-1]];
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
    elite1 = 0, elite2 = 0;
    for(int i=0; i<cost.size(); i++){
        if(cost[i] < min){
            elite2 = elite1;
            elite1 = i;
            sec = min;
            min = cost[i];
        }
    }
}

//Cost the same
bool sameCost(int _cost){
    for(int i=0; i<cost.size(); i++)
        if(_cost == cost[i])
            return true;
    return false;
}

//Calculate posibility
void calculatePosibility(){
    posibility.clear();
    double fitness_sum = 0.0;
    for(int i=0; i<fitness.size(); i++)
        fitness_sum += fitness[i];
    for(int i=0; i<fitness.size(); i++)
        posibility.push_back(fitness[i] / fitness_sum);
}

//Roulette Wheel Selection
int whellSelection(){
    srand(time(NULL));
    double ran = rand() / RAND_MAX;
    int i = 0;
    for(i=0; i<posibility.size(); i++){
        if(ran < 0) return i;
        else
            ran -= posibility[i];
    }
    return i;
}

//Crossover
void crossover(){
    int dad = whellSelection();
    int mom = whellSelection();
    while(dad == elite1 || dad == elite2)
        dad = whellSelection();
    while(mom == elite1 || mom == elite2)
        mom = whellSelection();

    double ran = rand() / RAND_MAX;

    if(ran > crossover_p){
        int start = rand() % (cities - 1);
        int end = rand() % cities;
        while(start >= end)
            end = rand() % cities;

        vector<int> dad_buf, mom_buf;
        dad_buf = gene[dad];
        mom_buf = gene[mom];

        //First child, dad_buf, mom
        for(int i=start; i<end; i++){
            child1.push_back(gene[mom][i]);
            for(int j=0; j<dad_buf.size(); j++){
                if(dad_buf[j] == gene[mom][j]){
                    dad_buf.erase(j);
                    break;
                }
            }
        }
        for(int i=0; i<dad_buf.size(); i++){
            child1.push_back(dad_buf[i]);
        }

        //Second child, mom_buf, dad
        for(int i=start; i<end; i++){
            child2.push_back(gene[dad][i]);
            for(int j=0; j<mom_buf.size(); j++){
                if(mom_buf[j] == gene[dad][j]){
                    mom_buf.erase(j);
                    break;
                }
            }
        }
        for(int i=0; i<mom_buf.size(); i++){
            child2.push_back(mom_buf[i]);
        }
    }else{
        child1 = gene[dad];
        child2 = gene[mom];
    }
    gene_child.push_back(child1);
    gene_child.push_back(child2);
}

//Mutation
void mutation(){
    double ran = rand() / RAND_MAX;
    if(ran >= mutation_p){

    }
}
