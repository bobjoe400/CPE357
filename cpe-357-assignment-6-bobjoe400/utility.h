#ifndef UTILITY_H
#define UTILITY_H
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>

#define NUM_FILTERS 16
#define NUMELEMS(x) sizeof(x)/sizeof(x[0])

//Enumerator for our Colums in the input .csv. The values of which relate to the proper columns in the .csv
enum COLUMNS{  County, 
        State, 
        EduBach=5, EduHS, 
        EthAIaANA = 11,  EthAA, EthBA, EthHoL, EthNHaOPIA, EthToMR, EthWA, EthWAnHoL,
        IMHI = 25, IPCI, IPBPL,
        P2014P = 38};

extern int COLUMNS_ITR[16];

extern char* filterTypes[16];

/*
Description of elements, repspectively, are:
    Name of the county
    Percentage of people in the education fields
    Percentage of people in the ethnicity fields
    Income amounts in the income fields
    Percentage of people below the poverty line
    Population for the county             
*/
typedef struct{
    char* name;
    float education[2];
    float ethnicities[8];
    int income[2];
    float incPBPL;
    int population;
}CountyData;

/*
Description of elements, repspectively, are:
    Size of the countylist for the state
    State abbreviation
    List of counties in the state
*/
typedef struct{
    int size;
    char* name;
    CountyData** data;
}StateData;

/*
Description of elements, repspectively, are:
    Size of the DataLlist
    Total people in each of the education fields
    Total people in each of the ethnicity fields
    Total income for each of the income fields
    Total people below the income poverty lines
    Total people in the datalist
    List of states in the DataList
*/
typedef struct{
    int size;
    float totalEducation[2];
    float totalEthnicities[8];
    float totalIncome[2];
    float totalIncPBPL;
    long totalPop;
    StateData** data;
}DataList;

//function pointer type definition for our operations
typedef DataList* (*operation)(int argc, char* argv[], DataList** data, int line_no);

/*
Description of elements, repspectively, are:
    Function pointer of the operation
    Name of the operation as referenced in the .ops file
    Number of argumnts (including the operation) attributed to the operation
    Usage string for errors related to the operation 
*/
typedef struct{
    operation op;
    char* name;
    int arg_num;
    char* usage_string;
}optable_t;

extern optable_t optable[6];

/*
Description of elements, repspectively, are:
    Field number as it relates to our fieldTypes array
    Type of comparison being done, 0 = '>', 1 = '<'
    The nmber we are comparing against
*/
typedef struct{
    short field;
    char comp;
    float num;
}Filter;

void* checked_malloc(size_t size);
int str_to_array(char** array, char* in_string, char* delim, int size);
int validate_arguments(int argc, char *argv[], int num_args, char* usage_string);

Filter* validate_filter(char* argv[]);
Filter* createFilter(Filter** filter, char* argv[], int line_no);

CountyData** add_county_data(CountyData** countyList, CountyData* county, int* size);
StateData** add_state_data(StateData** stateList, StateData* state, int* size);
StateData* get_state_data(DataList* datalist, char* statename);
CountyData* get_county_data(StateData* state, char* countyname);
StateData* get_state_data(DataList* datalist, char* statename);
DataList* updateTotalTotals(DataList** dataList, CountyData* county);

CountyData* countydup(CountyData* county);
StateData* statedup(StateData* state);

void free_data(DataList* data);
void free_state(StateData* data);
void free_county(CountyData* data);
#endif