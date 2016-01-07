#include <mpi.h>

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <climits>
#include <vector>
#include <algorithm>

#include <omp.h>

using namespace std;

/************
* Constants *
************/
#define MAX_NODE            100
#define MIGRATION_INTERVAL  100

/*********************
* Function Prototype *
*********************/
void InitializeVariables(void);
// File operation
void ReadParameterFile(const char *filename);
void ReadGraphFile(const char *filename);
// Population operation
void InitializePopulation(void);
void CalculateAllFitness(void);
vector<int> GetFittest(void);
vector<int> GetSecondFittest(void);
// Tour operation
int GetDistance(vector<int> tour);
void DisplayTour(vector<int> tour);
// Genetic Algorithm
void EvolveGeneration(void);
vector<int> CrossoverTour(vector<int> a, vector<int> b);
vector<int> MutateTour(vector<int> tour);
void MigratePopulation(void);
int GeneticAlgorithm(void);

/*******************
* Global Variables *
*******************/
// Graph
int graph[MAX_NODE][MAX_NODE];
// MPI related variables
int comm_size, my_rank;
// Genetic Algorithm parameters
int city_count, population_size, max_generations, migration_size;
double crossover_prob, mutation_prob;
// Population
vector< vector<int> > population;
vector<int> cost;
vector<double> fitness;
double fitness_sum;
int first_index, first_cost;
int second_index, second_cost;

int main(int argc, char *argv[]) {
    // Init MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    srand(time(NULL) + my_rank);

    // Read Parameter
    ReadParameterFile("parameter.txt");
    InitializeVariables();
    MPI_Barrier(MPI_COMM_WORLD);
    // Genetic Algorithm
    double start = omp_get_wtime();
    GeneticAlgorithm();
    MPI_Barrier(MPI_COMM_WORLD);
    if (my_rank == 0)
        printf("Used time: %.4f\n", omp_get_wtime() - start);

    // Finalize
    MPI_Finalize();

    return 0;
}

void InitializeVariables(void) {
    population.resize(population_size);

    cost.resize(population_size);
    fitness.resize(population_size);
}

void ReadParameterFile(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp) {
        char graph_filename[512];

        fscanf(fp, "%s", graph_filename);
        fscanf(fp, "%d%d%d%lf%lf%d", &city_count, &population_size, &max_generations,
            &crossover_prob, &mutation_prob, &migration_size);
        fclose(fp);

        ReadGraphFile(graph_filename);
    }
}

void ReadGraphFile(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp) {
        for (int i = 1; i <= city_count; ++i) {
            for (int k = 1; k <= city_count; ++k)
                fscanf(fp, "%d", &graph[i][k]);
        }

        fclose(fp);
    }
}

void InitializePopulation(void) {
    vector<int> tour(city_count);

    for (int i = 1; i <= city_count; ++i)
        tour[i - 1] = i;

    for (int i = 0; i < population_size; ++i) {
        random_shuffle(tour.begin(), tour.end());
        population[i] = tour;
    }

    CalculateAllFitness();
}

void CalculateAllFitness(void) {
    fitness_sum = 0;

    first_index = second_index = 0;
    first_cost = second_cost = INT_MAX;

    for (int i = 0; i < population_size; ++i) {
        cost[i] = GetDistance(population[i]);
        fitness[i] = 1.0 / cost[i];
        fitness_sum += fitness[i];

        if (cost[i] < first_cost) {
            second_index = first_index;
            first_index = i;
            second_cost = first_cost;
            first_cost = cost[i];
        } else if (cost[i] < second_cost) {
            second_index = i;
            second_cost = cost[i];
        }
    }
}

vector<int> GetFittest(void) {
    return population[first_index];
}

vector<int> GetSecondFittest(void) {
    return population[second_index];
}

int GetDistance(vector<int> tour) {
    int result = 0;

    for (int i = 1; i < city_count; ++i)
        result += graph[tour[i - 1]][tour[i]];
    result += graph[tour[city_count - 1]][tour[0]];

    return result;
}

void DisplayTour(vector<int> tour) {
    for (int i = 0; i < city_count; ++i)
        printf("%d ", tour[i]);
    printf("%d\n", tour[0]);
}

vector<int> CrossoverTour(vector<int> a, vector<int> b) {
    vector<int> result(city_count);

    bool contained[MAX_NODE] = { 0 };
    int start_index = rand() % city_count;
    int end_index = rand() % city_count;

    while(start_index == end_index)
        end_index = rand() % city_count;

    if (start_index > end_index)
        swap(start_index, end_index);

    for (int i = start_index; i <= end_index; ++i)
        contained[result[i] = a[i]] = true;

    for (int i = 0; i < city_count; ++i) {
        if (!contained[b[i]]) {
            for (int k = 0; k < city_count; ++k) {
                if (!result[k]) {
                    contained[result[k] = b[i]] = true;
                    break;
                }
            }
        }
    }

    return result;
}

vector<int> MutateTour(vector<int> tour) {
    vector<int> result = tour;

    int first_index = rand() % city_count;
    int second_index = rand() % city_count;

    while(first_index == second_index)
        second_index = rand() % city_count;

    swap(result[first_index], result[second_index]);

    return result;
}

vector<int> RandomSelectTour(void) {
    int result_index = rand() % population.size();
    double prob = (double) rand() / RAND_MAX;

    while(prob > fitness[result_index] / fitness_sum) {
        prob -= fitness[result_index] / fitness_sum;
        result_index = rand() % population.size();
    }

    return population[result_index];
}

void EvolveGeneration(void) {
    vector< vector<int> > new_population(population_size);

    new_population[0] = GetFittest();
    new_population[1] = GetSecondFittest();

    for (int i = 2; i < population_size; i += 2) {
        vector<int> parent1 = RandomSelectTour();
        vector<int> parent2 = RandomSelectTour();

        vector<int> child1 = parent1;
        vector<int> child2 = parent2;

        if ((double)rand() / RAND_MAX < crossover_prob) {
            child1 = CrossoverTour(parent1, parent2);
            child2 = CrossoverTour(parent2, parent1);
        }

        if ((double)rand() / RAND_MAX < mutation_prob)
            child1 = MutateTour(child1);
        if ((double)rand() / RAND_MAX < mutation_prob)
            child2 = MutateTour(child2);

        new_population[i] = child1;
        new_population[i + 1] = child2;
    }

    population = new_population;

    CalculateAllFitness();
}

void MigratePopulation(void) {
    vector< vector<int> > new_population(population_size);

    new_population[0] = GetFittest();
    new_population[1] = GetSecondFittest();

    for (int i = 2; i < population_size - migration_size; ++i)
        new_population[i] = RandomSelectTour();

    for (int i = population_size - migration_size; i < population_size; ++i) {
        vector<int> selected = RandomSelectTour();
        vector<int> received(city_count);

        int prev_rank = (my_rank - 1 + comm_size) % comm_size;
        int next_rank = (my_rank + 1) % comm_size;

        if (my_rank % 2) {
            MPI_Recv(&received[0], city_count, MPI_INT, prev_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(&selected[0], city_count, MPI_INT, next_rank, 0, MPI_COMM_WORLD);
        } else {
            MPI_Send(&selected[0], city_count, MPI_INT, next_rank, 0, MPI_COMM_WORLD);
            MPI_Recv(&received[0], city_count, MPI_INT, prev_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        new_population[i] = received;
    }

    population = new_population;

    CalculateAllFitness();
}

int GeneticAlgorithm(void) {
    InitializePopulation();

    printf("[Rank %d] Initial cost: %d\n", my_rank, GetDistance(GetFittest()));

    for (int i = 0; i < max_generations; ++i) {
        if (comm_size != 1 && i != 0 && i % MIGRATION_INTERVAL == 0) {
            //printf("[Rank %d] Round %d cost: %d\n", my_rank, i, GetDistance(GetFittest()));

            MPI_Barrier(MPI_COMM_WORLD);
            MigratePopulation();
        }

        EvolveGeneration();
        //printf("[Rank %d] Round %d cost: %d\n", my_rank, i, GetDistance(GetFittest()));
    }

    printf("[Rank %d] Optimal cost: %d\n", my_rank, GetDistance(GetFittest()));

    //DisplayTour(GetFittest());
    fflush(stdout);

    return 0;
}