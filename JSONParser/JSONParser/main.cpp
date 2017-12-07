// JSONParser.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "JSONParserTest.h"

int main()
{
	int nRet =  TestJSONParser();
	if (nRet != 0)
	{
		cout << "error" << endl;
	}

	char cGet;
	cin >> cGet;

	return 0;
}

