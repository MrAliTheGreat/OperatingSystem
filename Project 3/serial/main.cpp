#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <limits>

using namespace std;

#define NUM_ROWS_HEADER 1
#define NUM_COLUMNS 21
#define NUM_CLASSES 4

vector<double> extractor(string address){
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

void normalize_dataset(vector<double>& dataset){
	vector<double> max_columns = find_max_column(dataset);
	vector<double> min_columns = find_min_column(dataset);

	for(int j = 0 ; j < NUM_COLUMNS - 1 ; j++){ // excluding last column
		for(int i = j; i < dataset.size() - NUM_COLUMNS + 1 + j; i += NUM_COLUMNS){
			dataset[i] = (double)(dataset[i] - min_columns[j]) / (max_columns[j] - min_columns[j]);
		}
	}
}

vector<int> determine_classes(vector<double>& dataset , vector<double>& weights){
	vector<int> calculated_classes;
	double maxClassValue = numeric_limits<double>::min();
	double classValue = 0;
	int k = 0 , classification;

	for(int rowDataset = 0 ; rowDataset <= (int)((dataset.size() - 1) / NUM_COLUMNS) ; rowDataset++){
		for(int classNum = 0 ; classNum < NUM_CLASSES ; classNum++){
			for(int i = classNum * NUM_COLUMNS ; i < (classNum + 1) * NUM_COLUMNS - 1 ; i++){ // bias excluded
				classValue += weights[i] * dataset[rowDataset * NUM_COLUMNS + k];
				k++;
			}

			classValue += weights[(classNum + 1) * NUM_COLUMNS - 1];

			if(classValue > maxClassValue){
				maxClassValue = classValue;
				classification = classNum;
			}

			k = 0; classValue = 0;			
		}
		calculated_classes.push_back(classification);
		maxClassValue = numeric_limits<double>::min();
	}

	return calculated_classes;
}

double calculate_accuracy(vector<double>& dataset , vector<int>& calculated_classes){
	int j = 0 , corrects = (int)((dataset.size() - 1) / NUM_COLUMNS) + 1;
	
	for(int i = NUM_COLUMNS - 1 ; i < dataset.size(); i += NUM_COLUMNS){
		if((int)dataset[i] != calculated_classes[j]){
			corrects -= 1;
		}
		j++;
	}

	return ((double)corrects / ((int)((dataset.size() - 1) / NUM_COLUMNS) + 1)) * 100;
}



int main(int argc, char *argv[]){
	if(argc == 2){

		string TRAIN_SET_ADDRESS = "./" + string(argv[1]) + "/train.csv";
		string WEIGHTS_ADDRESS = "./" + string(argv[1]) + "/weights.csv";

		vector<double> dataset = extractor(TRAIN_SET_ADDRESS);
		normalize_dataset(dataset);

		vector<double> weights = extractor(WEIGHTS_ADDRESS);
		vector<int> calculated_classes = determine_classes(dataset , weights);

		double accuracy = calculate_accuracy(dataset , calculated_classes);

		printf("Accuracy: %.2f" , accuracy);
		cout << "%\n";
	}else{
		cout << "Invalid Input!\n";
	}

	return 0;
}