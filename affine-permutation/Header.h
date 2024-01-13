#ifndef Header_h_
#define Header_h_
#include<iostream>
#include<string>
#include<array>
using namespace std;

const string PATTERN_WIDTH_DATA_FILE_PREFIX = "pa-width-";
const string OUTPUT_FOLDER = "OUTPUT";
const string SEED_FILE = "seeds.txt";
const bool VERBOSE = false;
const bool SPLIT_LOG = true;

const string LOG_FILE_NAME = "log";
const string LOG_FILE_EXT = ".txt";
static int LOG_FILE_COUNT = 0;

enum class PatternTypes
{
	P321,
	P4321,
	P231,
	P4231,
	P4231Test,
};

enum class CompareTypes
{
	LT, // Less than
	GT, // Greater than
	LE, // Less or equal
	GE, // Greater or equal
};

template <typename T>

class DataArray
{
	private:
		int _sz;
		int _currentIdx = -1;
		T* _ptr = NULL;

	public:
		__declspec(property(get = property_get_currIdx)) int CurrentIdx;
		typedef int property_Value;
		property_Value property_get_currIdx()
		{
			return _currentIdx;
		}

		__declspec(property(get = property_get_value)) T ValueAtTop;
		typedef T property_ValueT;
		property_ValueT property_get_value()
		{
			if (_currentIdx < 0)  return NULL;
			return _ptr[_currentIdx];
		}

		__declspec(property(get = property_get_ptr)) T* Value;
		typedef T* property_type_ptr;
		property_type_ptr property_get_ptr()
		{
			return _ptr;
		}

		__declspec(property(get = property_get_size)) int Size;
		typedef int property_type_size;
		property_type_size property_get_size()
		{
			return _sz;
		}
	
		// Extract data
		DataArray<T> ExtractDownward(CompareTypes type, T value, int fromIdx, int toIdx);
		DataArray<T> ExtractUpward(CompareTypes type, T value, int fromIdx, int toIdx);

		// Transform: rotate data 180 degree
		void Transform180();
		void Negative();

		bool CopyTo(DataArray<T> data, int sz);
		
		void Alloc(int sz);
		void DeAlloc();
		T Pop();
		bool Push(T value);
		bool IsIncrease();
		bool IsDecrease();

		// Return the position of the first value matched or -1 if not found
		T FindFirst(CompareTypes type, T value, int fromIdx, int toIdx);

		// Return the position of the last value matched or -1 if not found
		T FindLast(CompareTypes type, T value, int fromIdx, int toIdx);

		// Return the position of the first value in range or -1 if not found
		T FindFirstInRange(T minValue, T maxValue, int fromIdx, int toIdx);

		// Return the position of the first value in range or -1 if not found
		T FindLastInRange(T minValue, T maxValue, int fromIdx, int toIdx);

		// Return the position of the greatest or -1 if not found
		T FindGreatest(T value, int fromIdx, int toIdx);

		// Return the position of the smallest  or -1 if not found
		T FindSmallest(T value, int fromIdx, int toIdx);

		// Extract data to calculate Left-to-right and Right-to-left minima
		// Return list of possitions that match the extraction
		DataArray<T> ExtractToCalcWidth(bool downward);
};

class Stats
{
	public:
		int Idx = 0;
		int FullLoopCount = 0;
		int GoodSeed = 0;
		int PatternAvoid = 0;
		time_t StartTime;
		time_t EndTime;
};

class BldResult
{
	public:
		DataArray<int> xRange;
		DataArray<int> Clone;
		DataArray<int> Insert;
		DataArray<int> Delete;
		DataArray<int> NewSeeds;
		int AtLoop = 0;
		bool IsGoodSeeds = false;
		bool IsAvoid = false;
		bool IsBuildOk = false;
		void DeAlloc();
};

bool MainBuild();

void MainTest4231();
void MainTest4231Item();

string PatternTypesToStr(PatternTypes pattern);
DataArray<int> AskSavePercent();
PatternTypes AskPatternType();
void AskCalcPatternWidth(int& patternWidth, bool& saveData);

bool ReadData(DataArray<int> data, string fileName);
int Random(int min, int max);
bool IsGoodSeeds(DataArray<int> data);

int GetDeletePos(int rangeSz);
int GetInsertPos(int rangeSz);
int GetInsertValue(int rangeSz, int insertPos);

bool WriteStats(Stats stats);
void ShowStats(Stats stats);
string TimeToStrc(time_t t);
void GenXRange(int min, int max, DataArray<int>& data);
bool Clone(DataArray<int> seeds, int decLoopCount, int incLoopCount, DataArray<int>& toData);
bool CreateByDelete(int atSeedPos, DataArray<int> arr, int seedSz, int decLoopCount, int incLoopCount, DataArray<int>&);
DataArray<int> CreateSeedsFromData(DataArray<int> data, int atIdx, int sz);
bool CreateSeedsByInsert(int atPos, int value, DataArray<int> seeds, DataArray<int>&);
DataArray<int> Create231AvoidCheck(DataArray<int> seeds);

void AskSeedParams(int& seedSz, int& decLoopCount, int& incLoopCount, int& fullLoopCount);
void ShowResult(string title, DataArray<int> data, int sz);
void ShowString(string title);

bool SaveDataINT(int arrSz, DataArray<int> data[], string fileName);
template <typename T>
bool SaveData(int arrSz, DataArray<T> data[], string fileName);

void SetLogFileStart();
void SetLogFileNew();
void SetLogFileCount(int count);
string GetLogFile();
bool WriteLog(string line, string fileName="");
void WriteError(string line);
bool CalcPatternWidth(int atLoop, DataArray<int> data, int xShift, string fileName, bool saveData);

int AskNumber(string text, int defaultValue, int min=0, int max = INT_MAX);
char AskCharacter(string text, char defaultChar, string allowChars);

BldResult* BuildData(Stats& stat, DataArray<int> seeds, int deletePos, int insertPos, int insertValue, int decLoopCount, int incLoopCount);
void SaveBuild(BldResult data, int id);

void PatternSplit(DataArray<int> data, DataArray<int>& toList1, DataArray<int>& toList2);
bool IsAvoid231_NealMadras(DataArray<int> data);
bool IsAvoid4231_NealMadras(DataArray<int> data, int insertPos);
bool IsAvoid4231_Nomal(DataArray<int> data);

string ToString(DataArray<int> data, string separator=":");

#endif
