// JSONParser.cpp : �������̨Ӧ�ó������ڵ㡣
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

