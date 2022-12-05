#define _GNU_SOURCE

#include "utility.h"
#include "operations.h"

#define USAGE_STRING "usage: %s <CORGIS data file name> <operations file name>\n"
#define MAX_ARGS 8
#define NUM_COLUMNS 52

/*
We have already guaranteed that the line is the correct number of fields, but we have yet to
confirm if the data in those fields are valid. 

Using our COLUMN_ENUM that describes the field that our needed data is in, we iterate over those
enums using COLUMN_ITR. During the iteration we use specific ranges in the enums, to determine which
field we are processing. These fields are described in the CountyData struct. 

If there are multiple subfields of a certain field we can use the difference between the first enum 
with that column and the current enum we are on to determine the index of our struct's array for which 
to put that field of data on. 

Ex. We have eight ethnicity fields that we want to use. The first ethnicity field is 
    "Ethnicity.American Indian and American Native Alone" (EthAIaANA), that field is equal to 11 in COLUMNS enum. 
    If we then just subtract the current enum that is selected in COLUMNS_ITR from EthAIaANA we get
    the index in our CountyData ethnicity array that is alighned with that field. 

If there is any error that happens during the data conversion, we go to the loaderror label, clean up
the used resources and return 1. 

If the state that the county is in is not in the DataList, we must add the county to the state, then add the state
to the DataList. If not, just add the county to the state. 

During either of the above, we add the population and income numbers to the total numbers in the DataList using
updateTotals().
*/
//Processes a line of .csv data
int processLine(DataList** data, char* line_arr[]){

    CountyData* county = checked_malloc(sizeof(CountyData));
    StateData* state = checked_malloc(sizeof(StateData));
    county->name = strdup(line_arr[County]);
    state->name = strdup(line_arr[State]);
    state->data = NULL;
    state->size = 0;

    char* str_ptr;
    char* end_ptr;
    for(int i = 2; i<NUMELEMS(COLUMNS_ITR); i++){
        str_ptr = line_arr[COLUMNS_ITR[i]];
        if(COLUMNS_ITR[i] < IMHI){ //we are processing floats
            if(COLUMNS_ITR[i] < EthAIaANA){//we are processing education data
                int education_index = COLUMNS_ITR[i] - EduBach;
                county->education[education_index] = strtof(str_ptr, &end_ptr);
            }else{ // we are processing ethnicity data
                int ethnicities_index = COLUMNS_ITR[i] - EthAIaANA;
                county->ethnicities[ethnicities_index] = strtof(str_ptr, &end_ptr);
            }
        }else{ // we are processing (mostly) ints
            if(COLUMNS_ITR[i] < P2014P){//we are processing income data
                if(COLUMNS_ITR[i] == IPBPL){
                    county->incPBPL = strtof(str_ptr, &end_ptr);
                }else{
                    int income_index = COLUMNS_ITR[i] - IMHI;
                    county->income[income_index] = (int) strtol(str_ptr, &end_ptr, 10);
                }
            }else{ //we are processing 2014 population data
                county->population = (int) strtol(str_ptr, &end_ptr, 10);
            }
        }
        if(errno != 0 || *end_ptr != '\0' || str_ptr==end_ptr){
            goto loadError;
        }
    }

    StateData* temp = NULL;
    if((temp = get_state_data((*data), state->name))!= NULL){
        *data = updateTotalTotals(data, county);
        temp->data = add_county_data(temp->data, county, &(temp->size));
        free_state(state);
    }else{ 
        *data = updateTotalTotals(data, county);
        state->data = add_county_data(state->data, county, &(state->size));
        (*data)->data = add_state_data((*data)->data, state, &((*data)->size));
    }
    return 0;
loadError:
    free(county->name);
    free(county);
    free(state->name);
    free(state);
    return 1;
}

/*
    This file is already guaranteed to exit but each line below the feilds are not guaranteed to have 
    the correct number of columns. 

    We first read the the first line and do nothing with it as this first line is the fields line. 
    If this line is empty, we return an error of an empty file.

    We can then read unil EOF, checking if the lines have the correct number of lines using the
    output of str_to_arr, and if the lines have the correct datatypes using processData(). 
*/
//Processes the input .csv datafile
int load_data(FILE* dataFile, DataList** data){

    char* line = NULL;
    ssize_t num = 0;
    size_t size = 0;

    if((num = getline(&line, &size, dataFile)) == 0){
        free(line);
        fprintf(stderr, "cpd: empty data file");
        return -1;
    }

    int cnt = 0;
    while((num = getline(&line, &size, dataFile))>0){
        char* line_arr[NUM_COLUMNS+1];
        if(str_to_array(line_arr, line, ",\"\n", NUM_COLUMNS)<NUM_COLUMNS){
            fprintf(stderr, "cpd column_check: improperly formatted data line %i \n", cnt);
            cnt+=1;
            continue;
        }
        if(processLine(data, line_arr)){
            fprintf(stderr, "cpd process_line: improperly formatted data line %i \n", cnt);
            cnt+=1;
            continue;
        }
        cnt+=1;
    }
    free(line);
    printf("%i records loaded\n", cnt);
    return 0;
}


/*
We iterate through the lines of our operations file, and on a valid operation, call that operation function 
on our datalist. 

We determine the correct operation using the operation name and whether or not if is in our operation table. 
The operations table contains operation stucts of each of our operations. Details of this struct are defined 
in the operation struct. 

We then pass the operation line into str_to_arr to get an argv and argc for the operation. This and the DataList
are passed into the operation for processing, altering the datalist if requirted. If these produce errors, 
the DataList is guaranteed to not be altered, we print an error, and move to the next operation. 

*/
//Process the operations called upon in our .ops files
void process_operations(FILE* opFile, DataList** data){

    char* line = NULL;
    size_t size = 0; 
    ssize_t num;
    char* operation = NULL;

    int line_no = 0;

    while((num = getline(&line, &size, opFile))>0){
        line_no++;
        operation = strdup(line);
        char* tok = strtok(line, ":\n");
        int i;
        for(i = 0; i<6; i++){
            if(tok == NULL){
                i = -1; 
                break;
            }
            if(strcmp(tok, optable[i].name)==0){
                break;
            }
        }
        if(i == -1){
            free(operation);
            continue;
        }
        if(i == 6){
            fprintf(stderr, "cdp bad_op_type: improperly formamatted data on line %i\n", line_no);
            free(operation);
            continue;
        }
        char* argv[MAX_ARGS];
        memset(argv, 0, sizeof(char*) * MAX_ARGS);
        int argc = str_to_array(argv, operation, ":\n", MAX_ARGS);
        if(validate_arguments(argc, argv, optable[i].arg_num, optable[i].usage_string)<0){
            fprintf(stderr, "cdp too_many_argument: improperly formamatted data on line %i\n", line_no);
            free(operation);
            continue;
        }
        *data = optable[i].op(argc, argv, data, line_no);
        free(operation);
    }
    free(line);
    return;
}

int main(int argc, char* argv[]){

    //validate the arguments provided
    if(validate_arguments(argc, argv, 3, USAGE_STRING)<0){
        exit(EXIT_FAILURE);
    }

    //Open our data and operation files
    FILE *dataFile, *opFile;
    int cnt = 1;
    if((dataFile = fopen(argv[1], "r")) == NULL){
        perror("cdp CORGIS data");
        cnt = 0;
    }
    if((opFile = fopen(argv[2], "r")) == NULL){
        perror("cdp operations");
        cnt = 0;
    }
    if(!cnt){
        if(dataFile) fclose(dataFile);
        if(opFile) fclose(opFile);
        exit(1);
    }

    //initialize our list of data
    DataList* data = checked_malloc(sizeof(DataList));
    data->size = 0;
    data->totalPop = 0;
    memset(data->totalEducation, 0, sizeof(data->totalEducation));
    memset(data->totalEthnicities, 0, sizeof(data->totalEthnicities));
    data->totalIncPBPL = 0;
    data->data = NULL;

    //load and process the data
    if(load_data(dataFile, &data) < 0);
    process_operations(opFile, &data);

    //close the files
    fclose(opFile);
    fclose(dataFile);

    //free the datalist
    free_data(data);
    return 0;
}