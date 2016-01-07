#include <mpi.h>
#include <omp.h>

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
vector<int> cost, child1, child2;
vector<double> fitness;
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
vector<int> CrossoverTour(vector<int> a, vector<int> b);
void mutation();
void immigrate();
void calculateCost();


//Main function
int main(int argc, char *argv[]) {

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
    cost.resize(population);
    fitness.resize(population);
    posibility.resize(population);
    printf("Start.\n");
    readFile();
    printf("readFile.\n");
    firstGeneration();
    printf("firstGeneration.\n");
    calculateCost();
    printf("calculateCost.\n");
    findElite();
    printf("findElite.\n");
    calculatePosibility();
    printf("calculatePosibility.\n");

    srand(time(NULL) + my_rank);

    //Crossover, Mutation
    int i, j;
    for(j=0; j<generations; j++){
        printf("Rank %d gen %d\n", my_rank, j);
        for(i=0; i<(population - immigration)/2-1; i++){
            child1.clear();
            child2.clear();
            crossover();
            mutation();
        }
        gene_child.push_back(gene[elite1]);
        gene_child.push_back(gene[elite2]);

        gene = gene_child;

        gene_child.clear();

        for(i=0; i<immigration; i++){
            immigrate();
        }
            printf("Finish immigrate.\n");

        calculateCost();
        printf("calculateCost Finish.\n");
        calculatePosibility();
        findElite();
    }

    printf("Finish.\n");
    //Print elite
    if(my_rank == 0){
        for(int i=1; i<comm_sz; i++){
            int temp_cost;
            MPI_Recv(&temp_cost, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("Rank %d cost %d\n", i, temp_cost);
            if(temp_cost < cost[elite1])
                cost[elite1] = temp_cost;
        }
        printf("%d\n", cost[elite1]);
    }else{
        MPI_Send(&cost[elite1], 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

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
    vector<int> randomnum;
    for(int i=0; i<cities; i++)
        randomnum.push_back(i);
    for(int i=0; i<population; i++){
        random_shuffle(randomnum.begin(), randomnum.end());
        gene.push_back(randomnum);
    }
}

//Find elite
void findElite(){
    int min = 2147483647, sec = 2147483647;
    elite1 = 0, elite2 = 0;

    #pragma omp parallel for num_threads(2)
    for(int i=0; i<cost.size(); i++){
        if(cost[i] < min){
            elite1 = i;
            min = cost[i];
        }
    }

    #pragma omp parallel for num_threads(2)
    for(int i=0; i<cost.size(); i++){
        if(cost[i] < sec && cost[i] > min){
            elite2 = i;
            sec = cost[i];
        }
    }
}

//Calculate posibility
void calculatePosibility(){
    double fitness_sum = 0.0;
    for(int i=0; i<fitness.size(); i++)
        fitness_sum += fitness[i];
    for(int i=0; i<fitness.size(); i++)
        posibility[i] = fitness[i] / fitness_sum;
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

        child1 = CrossoverTour(dad_buf, mom_buf);
        child2 = CrossoverTour(mom_buf, dad_buf);

    }else{
        child1 = gene[dad];
        child2 = gene[mom];
    }

    gene_child.push_back(child1);
    gene_child.push_back(child2);
}

//Crossover Tour
vector<int> CrossoverTour(vector<int> a, vector<int> b) {
    vector<int> result(cities);

    bool contained[100] = { 0 };
    int start_index = rand() % cities;
    int end_index = rand() % cities;

    while(start_index == end_index)
        end_index = rand() % cities;

    if (start_index > end_index)
        swap(start_index, end_index);

    for (int i = start_index; i <= end_index; ++i)
        contained[result[i] = a[i]] = true;

    for (int i = 0; i < cities; ++i) {
        if (!contained[b[i]]) {
            for (int k = 0; k < cities; ++k) {
                if (!result[k]) {
                    contained[result[k] = b[i]] = true;
                    break;
                }
            }
        }
    }

    return result;
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
    recv.resize(send.size());

    MPI_Barrier(MPI_COMM_WORLD);
    if(my_rank % 2){
        MPI_Send(&send[0], cities, MPI_INT, (my_rank + 1) % comm_sz, 0, MPI_COMM_WORLD);
        MPI_Recv(&recv[0], cities, MPI_INT, (my_rank - 1 + comm_sz) % comm_sz, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }else{
        MPI_Recv(&recv[0], cities, MPI_INT, (my_rank - 1 + comm_sz) % comm_sz, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Send(&send[0], cities, MPI_INT, (my_rank + 1) % comm_sz, 0, MPI_COMM_WORLD);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    gene_child.push_back(recv);
}

//Calculate Cost
void calculateCost(){
    for(int i=0; i<population; i++){
        int cost_sum = 0;
        double fitness_sum = 0.0;
        for(int j=0; j<cities; j++){
            cost_sum += map[gene[i][j]][gene[i][(j+1)%cities]];
        }
        cost[i] = cost_sum;
        fitness[i] = (double)1.0 / cost_sum;
    }
}
