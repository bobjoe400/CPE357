#ifndef OPERATIONS_H
#define OPERATIONS_H
#include "utility.h"

DataList* display_f(int argc, char* argv[], DataList** data);
DataList* filterState_f(int argc, char* argv[], DataList** data);
DataList* filter_f(int argc, char* argv[], DataList** data);
DataList* populationTotal_f(int argc, char* argv[], DataList** data);
DataList* population_f(int argc, char* argv[], DataList** data);
DataList* percent_f(int argc, char* argv[], DataList** data);

#endif