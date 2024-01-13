#include "Header.h"
#include <list>
#include <random>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <iterator>
#include <windows.h>
using namespace std;

void GenXRange(int min, int max, DataArray<int>& data)
{
	data.Alloc(max - min);
	for (int idx = 0; idx < data.Size; idx++)
	{
		data.Value[idx] = idx + min + 1;
	}
}
int Random(int min, int max)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dist(min, max);
	return dist(gen);
}

bool ReadData(DataArray<int> data, string fileName)
{	
	ifstream myfile(fileName);
	if (myfile.is_open() == false)
	{
		cout << "Unable to open file " << fileName << endl;
		return false;
	}
	string line;
	for (int id1 = 0; id1 < data.Size; id1++)
	{
		if (not getline(myfile, line))
		{
			myfile.close();
			return false;
		}
		data.Value[id1] = std::stoi(line);
	}
	myfile.close();
	return true;
}

void SetLogFileNew()
{
	SetLogFileCount(LOG_FILE_COUNT+1);
}
void SetLogFileStart()
{
	SetLogFileCount(0);
}
void SetLogFileCount(int count)
{
	LOG_FILE_COUNT = count;
	remove(GetLogFile().c_str());
}
string GetLogFile()
{
	return OUTPUT_FOLDER + "//" + LOG_FILE_NAME + "-" + std::to_string(LOG_FILE_COUNT) + LOG_FILE_EXT;
}
bool WriteLog(string line, string fileName)
{
	if (fileName.empty()) fileName = GetLogFile();
	ofstream myfile(fileName, std::ios::out | std::ios::app);
	if (myfile.is_open() == false)
	{
		cout << "Unable to open file " << fileName << endl;
		return false;
	}
	myfile << line << endl;
	myfile.close();
	return true;
}

void WriteError(string line)
{
	string fileName = OUTPUT_FOLDER + "//error.txt";
	ofstream myfile(fileName, std::ios::out | std::ios::app);
	if (myfile.is_open() == false)
	{
		cout << "Unable to open file " << fileName << endl;
		return;
	}
	myfile << TimeToStrc(time(0)) << '\t' << line << endl;
	myfile.close();
}

bool WriteStats(Stats stats)
{
	stats.EndTime = time(0);
	string fileName = GetLogFile();
	ofstream myfile(fileName, std::ios::out | std::ios::app);
	if (myfile.is_open() == false)
	{
		cout << "Unable to open file " << fileName << endl;
		return false;
	}
	myfile << endl;
	myfile << "Stats" << endl;
	myfile << "-Good seed count: " << stats.GoodSeed << endl;
	myfile << "-Avoid pattern count: " << stats.PatternAvoid << endl;
	if (stats.Idx > 0)
	{
		myfile << "-Avoid ratio: " << (double)(stats.PatternAvoid * 100) / (stats.Idx) << " %" << endl;
	}
	myfile << "-Start time: " << TimeToStrc(stats.StartTime);
	myfile << "-End time: " << TimeToStrc(stats.EndTime);
	myfile.close();
	return true;
}
void ShowStats(Stats stats)
{
	cout << endl;
	cout << "Stats" << endl;
	cout << "-Good seed count: " << stats.GoodSeed << endl;
	cout << "-Avoid pattern count: " << stats.PatternAvoid << endl;
	if (stats.FullLoopCount > 0)
	{
		cout << "-Avoid ratio: " << (double)(stats.PatternAvoid * 100) / stats.FullLoopCount << endl;
	}
	cout << "-Start time: " << TimeToStrc(stats.StartTime);
	cout << "-End time: " << TimeToStrc(stats.EndTime);
}

char AskCharacter(string text, char defaultChar, string allowChars)
{
	string str;
	while (true)
	{
		cout << text;
		getline(cin, str);
		if (str.empty()) return defaultChar;
		if (allowChars.empty()) return str[0];
		int idx = allowChars.find(tolower(str[0]));
		if (idx >= 0) return str[0];
	}
}

int AskNumber(string text, int defaultValue, int min, int max)
{
	string str;
	int number = 0;
	while (true)
	{
		cout << text;
		getline(cin, str);
		if (str.empty()) number = defaultValue;
		else
		{
			char* p;
			number = strtol(str.c_str(), &p, 10);
			// conversion failed because the input wasn't a number
			if (*p != '\0') continue;
		}
		if (number >= min && max >= number) return number;
	}
}
void AskSeedParams(int& seedSz, int& decLoopCount, int& incLoopCount, int& fullLoopCount)
{
	int multifier = 3;
	seedSz = AskNumber("Enter Seed size (range [2-1000], default = 3): ", 3, 2,1000);
	multifier = AskNumber("Enter Extend Loop multifier(range [0-10], default = 1): ",1, 0,10);
	fullLoopCount = AskNumber("Enter Full Loop Count (default = 100): ",100, 1);
	decLoopCount = multifier * seedSz;
	incLoopCount = multifier * seedSz;

	//decLoopCount = 1;
	//incLoopCount = 1;
}

string PatternTypesToStr(PatternTypes pattern)
{
	switch (pattern)
	{
		case PatternTypes::P321:	return "P321";
		case PatternTypes::P4321:	return "P4321";
		case PatternTypes::P231:	return "P231";
		case PatternTypes::P4231:	return "P4231";
		case PatternTypes::P4231Test:	return "P4231Test";
	}
	return "NA";
}

int GetDeletePos(int rangeSz)
{
	return Random(0, rangeSz - 1);
}
int GetInsertPos(int rangeSz)
{
	return Random(0, rangeSz);
}
int GetInsertValue(int rangeSz, int insertPos)
{
	return Random(insertPos - rangeSz + 1, insertPos + rangeSz + 1);
}

bool Clone(DataArray<int> seeds, int decLoopCount, int incLoopCount, DataArray<int>& toData)
{
	int seedSz = seeds.Size;
	toData.Alloc(seedSz * (decLoopCount + incLoopCount + 1));

	// Negative clone data from seed
	for (int id1 = decLoopCount-1; id1 >=0; id1--)
	{
		int startVal = seedSz * (decLoopCount-id1);
		for (int id2 = seedSz-1; id2 >=0; id2--)
		{
			toData.Value[id2 + seedSz * id1] = seeds.Value[id2] - startVal;
		}
	}
	// Seed and Positive clone data from seed
	for (int id1 = 0; id1 < incLoopCount + 1; id1++)
	{
		int startVal = seedSz * id1;
		for (int id2 = 0; id2 < seedSz; id2++)
		{
			toData.Value[id2 + seedSz * (id1 + decLoopCount)] = seeds.Value[id2] + startVal;
		}
	}
	return true;
}

string TimeToStrc(time_t t)
{
	char str[26];
	ctime_s(str, sizeof str, &t);
	string retVal = str;
	return retVal;
}

void ShowResult(string title, DataArray<int> data, int seedSz)
{
	cout << title;
	for (int id1 = 0; id1 < data.Size; id1++)
	{
		if (id1 % seedSz == 0) cout << endl;
		cout << data.Value[id1] << ":";
	}
	cout << endl;
}
void ShowString(string title)
{
	cout << title << endl;
}

// Slit an increasing series from a data serie.
void PatternSplit(DataArray<int> data, DataArray<int>& increaseList, DataArray<int>& otherList)
{
	if (data.Size == 0) return;
	DataArray<int> tmpList1, tmpList2;
	tmpList1.Alloc(data.Size);
	tmpList2.Alloc(data.Size);
	int list1Idx = -1, list2Idx = -1;

	tmpList1.Value[++list1Idx] = data.Value[0];
	for (int id1 = 1; id1 < data.Size; id1++)
	{
		if (data.Value[id1] > tmpList1.Value[list1Idx])
		{
			tmpList1.Value[++list1Idx] = data.Value[id1];
		}
		else
		{
			tmpList2.Value[++list2Idx] = data.Value[id1];
		}
	}
	increaseList.DeAlloc();
	increaseList.Alloc(list1Idx + 1); memcpy(increaseList.Value, tmpList1.Value, increaseList.Size * sizeof(int));
	otherList.Alloc(list2Idx + 1); memcpy(otherList.Value, tmpList2.Value, otherList.Size * sizeof(int));
	tmpList1.DeAlloc();
	tmpList2.DeAlloc();
}

DataArray<int> CreateSeedsFromData(DataArray<int> data, int atIdx, int sz)
{
	DataArray<int> seeds;
	if (sz < 0 || sz>data.Size) return seeds;
	seeds.Alloc(sz);

	// Copy not-deleted data
	memcpy(seeds.Value, data.Value + atIdx, sz * sizeof(int));
	return seeds;
}

bool CreateSeedsByInsert(int atPos, int value, DataArray<int> seeds, DataArray<int>& newSeeds) // Insertion
{
	int seedSz = seeds.Size;
	if (seedSz<=0 || atPos<0 || atPos> seedSz) return false;

	newSeeds.Alloc(seedSz + 1);
	// Copy old data
	memcpy(newSeeds.Value, seeds.Value, atPos * sizeof(int));
	memcpy(newSeeds.Value + atPos + 1, seeds.Value + atPos, (seedSz - atPos) * sizeof(int));
	// Put the inserted value in the correct place
	newSeeds.Value[atPos] = value;
	for (int id1 = 0; id1 < seedSz+1; id1++)
	{
		if (id1 == atPos) continue;
		int k = ceil((double)(newSeeds.Value[id1] - value +1 ) / seedSz);
		newSeeds.Value[id1] = newSeeds.Value[id1] + k;
	}
	return true;
}

bool CreateByDelete(int atSeedPos, DataArray<int> data, int seedSz, int decLoopCount, int incLoopCount, DataArray<int>& newData) // Deletion
{
	if (seedSz < 1 || atSeedPos>= seedSz) return false;
	int partCount = data.Size / seedSz;

	int newSeedSz = seedSz - 1;
	newData.Alloc(partCount * newSeedSz);

	// Copy not-deleted data
	memcpy(newData.Value, data.Value, atSeedPos * sizeof(int));
	for (int id1 = 0; id1 < partCount-1; id1++)
	{
		int deletePos = atSeedPos + id1 * seedSz;
		memcpy(newData.Value+ atSeedPos+(id1*newSeedSz), data.Value + deletePos+1, newSeedSz*sizeof(int));
	}
	int dataIdx = atSeedPos + (partCount - 1) * seedSz;
	int newDataIdx = atSeedPos + (partCount - 1) * newSeedSz;
	memcpy(newData.Value + newDataIdx, data.Value + dataIdx+1, (newData.Size- newDataIdx+1) * sizeof(int));
	// Calculate new value
	int newSeedPos = atSeedPos + decLoopCount * seedSz;
	for (int id1 = 0; id1 < partCount * newSeedSz; id1++)
	{
		int k = ceil((double)((long)newData.Value[id1] - data.Value[newSeedPos]) / seedSz);
		newData.Value[id1] = newData.Value[id1] - k;
	}
	return true;
}

bool IsGoodSeeds(DataArray<int> data) // Boundedness condition
{
	int total = 0;
	for (int id1 = 0; id1 < data.Size; id1++)
	{
		if (abs(data.Value[id1] - id1 - 1) >= data.Size)
			return false;
		total += data.Value[id1];
	}
	int checkTotal = (int)(data.Size * (1 + data.Size) / 2);
	if (total == checkTotal) return true;
	return false;
}
// Extended GSS to check for 4231 algorithm. 
DataArray<int> Create231AvoidCheck(DataArray<int> data)
{
	int dataSz = data.Size;

	DataArray<int> newData;
	if (dataSz == 0) return newData;
	newData.Alloc(dataSz);
	int newSeedIdx = -1;

	DataArray<int> stack;
	stack.Alloc(data.Size);

	for (int id1 = 0; id1 < dataSz; id1++)
	{
		while (true)
		{
			if (stack.CurrentIdx < 0 || data.Value[id1] < stack.ValueAtTop)
			{
				stack.Push(data.Value[id1]);
				break;
			}
			int valueAtTop = stack.Pop();
			newData.Push(valueAtTop);
		}
	}
	while (stack.CurrentIdx >= 0)
	{
		int valueAtTop = stack.Pop();
		newData.Push(valueAtTop);
	}
	stack.DeAlloc();
	return newData;
}

// Test if a string avoid-231 using GSS algorithm
bool IsAvoid231_NealMadras(DataArray<int> data)
{
	DataArray<int> newData = Create231AvoidCheck(data);
	bool retVal = newData.IsIncrease();
	newData.DeAlloc();
	return retVal;
}

// 4231 new algorithm
bool IsAvoid4231_NealMadras(DataArray<int> data, int insertPos)
{
	bool fAvoid;
	if (insertPos < 0 || insertPos >= data.Size)
	{
		WriteError("Error IsAvoid4231-1: " + std::to_string(insertPos) + "/" + std::to_string(data.Size));
		return false;
	}

	int insertValue = data.Value[insertPos];

	// Step 1: i = i_1 (Extract all smaller numbers downward)
	DataArray<int> checkData = data.ExtractDownward(CompareTypes::LT, insertValue, insertPos + 1, data.Size - 1);
	if (checkData.Size >= 2)
	{
		if (IsAvoid231_NealMadras(checkData) == false)
		{
			checkData.DeAlloc();
			return false;
		}
	}
	checkData.DeAlloc();

	// Step 2: i = i_2
	
	// Step 2.1: Check upward for the greatest value of all greater one than the insert value
	int foundIdx = data.FindGreatest(insertValue, 0, insertPos - 1);
	if (foundIdx >= 0)
	{
		// Step 2.2 : find downward a value in the range
		foundIdx = data.FindFirstInRange(insertValue, data.Value[foundIdx], insertPos + 1, data.Size - 1);

		if (foundIdx >= 0)
		{
			// Step 2.3
			foundIdx = data.FindFirst(CompareTypes::LT, insertValue, foundIdx + 1, data.Size - 1);
			if (foundIdx >= 0) return false;
		}
	}
	// Step 3: i = i_3
	// Step 3.1: find the smallest number downward
	foundIdx = data.FindSmallest(insertValue, insertPos + 1, data.Size - 1);

	if (foundIdx >= 0)
	{
		// Step 3.2
		foundIdx = data.FindLastInRange(data.Value[foundIdx], insertValue, 0, insertPos - 1);

		if (foundIdx >= 0)
		{
			// Step 3.3 : find a greater value upward
			foundIdx = data.FindFirst(CompareTypes::GT, insertValue, 0, foundIdx - 1);
		}
		
		if (foundIdx >= 0)	return false;
	}

	// Step 4: i = i_4 (Extract all greater numbers upward)
	checkData = data.ExtractUpward(CompareTypes::GT, insertValue, 0, insertPos - 1);
	if (checkData.Size >= 2)
	{
		checkData.Negative();
		fAvoid = IsAvoid231_NealMadras(checkData);
	}
	else
	{
		fAvoid = true;
	}
	checkData.DeAlloc();
	return fAvoid;
}

// Test if a string avoid-4231 using GSS extended algorithm
bool IsAvoid4231_Nomal(DataArray<int> data)
{
	for (int idx1 = 0; idx1 < data.Size; idx1++)
	{
		// Extract all smaller numbers downward
		DataArray<int> checkData = data.ExtractDownward(CompareTypes::LT, data.Value[idx1], idx1 + 1, data.Size - 1);
		if (checkData.Size > 0)
		{
			if (IsAvoid231_NealMadras(checkData) == false)
			{
				checkData.DeAlloc();
				return false;
			}
		}
	}
	return true;
}

void BldResult::DeAlloc()
{
	this->xRange.DeAlloc();
	this->Clone.DeAlloc();
	this->Insert.DeAlloc();
	this->Delete.DeAlloc();
	this->NewSeeds.DeAlloc();
}

template <typename T>
void DataArray<T>::Alloc(int sz)
{
	// Release the allocated mem if any
	this->DeAlloc();
	
	if (sz <= 0)
	{
		//WriteError("-Error: Alloc() invalid size " + std::to_string(sz));
		this->_sz = sz;
		return;
	}

	this->_ptr = (T*)malloc(sizeof(T) * sz);
	if (this->_ptr != NULL)
	{
		this->_sz = sz;
		return;
	}
	WriteError("-Error: Alloc() " + std::to_string(sizeof(T) * sz) + " bytes");
}

template <typename T>
void DataArray<T>::DeAlloc()
{
	if (this->_ptr != NULL)
	{
		free(this->_ptr);
		//delete[] this->_ptr;
		this->_ptr = NULL;
	}
	this->_sz = 0;
	this->_currentIdx = -1;
}

template <typename T>
bool DataArray<T>::CopyTo(DataArray<T> data, int sz)
{
	if (data.Size < sz) return false;
	memcpy(data.Value, this->Value, sz * sizeof(int));
	return true;
}

// Rotate the serie 180 degree clockwise
template <typename T>
void DataArray<T>::Transform180()
{
	int value;
	int dataSz = this->Size;
	for (int idx1 = 0,idx2 = dataSz-1; idx1 <= idx2; idx1++,idx2--)
	{
		value = this->Value[idx2];
		this->Value[idx2] = this->Value[idx1];
		this->Value[idx1] = value;
	}
}

// Negate all data
template <typename T>
void DataArray<T>::Negative()
{
	int dataSz = this->Size;
	for (int idx1 = 0; idx1 < dataSz; idx1++)
	{
		this->Value[idx1] = -this->Value[idx1];
	}
}

template <typename T>
DataArray<T> DataArray<T>::ExtractDownward(CompareTypes type, T value, int fromIdx, int toIdx)
{
	DataArray<int> checkData1, checkData2;
	checkData1.Alloc(toIdx - fromIdx + 1);
	int count = 0;
	switch (type)
	{
		case CompareTypes::LT: // less than
			for (int idx1 = fromIdx; idx1 <= toIdx; idx1++)
			{
				if (this->Value[idx1] < value) checkData1.Value[count++] = this->Value[idx1];
			}
			break;
		case CompareTypes::GT: // greater than
			for (int idx1 = fromIdx; idx1 <= toIdx; idx1++)
			{
				if (this->Value[idx1] > value) checkData1.Value[count++] = this->Value[idx1];
			}
			break;
		case CompareTypes::LE: // smallest
			for (int idx1 = fromIdx; idx1 <= toIdx; idx1++)
			{
				if (this->Value[idx1] <= value) checkData1.Value[count++] = this->Value[idx1];
			}
			break;
		case CompareTypes::GE: // greatest
			for (int idx1 = fromIdx; idx1 <= toIdx; idx1++)
			{
				if (this->Value[idx1] >= value) checkData1.Value[count++] = this->Value[idx1];
			}
			break;
	}
	if (count > 0)
	{
		checkData2.Alloc(count);
		checkData1.CopyTo(checkData2, count);
	}
	checkData1.DeAlloc();
	return checkData2;
}

template <typename T>
DataArray<T> DataArray<T>::ExtractUpward(CompareTypes type, T value, int fromIdx, int toIdx)
{
	DataArray<int> checkData1, checkData2;
	checkData1.Alloc(toIdx - fromIdx + 1);
	int count = 0;
	switch (type)
	{
	case CompareTypes::LT:
		for (int idx1 = toIdx; idx1 >= fromIdx; idx1--)
		{
			if (this->Value[idx1] < value) checkData1.Value[count++] = this->Value[idx1];
		}
		break;
	case CompareTypes::GT:
		for (int idx1 = toIdx; idx1 >= fromIdx; idx1--)
		{
			if (this->Value[idx1] > value) checkData1.Value[count++] = this->Value[idx1];
		}
		break;
	case CompareTypes::LE:
		for (int idx1 = toIdx; idx1 >= fromIdx; idx1--)
		{
			if (this->Value[idx1] <= value) checkData1.Value[count++] = this->Value[idx1];
		}
		break;
	case CompareTypes::GE:
		for (int idx1 = toIdx; idx1 >= fromIdx; idx1--)
		{
			if (this->Value[idx1] >= value) checkData1.Value[count++] = this->Value[idx1];
		}
		break;
	}
	if (count > 0)
	{
		checkData2.Alloc(count);
		checkData1.CopyTo(checkData2, count);
	}
	checkData1.DeAlloc();
	return checkData2;
}

template <typename T>
bool DataArray<T>::Push(T value)
{
	if (this->_currentIdx < 0)
	{
		this->_currentIdx = 0;
		this->_ptr[this->_currentIdx] = value;
		return true;
	}
	if (this->_currentIdx < this->_sz - 1)
	{
		this->_currentIdx++;
		this->_ptr[this->_currentIdx] = value;
		return true;
	}
	return false;
}

template <typename T>
T DataArray<T>::Pop()
{
	if (this->_currentIdx < 0)	WriteError("Error: Pop()");
	return this->_ptr[this->_currentIdx--];
}

//template <typename T>
string ToString(DataArray<int> data, string separator)
{
	string str = "";
	for (int idx = 0; idx < data.Size; idx++)
	{
		str += std::to_string(data.Value[idx]) + separator;
	}
	return str;
}

template <typename T>
bool SaveData(int arrSz, DataArray<T> data[], string fileName)
{
	ofstream myfile(fileName);
	if (myfile.is_open() == false)
	{
		cout << "Unable to open file " << fileName << endl;
		return false;
	}
	int maxItemSz = 0;
	for (int id1 = 0; id1 < arrSz; id1++)
	{
		if (maxItemSz < data[id1].Size) maxItemSz = data[id1].Size;
	}
	for (int id1 = 0; id1 < maxItemSz; id1++)
	{
		for (int id2 = 0; id2 < arrSz; id2++)
		{
			if (id1 < data[id2].Size) myfile << data[id2].Value[id1];
			else myfile << " ";
			if (id2 < arrSz - 1) myfile << "\t";
		}
		myfile << endl;
	}
	myfile.close();
	return true;
}
bool SaveDataINT(int arrSz, DataArray<int> data[], string fileName)
{
	return SaveData(arrSz, data, fileName);
}

bool CalcPatternWidth(int atLoop, DataArray<int> data, int xShiff, string fileName, bool saveData) // Width
{
	DataArray<int> idxListInc = data.ExtractToCalcWidth(true);
	DataArray<int> incList;
	incList.Alloc(idxListInc.Size);
	double totalIncHeight = 0;
	for (int idx = 0; idx < idxListInc.Size; idx++)
	{
		int pos = xShiff + idxListInc.Value[idx];
		totalIncHeight += data.Value[idxListInc.Value[idx]] - pos;
		incList.Value[idx] = data.Value[idxListInc.Value[idx]];
	}
	if (idxListInc.Size > 0) totalIncHeight = totalIncHeight / idxListInc.Size;

	DataArray<int> idxListDec = data.ExtractToCalcWidth(false);
	DataArray<int> decList;
	decList.Alloc(idxListDec.Size);
	double totalDecHeight = 0;
	for (int idx = 0; idx < idxListDec.Size; idx++)
	{
		int pos = xShiff + idxListDec.Value[idx];
		totalDecHeight += (int)(pos - data.Value[idxListDec.Value[idx]]);
		decList.Value[idx] = data.Value[idxListDec.Value[idx]];
	}
	if (idxListDec.Size > 0) totalDecHeight = totalDecHeight / idxListDec.Size;

	idxListInc.DeAlloc();
	idxListDec.DeAlloc();
	
	// Save data file for checking
	if (saveData)
	{
		DataArray<int> arr[] = { decList, incList };
		SaveData(2, arr, OUTPUT_FOLDER + "\\" + PATTERN_WIDTH_DATA_FILE_PREFIX + std::to_string(atLoop) + ".txt");
	}
	decList.DeAlloc();
	incList.DeAlloc();
	
	// Save summary file
	ofstream myfile(fileName, std::ios::out | std::ios::app);
	if (myfile.is_open() == false)
	{
		cout << "Unable to open file " << fileName << endl;
		return false;
	}
	myfile << "Loop " << atLoop << "\t" << totalDecHeight + totalIncHeight << "\t" << totalDecHeight << "\t" << totalIncHeight << endl;
	myfile.close();
	return true;
}

template <typename T>
bool DataArray<T>::IsIncrease()
{
	for (int id1 = 1; id1 < this->Size; id1++)
	{
		if (this->Value[id1] < this->Value[id1 - 1])
			return false;
	}
	return true;
}

template <typename T>
bool DataArray<T>::IsDecrease()
{
	for (int id1 = 1; id1 < this->Size; id1++)
	{
		if (this->Value[id1] > this->Value[id1 - 1])
			return false;
	}
	return true;
}

template <typename T>
T DataArray<T>::FindFirstInRange(T minValue, T maxValue, int fromIdx, int toIdx)
{
	for (int idx = fromIdx; idx <= toIdx; idx++)
	{
		if (this->Value[idx] > minValue && this->Value[idx] < maxValue)
			return idx;
	}
	return -1;
}

template <typename T>
T DataArray<T>::FindLastInRange(T minValue, T maxValue, int fromIdx, int toIdx)
{
	for (int idx = toIdx; idx >= fromIdx; idx--)
	{
		if (this->Value[idx] > minValue && this->Value[idx] < maxValue)
			return idx;
	}
	return -1;
}

template <typename T>
T DataArray<T>::FindFirst(CompareTypes type, T value, int fromIdx, int toIdx)
{
	for (int idx = fromIdx; idx <= toIdx; idx++)
	{
		switch (type)
		{
			case CompareTypes::GE:
				if (this->Value[idx] >= value) return idx;
				break;
			case CompareTypes::GT:
				if (this->Value[idx] > value) return idx;
				break;

			case CompareTypes::LE:
				if (this->Value[idx] <= value) return idx;
				break;
			case CompareTypes::LT:
				if (this->Value[idx] < value) return idx;
				break;
		}
	}
	return -1;
}

template <typename T>
T DataArray<T>::FindLast(CompareTypes type, T value, int fromIdx, int toIdx)
{
	for (int idx = toIdx; idx <= fromIdx; idx--)
	{
		switch (type)
		{
			case CompareTypes::GE:
				if (this->Value[idx] >= value) return idx;
				break;
			case CompareTypes::GT:
				if (this->Value[idx] > value) return idx;
				break;

			case CompareTypes::LE:
				if (this->Value[idx] <= value) return idx;
				break;
			case CompareTypes::LT:
				if (this->Value[idx] < value) return idx;
				break;
		}
	}
	return -1;
}

template <typename T>
T DataArray<T>::FindGreatest(T value, int fromIdx, int toIdx)
{
	int maxValIdx = -1;
	int maxVal = value;
	for (int idx = fromIdx; idx <= toIdx; idx++)
	{
		if (maxValIdx >= 0 && this->Value[idx] <= maxVal) continue;
		maxValIdx = idx;
		maxVal = this->Value[maxValIdx];
	}
	return maxValIdx;
}

template <typename T>
T DataArray<T>::FindSmallest(T value, int fromIdx, int toIdx)
{
	int minValIdx = -1;
	int minVal = value;
	for (int idx = fromIdx; idx <= toIdx; idx++)
	{
		if (minValIdx >= 0 && this->Value[idx] >= minVal) continue;
		minValIdx = idx;
		minVal = this->Value[minValIdx];
	}
	return minValIdx;
}

// Extract data to calculate Left-to-right and Right-to-left minima
// Return list of possitions that match the extraction
template <typename T>
DataArray<T> DataArray<T>::ExtractToCalcWidth(bool downward)
{
	DataArray<int> retList;
	if (this->Size == 0) return retList;
	
	DataArray<int> tmpList;
	tmpList.Alloc(this->Size);
	int count = 0;
	int lastValue;
	if (downward)
	{
		lastValue = this->Value[0];
		tmpList.Value[count++] = 0;
		for (int id1 = 1; id1 < this->Size; id1++)
		{
			if (this->Value[id1] <= lastValue) continue;
			tmpList.Value[count++] = id1;
			lastValue = this->Value[id1];
		}
	}
	else
	{
		lastValue = this->Value[this->Size-1];
		tmpList.Value[count++] = this->Size - 1;
		for (int id1 = this->Size-2; id1 >= 0; id1--)
		{
			if (this->Value[id1] >= lastValue) continue;
			tmpList.Value[count++] = id1;
			lastValue = this->Value[id1];
		}
	}
	
	retList.Alloc(count);
	memcpy(retList.Value, tmpList.Value, count * sizeof(T));
	tmpList.DeAlloc();
	return retList;
}