#include<iostream>
#include<thread>
#include<mutex>
#include"sampleOnnxMNIST.h"
#include"DataLoader.h"

using namespace std;

int main(int argc, char** argv)
{
	// Multiple thread
	std::mutex* pMutex = new std::mutex;								// Mutex quantity
	std::condition_variable* pCondVal = new std::condition_variable;	// Conditional variable
	bool isArrFull = false;	// If the batch array is full. If full, it can fetch data
	bool isDataTakeOff = false;	// Whether the data has been taken
	bool isProcessOver = false;	// Process is over
	bool pIsSetimgRawToHostBuffer = false;

	/**************************Initialization of data load**********************************/
	SampleOnnxMNIST sample;
	string inputDataDir = "D:\\project\\Pro7-mEDSR-STORM\\code\\FID-STORM-main\\data\\input";	//The raw images folder
	string outputDataDir= "D:\\project\\Pro7-mEDSR-STORM\\code\\FID-STORM-main\\data\\output";//The result folder

	string fileName = inputDataDir + "\\" + "rawImg_256x256.tif";
	//string fileName = inputDataDir + "\\" + "256x256-500.tif";
	int batchSize	= 8;
	bool fp16		= false;
	int modelType	= 256;
	int scaleFactor = 8;

	sample.dataloader.init(fileName, batchSize, pMutex, pCondVal, &isArrFull, &isProcessOver, &isDataTakeOff, &pIsSetimgRawToHostBuffer, fp16);
	/**************************Initialization of network inference**********************************/
	auto sampleTest = sample::gLogger.defineTest("my tensorRT", argc, argv);	// Define a logging class
	sample::gLogger.reportTestStart(sampleTest);								// The start of logging

	// ����Parameter analysis
	sample.initializeSampleParams(inputDataDir, outputDataDir, scaleFactor, modelType, pMutex, pCondVal, &isArrFull, &isProcessOver, &isDataTakeOff, &pIsSetimgRawToHostBuffer, fp16);	// Initialization parameters		

	// ����Constructing a Network
	if (!sample.build())	return sample::gLogger.reportFail(sampleTest);		// 

	/**************************Network inference execution**********************************/
	auto startTime = chrono::high_resolution_clock::now();	// Current clock
	// ����Infer,multiple thread
	thread myThreadDataLoad(&DataLoader::imgRead, std::ref(sample.dataloader));	// Thread 1��Keep loading data into the array
	thread myThreadInfer(&SampleOnnxMNIST::infer, std::ref(sample));			// Thread 2��Keep pulling out the data and inferring

	myThreadDataLoad.join();
	myThreadInfer.join();

	auto  endTime = chrono::high_resolution_clock::now();
	float totalTime = chrono::duration<float, milli>(endTime - startTime).count();

	cout << "total time��" << totalTime << " ms" << endl;
	/**************************Free memory**********************************/
	sample.dataloader.deInit();
	delete pMutex;				// Release the mutex
	delete pCondVal;			// Release conditional variable

	sample.deinit();

	system("pause");
	return 0;
}