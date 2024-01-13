#include <direct.h>
#include <stdlib.h>
#include <atltime.h>
#include <fstream>
#include "Header.h"

using namespace std;

PatternTypes PATTERN_OPT; // Choose one of 321-, 4321-,4231- options
DataArray<int> SAVE_DATA_AT_PERCS;
string BUILD_FILE, PATTERN_HEIGHT_FILE;

bool MainBuild()
{
	PATTERN_OPT = AskPatternType();

	int seedSz = 10, decLoopCount = 1, incLoopCount = 1, fullLoopCount = 1000;
	AskSeedParams(seedSz, decLoopCount, incLoopCount, fullLoopCount);

	bool fSaveInsertData = AskCharacter("Save insert data? (y/N) ", 'n', "yn")=='y';
	SAVE_DATA_AT_PERCS = AskSavePercent();

	int CalcPatternWidthStep = 0;
	bool fSavePatternWidthData = false;
	int LastPatternWidthSaveAt = -1;
	AskCalcPatternWidth(CalcPatternWidthStep, fSavePatternWidthData);
	if (CalcPatternWidthStep > 0) CalcPatternWidthStep = fullLoopCount / CalcPatternWidthStep;

	int logSplitStep = 0;
	if (SPLIT_LOG) logSplitStep = AskNumber("How many log to split (default = 1)? ",1,0);
	if (logSplitStep > 0) logSplitStep = fullLoopCount / logSplitStep;
	
	if (AskCharacter("Confirm to build? (Y/n) ", 'y', "yn")=='n') return false;

	BUILD_FILE = OUTPUT_FOLDER + "//build.txt";

	//Pattern width summary file
	PATTERN_HEIGHT_FILE = OUTPUT_FOLDER + "//pa-width.txt";
	
	_mkdir(OUTPUT_FOLDER.c_str());
	remove(BUILD_FILE.c_str());
	remove(PATTERN_HEIGHT_FILE.c_str());
	SetLogFileStart();

	WriteLog("Seed size: " + std::to_string(seedSz));
	WriteLog("Decrease Loop: " + std::to_string(decLoopCount));
	WriteLog("Increase Loop: " + std::to_string(incLoopCount));
	WriteLog("Full Loop: " + std::to_string(fullLoopCount));

	string tmp = "Avoid mode: " + PatternTypesToStr(PATTERN_OPT);
	WriteLog(tmp);	

	// SEED
	DataArray<int> curSeeds;
	curSeeds.Alloc(seedSz);

	if (ReadData(curSeeds, SEED_FILE) == false)
	{
		curSeeds.DeAlloc();
		SAVE_DATA_AT_PERCS.DeAlloc();
		return 0;
	}
	if (VERBOSE) ShowResult("Seed", curSeeds, seedSz);

	Stats stats;
	stats.FullLoopCount = fullLoopCount;
	stats.StartTime = time(0);
	bool fSaveData;

	int deletePos, insertPos, insertValue;
	BldResult* lastAvoidResult = NULL;
	int saveDataPercIdx = 0;
	for (int loopIdx = 0; loopIdx < fullLoopCount; loopIdx++)
	{
		fSaveData = false;
		if (saveDataPercIdx < SAVE_DATA_AT_PERCS.Size)
		{
			fSaveData =  (loopIdx >= fullLoopCount * SAVE_DATA_AT_PERCS.Value[saveDataPercIdx]/100);
		}
		if (VERBOSE == false)
		{
			ShowString("Loop " + std::to_string(loopIdx));
		}
		deletePos = GetDeletePos(seedSz);
		insertPos = GetInsertPos(seedSz-1);
		insertValue = GetInsertValue(seedSz-1, insertPos);
		
		BldResult* result = BuildData(stats, curSeeds, deletePos, insertPos, insertValue, decLoopCount, incLoopCount);
		if (result->IsBuildOk == false)
		{
			ShowString("Build error " + std::to_string(loopIdx));
			result->DeAlloc();
			break;
		}
		
		if (fSaveInsertData)
		{
			// Build result list
			tmp = std::to_string(loopIdx) + "\t";
			if (result->IsGoodSeeds)
			{
				if (result->IsAvoid) tmp += "AVOID"; else tmp += "NOT-AVOID";
			}
			else tmp += "BAD";
			WriteLog(tmp, BUILD_FILE);

			// Insert data for checking
			DataArray<int> arr1[] = { result->Insert };
			SaveDataINT(1, arr1, OUTPUT_FOLDER + "\\insert-" + std::to_string(loopIdx) + ".txt");
		}

		if (result->IsAvoid)
		{
			result->NewSeeds.CopyTo(curSeeds, result->NewSeeds.Size);
			if (lastAvoidResult != NULL) lastAvoidResult->DeAlloc();
			lastAvoidResult = result;
		}
		else
		{
			result->DeAlloc();
		}
		if (lastAvoidResult != NULL && LastPatternWidthSaveAt < lastAvoidResult->AtLoop &&
			CalcPatternWidthStep > 0 && (loopIdx % CalcPatternWidthStep) == 0)
		{
			LastPatternWidthSaveAt = loopIdx;
			CalcPatternWidth(lastAvoidResult->AtLoop, lastAvoidResult->NewSeeds, 1, PATTERN_HEIGHT_FILE, fSavePatternWidthData);
		}

		if (fSaveData && lastAvoidResult != NULL)
		{
			SaveBuild(*lastAvoidResult, loopIdx);
			lastAvoidResult->DeAlloc();
			lastAvoidResult = NULL;

			saveDataPercIdx++;
			
			WriteStats(stats);
		}
		if (logSplitStep > 0 && loopIdx > 0 && (loopIdx % logSplitStep) == 0)
		{
			WriteStats(stats);
			SetLogFileNew();
		}
		stats.Idx++;
	}
	if (lastAvoidResult != NULL)
	{
		if (lastAvoidResult->IsAvoid && CalcPatternWidthStep > 0 && LastPatternWidthSaveAt < fullLoopCount-1)
		{
			CalcPatternWidth(lastAvoidResult->AtLoop, lastAvoidResult->NewSeeds, 1, PATTERN_HEIGHT_FILE, fSavePatternWidthData);
		}
		SaveBuild(*lastAvoidResult, fullLoopCount);
		lastAvoidResult->DeAlloc();
		lastAvoidResult = NULL;
	}
	
	WriteStats(stats);
	if (VERBOSE) ShowStats(stats);

	curSeeds.DeAlloc();
	SAVE_DATA_AT_PERCS.DeAlloc();
	return true;
}

void SaveBuild(BldResult data, int id)
{
	DataArray<int> arr1[] = { data.xRange, data.Insert };
	SaveDataINT(2, arr1, OUTPUT_FOLDER + "\\data-" + std::to_string(id) + ".txt");
}

DataArray<int> AskSavePercent()
{
	DataArray<int> dataPerc;
	int count = AskNumber("How many AVOID data to save (default = 3)? ",3,0);
	if (count > 1)
	{
		dataPerc.Alloc(count-1);
		int step = 100 / count;
		for (int idx = 0; idx < count-1; idx++)
		{
			dataPerc.Value[idx] = (idx+1) * step;
		}
	}
	return dataPerc;
}
PatternTypes AskPatternType()
{
	char opt;
	while (true)
	{
		opt = AskCharacter("Select pattern  1-Avoid321   2-Avoid4321   3-Avoid231  4-Avoid 4231   5-Avoid4231 test: ", ' ', "12345");
		switch (opt)
		{
			case '1':	return PatternTypes::P321;
			case '2':	return PatternTypes::P4321;
			case '3':	return PatternTypes::P231;
			case '4':	return PatternTypes::P4231;
			case '5':	return PatternTypes::P4231Test;
			default:continue;
		}
		break;
	}
}
void AskCalcPatternWidth(int& patternWidth, bool& saveData)
{
	patternWidth = AskNumber("How many pattern width to check (default = 0)? ",0);
	saveData = AskCharacter("Save pattern width data? (y/N)", 'n', "yn")=='y';
}

BldResult* BuildData(Stats& stat, DataArray<int> seeds, int deletePos, int insertPos, int insertValue, int decLoopCount, int incLoopCount)
{
	BldResult* result = new BldResult();
	result->IsGoodSeeds = false;
	result->IsAvoid = false;
	result->IsBuildOk = false;
	result->AtLoop = stat.Idx;

	string tmp;
	bool b;
	WriteLog("\nLoop  " + std::to_string(stat.Idx));
	WriteLog("-Seeds: " + ToString(seeds));
	// Clone data
	if (Clone(seeds, decLoopCount, incLoopCount, result->Clone)==false) 	return result;
	
	if (VERBOSE) ShowResult("X" + std::to_string(stat.Idx), result->Clone, seeds.Size);
	
	tmp = "-Delete at " + std::to_string(deletePos);
	WriteLog(tmp);
	if (VERBOSE) ShowString(tmp);
	int loopCount = decLoopCount + incLoopCount;
	b = CreateByDelete(deletePos, result->Clone, seeds.Size, decLoopCount, incLoopCount, result->Delete);
	if (b == false) return result;
	
	// Create new seeds by inserting a value
	int seedsFromDeleteSz = seeds.Size - 1;
	DataArray<int> seedsFromDelete = CreateSeedsFromData(result->Delete, decLoopCount * seedsFromDeleteSz, seedsFromDeleteSz);
	if (seedsFromDelete.Size == 0)
	{
		cout << "Error on calling CreateSeedsFromData()" << endl;
		seedsFromDelete.DeAlloc();
		result->IsBuildOk = false;
		return result;
	}	

	tmp = "-Insert value " + std::to_string(insertValue) + " at " + std::to_string(insertPos);
	WriteLog(tmp);
	if (VERBOSE) ShowString(tmp);
	
	b = CreateSeedsByInsert(insertPos, insertValue, seedsFromDelete,result->NewSeeds);
	if (b==false)
	{
		cout << "Error on calling CreateSeedsByInsert()" << endl;
		seedsFromDelete.DeAlloc();
		return result;
	}
	// Clone data
	if (Clone(result->NewSeeds, decLoopCount, incLoopCount, result->Insert) ==false)
	{
		seedsFromDelete.DeAlloc();
		return result;
	}
	seedsFromDelete.DeAlloc();

	if (IsGoodSeeds(result->NewSeeds))
	{
		DataArray<int> Avoid321P1, Avoid321P2;
		DataArray<int> Avoid4321P1, Avoid4321P2;
		int newInsPos;
		result->IsGoodSeeds = true;
		stat.GoodSeed++;
		switch (PATTERN_OPT)
		{
			case PatternTypes::P321:
				PatternSplit(result->Insert, Avoid321P1, Avoid321P2);
				if (Avoid321P2.IsIncrease())
				{
					stat.PatternAvoid++;
					result->IsAvoid = true;
				}
				Avoid321P1.DeAlloc();	Avoid321P2.DeAlloc();
				break;
			case PatternTypes::P4321:
				
				PatternSplit(result->Insert, Avoid321P1, Avoid321P2);
				PatternSplit(Avoid321P2, Avoid4321P1, Avoid4321P2);
				if (Avoid4321P2.IsIncrease())
				{
					stat.PatternAvoid++;
					result->IsAvoid = true;
				}
				Avoid321P1.DeAlloc();	Avoid321P2.DeAlloc();
				Avoid4321P1.DeAlloc();	Avoid4321P2.DeAlloc();
				break;
			case PatternTypes::P231:
				if (IsAvoid231_NealMadras(result->Insert))
				{
					stat.PatternAvoid++;
					result->IsAvoid = true;
				}
				break;

			case PatternTypes::P4231:
				newInsPos = decLoopCount*seeds.Size + insertPos;
				result->IsAvoid = IsAvoid4231_NealMadras(result->Insert, newInsPos);
				if (result->IsAvoid) stat.PatternAvoid++;
				break;

			case PatternTypes::P4231Test:
				result->IsAvoid = IsAvoid4231_Nomal(result->Insert);
				if (result->IsAvoid) stat.PatternAvoid++;
				break;
		}
		tmp = "-GOOD Seeds : " + ToString(result->NewSeeds) + "\t";
		if (result->IsAvoid) tmp += "AVOID"; else tmp += "NOT-AVOID";
		WriteLog(tmp);		
		if (VERBOSE) ShowString(tmp);

		GenXRange(-decLoopCount * seeds.Size, (incLoopCount + 1) * seeds.Size, result->xRange);
	}
	else
	{
		tmp = "-BAD Seeds : " + ToString(result->NewSeeds);
		WriteLog(tmp);
		if (VERBOSE) ShowString(tmp);
	}

	if (VERBOSE) ShowString("");
	
	result->IsBuildOk = true;
	return result;
}
