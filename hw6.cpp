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

//Const functions
int myrandom(int i) {
    return rand()%i;
}

//Functions
void readFile(char*, int);
void firstGeneration(int, int);

//Main function
int main(int argc, char const *argv[]) {
    char filename[512];
    int city_count;
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
        fscanf(conf, "%d", &city_count);    //City amount
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
    readFile(filename, city_count);
    firstGeneration(city_count, population);
    //for(int i=0; i<cost.size(); i++) printf("Cost: %d, fitness: %lf\n", cost[i], fitness[i]);
    for(int i=0; i<generations; i++){

    }

    return 0;
}

//Read file function
void readFile(char *_filename, int _city_count) {
    FILE *fptr;
    fptr = fopen(_filename, "r");
    for(int i=0; i<_city_count; i++){
        for(int j=0; j<_city_count; j++){
            fscanf(fptr, "%d", &map[i][j]);
        }
    }
}

//First generation
void firstGeneration(int _city_count, int _population) {
    srand(time(NULL));
    vector<int> randomnum;
    for(int i=0; i<_city_count; i++)
        randomnum.push_back(i);
    for(int i=0; i<_population; i++){
        random_shuffle(randomnum.begin(), randomnum.end());
        for(int j=0; j<_city_count; j++)
            temp.push_back(randomnum[j]);
        int cost_sum = 0;
        for(int j=0; j<_city_count-1; j++)
            cost_sum += map[randomnum[j]][randomnum[j+1]];
        cost.push_back(cost_sum);
        fitness.push_back((double)1/(double)cost_sum);
        gene.push_back(temp);
    }
}
