#include "Header.h"
#include <direct.h>
#include <stdlib.h>
#include <atltime.h>
#include <fstream>

 DataArray<int> IsAvoid4231(DataArray<int> data, bool& fAvoid, int& atIdx)
{
	atIdx = -1;
	for (int idx1 = 0; idx1 < data.Size; idx1++)
	{
		// Extract all smaller numbers downward
		DataArray<int> checkData = data.ExtractDownward(CompareTypes::LT, data.Value[idx1], idx1 + 1, data.Size - 1);
		if (checkData.Size > 0)
		{
			DataArray<int> newData = Create231AvoidCheck(checkData);
			checkData.DeAlloc();

			if (newData.IsIncrease() == false)
			{
				fAvoid = false;
				atIdx = idx1;
				return newData;
			}
		}
	}
	fAvoid = true;
	DataArray<int> emptyCheckData;
	return emptyCheckData;
}

// Test all data files if they are avoid 4231.
// The test uses reliable 4231 algorithm
void MainTest4231()
{
	int seedSz = 500, decLoopCount = 1500, incLoopCount = 1500, fullLoopCount = 30;
	AskSeedParams(seedSz, decLoopCount, incLoopCount, fullLoopCount);

	string outFile = OUTPUT_FOLDER + "\\test4231.txt";
	ofstream myfile(outFile, std::ios::out);
	if (myfile.is_open() == false)
	{
		cout << "Unable to open file " << outFile << endl;
		return;
	}

	int dataSz = seedSz * (decLoopCount + incLoopCount) + seedSz;
	DataArray<int> data;
	data.Alloc(dataSz);
	bool fAvoid;
	int atIdx;

	for (int idx = 0; idx < fullLoopCount; idx++)
	{
		string fileName = OUTPUT_FOLDER + "//insert-" + std::to_string(idx) + ".txt";
		cout << fileName << endl;

		myfile << idx << "\t";
		if (ReadData(data, fileName) == false)
		{
			myfile << "NOT-FOUND" << endl;
			continue;
		}
		DataArray<int> retData = IsAvoid4231(data, fAvoid, atIdx);
		if (fAvoid) myfile << "AVOID" << endl;
		else
		{
			//myfile << "NOT-AVOID\t" << atIdx << "\t" << data.Value[atIdx] << "\t" << retData.ToString()<<endl;
			myfile << "NOT-AVOID\t" << atIdx << "\t" << data.Value[atIdx] << endl;
		}
		retData.DeAlloc();
	}
	myfile.close();
	data.DeAlloc();
}

// Test data if they are avoid 4231 
void MainTest4231Item()
{
	int insertPos = 2;
	int seedSz = 6, multifier = 3;
	int insertFileId;

	seedSz = AskNumber("Enter Seed size (default = 3): ", 3,1);
	multifier = AskNumber("Enter Extend Loop multifier (default = 1): ", 1, 0);
	insertPos = AskNumber("Enter insert position: (default = 0) ", 0);
	insertFileId = AskNumber("Enter insert file Id (default = 0): ",0);

	int decLoopCount = multifier * seedSz;
	int incLoopCount = multifier * seedSz;
	string fileName = OUTPUT_FOLDER + "\\insert-" + std::to_string(insertFileId) + ".txt";
	int dataSz = seedSz * (decLoopCount + incLoopCount) + seedSz;

	DataArray<int> data;
	data.Alloc(dataSz);
	
	if (ReadData(data, fileName) == false) return;
	bool fAvoid = IsAvoid4231_NealMadras(data,  decLoopCount*seedSz + insertPos);
	data.DeAlloc();
	cout << (fAvoid ? "AVOID" : "NOTAVOID") << endl;
}