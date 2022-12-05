#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "utility.h"

/*
Possible types of filters. Even though we only care about flitering a select number
of these fields, we make it the same size as our total number of fields for easier 
processing. 
*/
char* filterTypes[16] = {	"County",
							"State",
							"Education.Bachelor's Degree or Higher",
							"Education.High School or Higher",
							"Ethnicities.American Indian and Alaska Native Alone",
							"Ethnicities.Asian Alone",
							"Ethnicities.Black Alone",
							"Ethnicities.Hispanic or Latino",
							"Ethnicities.Native Hawaiian and Other Pacific Islander Alone",
							"Ethnicities.Two or More Races",
							"Ethnicities.White Alone",
							"Ethnicities.White Alone not Hispanic or Latino",
							"Income.Median Household Income",
							"Income.Per Capita Income",
							"Income.Persons Below Poverty Level",
							"Population.2014 Population"
						};

//Iterator for our COLUMNS enumarator
int COLUMNS_ITR[16] =	{ 	County,
							State,
							EduBach, EduHS,
							EthAIaANA, EthAA, EthBA, EthHoL, EthNHaOPIA, EthToMR, EthWA, EthWAnHoL,
							IMHI, IPCI, IPBPL,
							P2014P
                    	};

void* checked_malloc(size_t size){
	void *p;
	p = malloc(size);
	if(p == NULL){
		perror("malloc");
		exit(1);
	}
	return p;
}

void replace_string(char* in_str, char delim){
	char *r, *w;
	for (w = r = in_str; *r; r++) {
		if (*r != delim) {
			*w++ = *r;
		}
	}
	*w = '\0';
}

/*
uses strtok to create an array of strings from in_string, a selected delim delimiter, and a specific array size.
Returns the number of elements put into the array. arr must be at least arr_size of char*
*/
int str_to_array(char** arr, char* in_string, char* delim, int arr_size){
	char* temp;

	char** next = arr;
	int counter = 0;
	temp = strtok(in_string, delim);
	while(temp!=NULL && counter < arr_size){
		counter++;
		*next++ = temp;
		temp = strtok(NULL, delim);
	}
	*next = NULL;
	return counter;
}

//validate that argv is num_args big. if it's not, print ussage_string. 
int validate_arguments(int argc, char *argv[], int num_args, char* usage_string){
    if (argc == 0){
        fprintf(stderr, usage_string, "cdp");
        return -1;
    }
    else if (argc < num_args || argc > num_args){
        fprintf(stderr, usage_string, argv[0]);
        return -1;
    }
	return 0;
}

//returns a malloc'd Filter if argv producdes a valid filter
Filter* validate_filter(char* argv[]){
	Filter *filter = checked_malloc(sizeof(Filter));
	filter->num = 0;
	for(int i = 0; i<NUM_FILTERS; i++){
		replace_string(argv[1], ',');
		if(strcmp(argv[1], filterTypes[i]) == 0){
			filter->field = (short) i;
			if(strcmp(argv[2], "ge") == 0 || strcmp(argv[2], "le") == 0){
				filter->comp = (argv[2][0] == 'g') ? 1 : 0;
				if(argv[3] != NULL){
					char* str_ptr = argv[3];
					char* end_ptr;
					filter->num = strtof(str_ptr, &end_ptr);
					if((errno == 0) && (*end_ptr == '\0') && (str_ptr != end_ptr)){
						return filter;
					}
				}else{
					return filter;
				}
			}
		}
	}
	free(filter);
	return NULL;	
}

//returns the (if valid) malloc' filter created by valid_filter()
Filter* createFilter(Filter** filter, char* argv[], int line_no){
	if(((*filter) = validate_filter(argv)) == NULL){
        free(*filter);
        fprintf(stderr, "cdp bad_arguments: improperly formamatted data on line %i\n", line_no);
        return NULL;
    }
	return (*filter);
}

//adds a county to a CountyData arraylist
CountyData** add_county_data(CountyData** countyList, CountyData* county, int* size){
    CountyData** temp = (CountyData**) realloc(countyList, (sizeof(CountyData*) * ((*size)+1)));
	if(temp){
		countyList = temp;
		countyList[(*size)++] = county;
	}else{
		fprintf(stderr, "cdp: realloc error!");
		exit(1);
	}
    return countyList;
}

//adds a state to the StateData arraylist
StateData** add_state_data(StateData** stateList, StateData* state, int* size){
	StateData** temp = (StateData**) realloc(stateList, (sizeof(StateData*) * ((*size)+1)));
	if(temp){
		stateList = temp;
		stateList[(*size)++] = state;
	}else{
		fprintf(stderr, "cdp: realloc error!");
		exit(1);
	}
    return stateList;
}

//gets a county from the CountyData arraylist
CountyData* get_county_data(StateData* state, char* countyname){
	for(int i = 0; i<state->size; i++){
		if(strcmp(state->data[i]->name, countyname) == 0){
			return state->data[i];
		}
	}
	return NULL;
}

//gets a state from a StateData arraylsit
StateData* get_state_data(DataList* datalist, char* statename){
	for(int i = 0; i < datalist->size; i++){
		if(strcmp(datalist->data[i]->name, statename) == 0){
			return datalist->data[i];
		}
	}
	return NULL;
}

//adds to the total arrays in the DataList from the data in the county
DataList* updateTotalTotals(DataList** dataList, CountyData* county){
	(*dataList)->totalEducation[0] += county->education[0] * county->population;
	(*dataList)->totalEducation[1] += county->education[1] * county->population;
	for(int i = 0; i < 8; i++){
		(*dataList)->totalEthnicities[i] += county->ethnicities[i] * county->population;
	}
	(*dataList)->totalIncome[0] += county->income[0] * county->population;
	(*dataList)->totalIncome[1] += county->income[1] * county->population;
	(*dataList)->totalIncPBPL += county->incPBPL * county->population;
	(*dataList)->totalPop += county->population;
	return *dataList;
}

//returns a malloc'd copy of SRC county
CountyData* countydup(CountyData* SRC){
	CountyData* county = checked_malloc(sizeof(CountyData));
	county->name = strdup(SRC->name);
	county->incPBPL = SRC->incPBPL;
	county->population = SRC->population;
	memcpy(county->education, SRC->education, sizeof(county->education));
	memcpy(county->ethnicities, SRC->ethnicities, sizeof(county->ethnicities));
	memcpy(county->income, SRC->income, sizeof(county->income));
	return county;
}

//returns a malloc'd copy of SRC state
StateData* statedup(StateData* SRC){
	StateData* state = checked_malloc(sizeof(StateData));
	state->name = strdup(SRC->name);
	state->size = 0;
	state->data = NULL;
	for(int i = 0; i < SRC->size; i++){
		state->data = add_county_data(state->data, countydup(SRC->data[i]), &(state->size));
	}
	return state;
}

//Frees all malloc'd data within and including the DataList structure
void free_data(DataList* data){
	for(int i = 0; i < data->size; i++){
		free_state(data->data[i]);
	}
	free(data->data);
	free(data);
    return;
}

//frees the malloc'd data within and including the StateData structure
void free_state(StateData* data){
	free(data->name);
	for(int i = 0; i < data->size; i++){
		free_county(data->data[i]);
	}
	free(data->data);
	free(data);
	return;
}

//frees the malloc'd data within and including the CountyData Structure
void free_county(CountyData* data){
	free(data->name);
	free(data);
	return;
}
