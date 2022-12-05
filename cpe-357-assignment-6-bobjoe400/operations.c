#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "utility.h"

//loop through our datalist, displaying the data in a readable format
DataList* display_f(int argc, char* argv[], DataList** data, int line_no){
    int entries = 0;
    for(int i = 0; i < (*data)->size; i++){
        StateData* currState = (*data)->data[i];
        for(int j = 0; j < currState->size; j++){
            entries++;
            CountyData* currCounty = currState->data[j];
            printf("%s, %s\n", currCounty->name, currState->name);
                printf("\tPopulation: %i\n",currCounty->population); (*data)->totalPop += currCounty->population;
                printf("\tEducation\n");
                    printf("\t\t>= High School: %.1f%%\n", currCounty->education[0]);
                    printf("\t\t>= Bachelor's: %.1f%%\n", currCounty->education[1]);
                printf("\tEthnicity Percentages\n");
                    printf("\t\tAmerican Indian and Alaska Native: %.1f%%\n",currCounty->ethnicities[0]);
                    printf("\t\tAsian Alone: %.1f%%\n", currCounty->ethnicities[1]);
                    printf("\t\tBlack Alone: %.1f%%\n", currCounty->ethnicities[2]);
                    printf("\t\tHispanic or Latino: %0.1f%%\n", currCounty->ethnicities[3]);
                    printf("\t\tNative Hawaiian and Other Pacific Islander Alone: %0.1f%%\n", currCounty->ethnicities[4]);
                    printf("\t\tTwo or More Races: %0.1f%%\n", currCounty->ethnicities[5]);
                    printf("\t\tWhite Alone: %0.1f%%\n", currCounty->ethnicities[6]);
                    printf("\t\tWhite Alone, not Hispanic or Latino: %0.1f%%\n", currCounty->ethnicities[7]);
                printf("\tIncome\n");
                    printf("\t\tMedian Household: %i\n", currCounty->income[0]);
                    printf("\t\tPer Capita: %i\n", currCounty->income[1]);
                    printf("\t\tBelow Poverty Level: %.1f%%\n", currCounty->incPBPL);
            printf("\n");
        }
    }
    return *data;
}

//Filters the DataList by state, returning a new malloc'd DataList and free()ing the provided DataList
DataList* filterState_f(int argc, char* argv[], DataList** data, int line_no){

    //initialize new DataList
    DataList* retData = checked_malloc(sizeof(DataList));
    retData->size = 0;
    retData->totalPop = 0;
    memset(retData->totalEducation, 0, sizeof(retData->totalEducation));
    memset(retData->totalEthnicities, 0, sizeof(retData->totalEthnicities));
    retData->totalIncPBPL = 0;
    retData->data = NULL;

    //get the state from the DataList, if it exists
    StateData* filter = get_state_data(*data, argv[1]);

    if(filter!=NULL){
        //loop through the counties of the state, adding the totals of those to the totals
        //of the return datalist
        for(int i = 0; i < filter->size; i++){
            retData = updateTotalTotals(&retData, filter->data[i]);
        }

        //set the only element of the return datalist to a duplicated state from our filter
        retData->data = add_state_data(retData->data, statedup(filter), &(retData->size));
    }

    printf("Filter: state == %s (%i entries)\n", argv[1], (filter == NULL) ? 0 : retData->data[0]->size);

    //free the original datalist
    free_data(*data);

    *data = retData;
    return *data;
}

/*
We start by checking that our filter is valid (field, lessthan or greater than, and if we provide a number to check against).

We then initialize the new datalist and loop through all the counties of the datalist adding the duped county to the new state if it's 
valid to add. If the new state isn't empty by the time we filter through it, add the new state to the new datalist. 

The "field" field of our Filter struct aligns with the correct COLUMN enum in our COLUMN_ITR array and we can use a switch-case 
with that to determine which field we are filtering. 
*/
//Filter the datalist based upon a field, returning a new malloc'd datalist and free()ing the provided DataList
DataList* filter_f(int argc, char* argv[], DataList** data, int line_no){

    Filter *filter = NULL;
    if((filter = createFilter(&filter, argv, line_no)) == NULL){
        return NULL;
    }

    DataList* retData = checked_malloc(sizeof(DataList));
    retData->size = 0;
    retData->totalPop = 0;
    memset(retData->totalEducation, 0, sizeof(retData->totalEducation));
    memset(retData->totalEthnicities, 0, sizeof(retData->totalEthnicities));
    retData->totalIncPBPL = 0;
    retData->data = NULL;

    int entries = 0;

    for(int i = 0; i < (*data)->size; i++){

        StateData* stateIn = (*data)->data[i];

        StateData* retState = checked_malloc(sizeof(StateData));
        retState->size = 0;
        retState->data = NULL;
        retState->name = strdup(stateIn->name);

        for(int j = 0; j < stateIn->size; j++){

            CountyData* countyIn = stateIn->data[j];

            char add = 0;

            switch(COLUMNS_ITR[filter->field]){
                case EduBach:
                    if(filter->comp){
                        add = (countyIn->education[0] >= filter->num) ? 1 : 0;
                    }else{
                        add = (countyIn->education[0] <= filter->num) ? 1 : 0;
                    }
                    break;
                case EduHS:
                    if(filter->comp){
                        add = (countyIn->education[1] >= filter->num) ? 1 : 0;
                    }else{
                        add = (countyIn->education[1] <= filter->num) ? 1 : 0;
                    }
                    break;
                case EthAIaANA:
                    if(filter->comp){
                        add = (countyIn->ethnicities[0] >= filter->num) ? 1 : 0;
                    }else{
                        add = (countyIn->ethnicities[0] <= filter->num) ? 1 : 0;
                    }
                    break;
                case EthAA:
                    if(filter->comp){
                        add = (countyIn->ethnicities[1] >= filter->num) ? 1 : 0;
                    }else{
                        add = (countyIn->ethnicities[1] <= filter->num) ? 1 : 0;
                    }
                    break;
                case EthBA:
                    if(filter->comp){
                        add = (countyIn->ethnicities[2] >= filter->num) ? 1 : 0;
                    }else{
                        add = (countyIn->ethnicities[2] <= filter->num) ? 1 : 0;
                    }
                    break;
                case EthHoL:
                    if(filter->comp){
                        add = (countyIn->ethnicities[3] >= filter->num) ? 1 : 0;
                    }else{
                        add = (countyIn->ethnicities[3] <= filter->num) ? 1 : 0;
                    }
                    break;
                case EthNHaOPIA:
                    if(filter->comp){
                        add = (countyIn->ethnicities[4] >= filter->num) ? 1 : 0;
                    }else{
                        add = (countyIn->ethnicities[4] <= filter->num) ? 1 : 0;
                    }
                    break;
                case EthToMR:
                    if(filter->comp){
                        add = (countyIn->ethnicities[5] >= filter->num) ? 1 : 0;
                    }else{
                        add = (countyIn->ethnicities[5] <= filter->num) ? 1 : 0;
                    }
                    break;
                case EthWA:
                    if(filter->comp){
                        add = (countyIn->ethnicities[6] >= filter->num) ? 1 : 0;
                    }else{
                        add = (countyIn->ethnicities[6] <= filter->num) ? 1 : 0;
                    }
                    break;
                case EthWAnHoL:
                    if(filter->comp){
                        add = (countyIn->ethnicities[7] >= filter->num) ? 1 : 0;
                    }else{
                        add = (countyIn->ethnicities[7] <= filter->num) ? 1 : 0;
                    }
                    break;
                case IMHI:
                    if(filter->comp){
                        add = (countyIn->income[0] >= filter->num) ? 1 : 0;
                    }else{
                        add = (countyIn->income[0] <= filter->num) ? 1 : 0;
                    }
                    break;
                case IPCI:
                    if(filter->comp){
                        add = (countyIn->income[1] >= filter->num) ? 1 : 0;
                    }else{
                        add = (countyIn->income[1] <= filter->num) ? 1 : 0;
                    }
                    break;
                case IPBPL:
                    if(filter->comp){
                        add = (countyIn->incPBPL >= filter->num) ? 1 : 0;
                    }else{
                        add = (countyIn->incPBPL <= filter->num) ? 1 : 0;
                    }
                    break;
                case P2014P:
                    if(filter->comp){
                        add = (countyIn->population >= filter->num) ? 1 : 0;
                    }else{
                        add = (countyIn->population <= filter->num) ? 1 : 0;
                    }
                    break;
            }

            if(!add) continue;
            CountyData* retCounty = countydup(countyIn);
            retState->data = add_county_data(retState->data, retCounty, &(retState->size));
            retData = updateTotalTotals(&retData, retCounty);
            entries++;
        }
        if(retState->size == 0){
            free_state(retState);
        }else{
            retData->data = add_state_data(retData->data, retState, &(retData->size));
        }
    }

    printf("Filter: %s %s %f (%i entries)\n", filterTypes[filter->field], (filter->comp)? "ge" : "le", filter->num, entries);

    free(filter);
    free_data(*data);
    
    *data = retData;
    return *data;
}

//print the total population of the datalist
DataList* populationTotal_f(int argc, char* argv[], DataList** data, int line_no){

    printf("2014 Population: %li\n", (*data)->totalPop);
    return *data;
}

/*
This follows the same algorithm as filter_f(). Instead of filtering the data, we just choose the 
data to print from our total fields in the DataList struct. 
*/
//print the number of people in the datalist that are attributed to the selected field
DataList* population_f(int argc, char* argv[], DataList** data, int line_no){
    
    float population = 0;

    argv[2] = "ge";
    Filter *filter = NULL;
    if((filter = createFilter(&filter, argv, line_no)) == NULL){
        return *data;
    }

    switch(COLUMNS_ITR[filter->field]){
        case EduBach:
            population = (*data)->totalEducation[0];
            break;
        case EduHS:
            population = (*data)->totalEducation[1];
            break;
        case EthAIaANA:
            population = (*data)->totalEthnicities[0];
            break;
        case EthAA:
            population = (*data)->totalEthnicities[1];
            break;
        case EthBA:
            population = (*data)->totalEthnicities[2];
            break;
        case EthHoL:
            population = (*data)->totalEthnicities[3];
            break;
        case EthNHaOPIA:
            population = (*data)->totalEthnicities[4];
            break;
        case EthToMR:
            population = (*data)->totalEthnicities[5];
            break;
        case EthWA:
            population = (*data)->totalEthnicities[6];
            break;
        case EthWAnHoL:
            population = (*data)->totalEthnicities[7];
            break;
        case IPBPL:
            population = (*data)->totalIncPBPL;
            break;
        default:
            fprintf(stderr, "cdp bad_filter_type: improperly formamatted data on line %i\n", line_no);
            free(filter);
            return *data;
    }

    printf("2014 %s population: %f\n", filterTypes[filter->field], population/100);
    free(filter);
    return *data;
}

/*
This follows the same alogrithm a population_f(). This time instead of printing the number of people 
attributed to a selected field, we divide that number of people by the total population to give us a percentage. 
*/
//prints the percentage of the total population in the datalist that are attributed to a selected field
DataList* percent_f(int argc, char* argv[], DataList** data, int line_no){
    float percent = 0;

    argv[2] = "ge";
    Filter *filter = NULL;
    if((filter = createFilter(&filter, argv, line_no)) == NULL){
        return *data;
    }

    switch(COLUMNS_ITR[filter->field]){
        case EduBach:
            percent = (*data)->totalEducation[0]/(*data)->totalPop;
            break;
        case EduHS:
            percent = (*data)->totalEducation[1]/(*data)->totalPop;
            break;
        case EthAIaANA:
            percent = (*data)->totalEthnicities[0]/(*data)->totalPop;
            break;
        case EthAA:
            percent = (*data)->totalEthnicities[1]/(*data)->totalPop;
            break;
        case EthBA:
            percent = (*data)->totalEthnicities[2]/(*data)->totalPop;
            break;
        case EthHoL:
            percent = (*data)->totalEthnicities[3]/(*data)->totalPop;
            break;
        case EthNHaOPIA:
            percent = (*data)->totalEthnicities[4]/(*data)->totalPop;
            break;
        case EthToMR:
            percent = (*data)->totalEthnicities[5]/(*data)->totalPop;
            break;
        case EthWA:
            percent = (*data)->totalEthnicities[6]/(*data)->totalPop;
            break;
        case EthWAnHoL:
            percent = (*data)->totalEthnicities[7]/(*data)->totalPop;
            break;
        case IPBPL:
            percent = (*data)->totalIncPBPL/(*data)->totalPop;
            break;
        default:
            fprintf(stderr, "cdp bad_filter_type: improperly formamatted data on line %i\n", line_no);
            free(filter);
            return *data;
    }

    printf("2014 %s percentage: %f%%\n", filterTypes[filter->field], percent);
    free(filter);
    return *data;
}

//our table of operations. 
optable_t optable[6] = {
                        {display_f, "display", 1, "usage: \"display takes no arguments\"\n"},
                        {filterState_f, "filter-state", 2, "usage: \"filter-state:<state abbreviation>\"\n"},
                        {filter_f, "filter", 4, "usage: \"filter:<field>:<ge/le>:<number>\"\n"},
                        {populationTotal_f, "population-total", 1, "usage: \"population-total takes no arguments\"\n"},
                        {population_f, "population", 2, "usage: \"population:<field>\"\n"},
                        {percent_f, "percent", 2, "usage: \"percent:<field>\"\n"}
                    };