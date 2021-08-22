PhonePricePrediction.out : main.o 
	      g++ -std=c++11 -pthread -oPhonePricePrediction.out main.o

main.o: main.cpp
	g++ -std=c++11 -pthread -c main.cpp

clean:
	rm -rf $(BUILD_DIR) *.o *.out