#include <mpi.h>

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
int my_rank, comm_sz;

//Constants
char filename[512];
int cities;
int population;
int generations;
double crossover_p;
double mutation_p;
int immigration;

//Functions
void readFile();
void firstGeneration();
void findElite();
bool sameCost(int);
void calculatePosibility();
int whellSelection();
void crossover();
void mutation();
void immigrate();


//Main function
int main(int argc, char const *argv[]) {

    MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    //Read configure file
    FILE *conf;
    conf = fopen("config.conf", "r");
    if(conf != NULL){
        fscanf(conf, "%s", filename);       //File name
        fscanf(conf, "%d", &cities);        //Cites amount
        fscanf(conf, "%d", &population);    //Population amount
        fscanf(conf, "%d", &generations);   //Generation amount
        fscanf(conf, "%lf", &crossover_p);  //Crossover probability
        fscanf(conf, "%lf", &mutation_p);   //Mutation probability
        fscanf(conf, "%d", &immigration);   //Immigration amount
        fclose(conf);
    }else{
        printf("Configure file is not exist!\n");
    }

    //Initialize
    readFile();
    firstGeneration();
    findElite();
    calculatePosibility();

    srand(time(NULL) + my_rank);

    //Crossover, Mutation
    for(int j=0; j<generations; j++){
        for(int i=0; i<(population - immigration)/2-1; i++){
            child1.clear();
            child2.clear();
            crossover();
            mutation();
        }
        gene_child.push_back(gene[elite1]);
        gene_child.push_back(gene[elite2]);
        cost_child.push_back(cost[elite1]);
        cost_child.push_back(cost[elite2]);
        fitness_child.push_back(fitness[elite1]);
        fitness_child.push_back(fitness[elite2]);

        gene = gene_child;
        cost = cost_child;
        fitness = fitness_child;

        gene_child.clear();
        cost_child.clear();
        fitness_child.clear();
        calculatePosibility();
        findElite();

        for(int i=0; i<immigration; i++){
            immigrate();
        }
    }

    //Print elite
    printf("%d\n", cost[elite1]);

    MPI_Finalize();
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
        cost.push_back(cost_sum);
        fitness.push_back((double)1/(double)cost_sum);
        gene.push_back(temp);
        temp.clear();
    }
}

//Find elite
void findElite(){
    int min = 2147483647, sec = 2147483647;
    elite1 = 0, elite2 = 0;
    for(int i=0; i<cost.size(); i++){
        if(cost[i] < min){
            elite1 = i;
            min = cost[i];
        }
    }
    for(int i=0; i<cost.size(); i++){
        if(cost[i] < sec && cost[i] > min){
            elite2 = i;
            sec = cost[i];
        }
    }
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
    double ran = (double)rand() / RAND_MAX;
    int i = 0;
    while(ran > 0){
        for(i=0; i<posibility.size(); i++){
            ran -= posibility[i];
            if(ran < 0) return i;
        }
    }
    return i;
}

//Crossover
void crossover(){
    int dad = whellSelection();
    int mom = whellSelection();
    while(dad == mom)
        mom = whellSelection();

    double ran = (double)rand() / RAND_MAX;

    if(ran < crossover_p){
        int start = rand() % (cities - 1);
        int end = rand() % cities;
        while(start >= end)
            end = rand() % cities;

        vector<int> dad_buf, mom_buf;
        dad_buf = gene[dad];
        mom_buf = gene[mom];

        for(int i=start; i<end; i++){
            child1.push_back(gene[mom][i]);
        }
        for(int i=start; i<end; i++){
            for(int j=0; j<dad_buf.size(); j++){
                if(dad_buf[j] == gene[mom][i]){
                    dad_buf.erase(dad_buf.begin() + j);
                    break;
                }
            }
        }
        for(int i=0; i<dad_buf.size(); i++){
            child1.push_back(dad_buf[i]);
        }

        for(int i=start; i<end; i++){
            child2.push_back(gene[dad][i]);
        }
        for(int i=start; i<end; i++){
            for(int j=0; j<mom_buf.size(); j++){
                if(mom_buf[j] == gene[dad][i]){
                    mom_buf.erase(mom_buf.begin() + j);
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

    //Calculate cost
    int cost_sum = 0;

    for(int j=0; j<cities-1; j++)
        cost_sum += map[child1[j]][child1[j+1]];
    cost_sum += map[child1[0]][child1[cities-1]];
    cost_child.push_back(cost_sum);
    fitness_child.push_back((double)1/(double)cost_sum);

    cost_sum = 0;
    for(int j=0; j<cities-1; j++)
        cost_sum += map[child2[j]][child2[j+1]];
    cost_sum += map[child2[0]][child2[cities-1]];
    cost_child.push_back(cost_sum);
    fitness_child.push_back((double)1/(double)cost_sum);

    gene_child.push_back(child1);
    gene_child.push_back(child2);
}

//Mutation
void mutation(){
    double ran = (double)rand() / RAND_MAX;
    if(ran < mutation_p){
        int index1, index2, tmp;
        index1 = rand() % cities;
        index2 = rand() % cities;
        tmp = child1[index1];
        child1[index1] = child1[index2];
        child1[index2] = tmp;
        tmp = child2[index1];
        child2[index1] = child2[index2];
        child2[index2] = tmp;
    }
}

//Immigrate
void immigrate(){
    int index = whellSelection();
    vector<int> recv, send;
    send = gene[index];
    if(my_rank == 0){
        MPI_Send(&send[0], cities, MPI_INT, my_rank + 1, 0, MPI_COMM_WORLD);
        MPI_Recv(&recv[0], cities, MPI_INT, comm_sz - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }else if(my_rank == comm_sz - 1){
        MPI_Send(&send[0], cities, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Recv(&recv[0], cities, MPI_INT, my_rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }else{
        MPI_Send(&send[0], cities, MPI_INT, my_rank + 1, 0, MPI_COMM_WORLD);
        MPI_Recv(&recv[0], cities, MPI_INT, my_rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    gene_child.push_back(recv);
    cost_sum = 0;
    for(int j=0; j<cities-1; j++)
        cost_sum += map[recv[j]][recv[j+1]];
    cost_sum += map[recv[0]][recv[cities-1]];
    cost_child.push_back(cost_sum);
    fitness_child.push_back((double)1/(double)cost_sum);
}
