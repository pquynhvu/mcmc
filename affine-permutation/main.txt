#include "Header.h"
#include <direct.h>
#include <stdlib.h>
#include <atltime.h>
#include <fstream>

int main()
{
	try
	{
		while (true)
		{
			char opt = AskCharacter("Select  1-Build   2-Check4231All   3-Check4231Item   4-Exit: ", '1', "1234");
			switch (opt)
			{
				case '1': MainBuild(); break;
				case '2': MainTest4231(); break;
				case '3': MainTest4231Item(); break;
				case '4': return 0;
			}
		}
	}
	catch (exception e)
	{
		cout << e.what() << endl;
		WriteError(e.what());
		return 1;
	}
}
