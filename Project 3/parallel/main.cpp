#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <limits>

#include <pthread.h>

using namespace std;

#define NUM_ROWS_HEADER 1
#define NUM_COLUMNS 21
#define NUM_CLASSES 4

#define NUMBER_OF_THREADS 4

vector<vector<double> > dataset_threads; vector<vector<double> > min_threads; vector<vector<double> > max_threads;
vector<vector<int> > calculated_classes_threads;
vector<double> weights; vector<double> maxAllThreads; vector<double> minAllThreads;
double accuracies[NUMBER_OF_THREADS];
string TRAIN_SET_ADDRESS;



vector<double> extractor_weights(string address){
	vector<double> extracted;
	stringstream ss_line;
	string line , number;
	int counter_line = 1;
	fstream file(address);

	while(getline(file , line)){
		ss_line.str(line);
		ss_line.clear();
		if(counter_line > NUM_ROWS_HEADER){
			while(getline(ss_line , number , ',')){
				extracted.push_back(stod(number));
			}
		}
		counter_line++;
	}
	file.close();
	return extracted;
}

vector<double> find_min_column(vector<double> dataset){
	vector<double> min_columns;
	double min;

	for(int j = 0 ; j < NUM_COLUMNS - 1 ; j++){ // excluding last column
		min = numeric_limits<double>::max();
		for(int i = j; i < dataset.size() - NUM_COLUMNS + 1 + j; i += NUM_COLUMNS){
			if(dataset[i] < min){
				min = dataset[i];
			}
		}
		min_columns.push_back(min);
	}

	return min_columns;		
}

vector<double> find_max_column(vector<double> dataset){
	vector<double> max_columns;
	double max;

	for(int j = 0 ; j < NUM_COLUMNS - 1 ; j++){ // excluding last column
		max = numeric_limits<double>::min();
		for(int i = j; i < dataset.size() - NUM_COLUMNS + 1 + j; i += NUM_COLUMNS){
			if(dataset[i] > max){
				max = dataset[i];
			}
		}
		max_columns.push_back(max);
	}	

	return max_columns;
}

void* extractorAndMinMax(void* tid){
	string address = TRAIN_SET_ADDRESS + to_string((long)tid) + ".csv";
	stringstream ss_line;
	string line , number;
	int counter_line = 1;
	fstream file(address);

	while(getline(file , line)){
		ss_line.str(line);
		ss_line.clear();
		if(counter_line > NUM_ROWS_HEADER){
			while(getline(ss_line , number , ',')){
				if(dataset_threads[(long)tid][0] == numeric_limits<double>::min()){
					dataset_threads[(long)tid][0] = stod(number);
				}else{
					dataset_threads[(long)tid].push_back(stod(number));
				}
			}
		}
		counter_line++;
	}
	file.close();

	min_threads[(long)tid] = find_min_column(dataset_threads[(long)tid]);
	max_threads[(long)tid] = find_max_column(dataset_threads[(long)tid]);

	pthread_exit(NULL);
}

void* normalizeDataset_determineClasses_calculateAccuracy(void* tid){
	for(int j = 0 ; j < NUM_COLUMNS - 1 ; j++){ // excluding last column
		for(int i = j; i < dataset_threads[(long)tid].size() - NUM_COLUMNS + 1 + j; i += NUM_COLUMNS){
			dataset_threads[(long)tid][i] = (double)(dataset_threads[(long)tid][i] - minAllThreads[j]) / (maxAllThreads[j] - minAllThreads[j]);
		}
	}

	double maxClassValue = numeric_limits<double>::min();
	double classValue = 0;
	int k = 0 , classification;

	for(int rowDataset = 0 ; rowDataset <= (int)((dataset_threads[(long)tid].size() - 1) / NUM_COLUMNS) ; rowDataset++){
		for(int classNum = 0 ; classNum < NUM_CLASSES ; classNum++){
			for(int i = classNum * NUM_COLUMNS ; i < (classNum + 1) * NUM_COLUMNS - 1 ; i++){ // bias excluded
				classValue += weights[i] * dataset_threads[(long)tid][rowDataset * NUM_COLUMNS + k];
				k++;
			}

			classValue += weights[(classNum + 1) * NUM_COLUMNS - 1];

			if(classValue > maxClassValue){
				maxClassValue = classValue;
				classification = classNum;
			}

			k = 0; classValue = 0;			
		}

		if(calculated_classes_threads[long(tid)][0] == -1){
			calculated_classes_threads[long(tid)][0] = classification;
		}else{
			calculated_classes_threads[long(tid)].push_back(classification);
		}
		maxClassValue = numeric_limits<double>::min();
	}

	int j = 0 , corrects = (int)((dataset_threads[(long)tid].size() - 1) / NUM_COLUMNS) + 1;
	
	for(int i = NUM_COLUMNS - 1 ; i < dataset_threads[(long)tid].size(); i += NUM_COLUMNS){
		if((int)dataset_threads[(long)tid][i] != calculated_classes_threads[long(tid)][j]){
			corrects -= 1;
		}
		j++;
	}

	accuracies[long(tid)] = ((double)corrects / ((int)((dataset_threads[(long)tid].size() - 1) / NUM_COLUMNS) + 1)) * 100;	

	pthread_exit(NULL);
}

void initialize_vector_double(vector<double> temp_vector_double){
	for(int i = 0 ; i < NUMBER_OF_THREADS ; i++){
		dataset_threads.push_back(temp_vector_double);
		min_threads.push_back(temp_vector_double);
		max_threads.push_back(temp_vector_double);
	}
}

void initialize_vector_int(vector<int> temp_vector_int){
	for(int i = 0 ; i < NUMBER_OF_THREADS ; i++){
		calculated_classes_threads.push_back(temp_vector_int);
	}	
}



int main(int argc , char* argv[]){
	if(argc == 2){
		string WEIGHTS_ADDRESS = "./" + string(argv[1]) + "/weights.csv";
		TRAIN_SET_ADDRESS = "./" + string(argv[1]) + "/train_";

		weights = extractor_weights(WEIGHTS_ADDRESS);

		pthread_t threads[NUMBER_OF_THREADS];
		int return_code; void* status;

		vector<double> temp_vector_double; temp_vector_double.push_back(numeric_limits<double>::min());
		vector<int> temp_vector_int; temp_vector_int.push_back(-1);

		initialize_vector_double(temp_vector_double);
		initialize_vector_int(temp_vector_int);


		for(long tid = 0; tid < NUMBER_OF_THREADS; tid++){

			return_code = pthread_create(&threads[tid] , NULL , extractorAndMinMax , (void*) tid);

			if (return_code){
				printf("ERROR EXTRACTOR_MINMAX; return code from pthread_create() is %d\n",
						return_code);
				exit(-1);
			}
		}

		for(long tid = 0; tid < NUMBER_OF_THREADS; tid++){
			return_code = pthread_join(threads[tid], &status);
		}

		double maxValueAllThreads = numeric_limits<double>::min();
		double minValueAllThreads = numeric_limits<double>::max();

		for(int j = 0 ; j < NUM_COLUMNS - 1 ; j++){ // excluding last column
			for(int i = 0; i < max_threads.size(); i++){
				if(max_threads[i][j] > maxValueAllThreads){
					maxValueAllThreads = max_threads[i][j];
				}

				if(min_threads[i][j] < minValueAllThreads){
					minValueAllThreads = min_threads[i][j];
				}
			}
			maxAllThreads.push_back(maxValueAllThreads);
			minAllThreads.push_back(minValueAllThreads);

			maxValueAllThreads = numeric_limits<double>::min(); minValueAllThreads = numeric_limits<double>::max();
		}		

		for(long tid = 0; tid < NUMBER_OF_THREADS; tid++){

			return_code = pthread_create(&threads[tid] , NULL , normalizeDataset_determineClasses_calculateAccuracy , (void*) tid);

			if (return_code){
				printf("ERROR NORMALIZE_DETERMINE_ACCURACY; return code from pthread_create() is %d\n",
						return_code);
				exit(-1);
			}
		}

		for(long tid = 0; tid < NUMBER_OF_THREADS; tid++){
			return_code = pthread_join(threads[tid], &status);
		}

		double final_accuracy = 0;

		for(int i = 0 ; i < NUMBER_OF_THREADS ; i++){
			final_accuracy += accuracies[i];
		}

		printf("Accuracy: %.2f" , (double)final_accuracy / NUMBER_OF_THREADS);
		cout << "%\n";

	}else{
		cout << "Invalid Input\n";
	}

	pthread_exit(NULL);
}