//#include <mpi.h>

#include <cstdio>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <vector>
using namespace std;

int map[100][100];
char* filename;
int city_count;
int population;
int generations;
double crossover;
double mutation;
int move;

int main(int argc, char const *argv[]) {

    // 讀取參數檔
    FILE *conf;
    conf = fopen("config.conf", "r");
    if(conf != NULL){
        fscanf(conf, "%s", filename);     // 城市相鄰矩陣檔案名稱
        printf("%s\n", filename);
        fscanf(conf, "%d", &city_count);    // 城市數
        printf("%d\n", city_count);
        fscanf(conf, "%d", &population);    // 人口數
        fscanf(conf, "%d", &generations);   // 世代數
        fscanf(conf, "%f", &crossover);     // 交配機率
        fscanf(conf, "%f", &mutation);      // 突變機率
        fscanf(conf, "%d", &move);          // 人口遷移數
        fclose(conf);
    }else{
        printf("參數檔不存在!\n");
    }


    return 0;
}
