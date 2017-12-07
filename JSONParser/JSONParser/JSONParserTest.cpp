#include "stdafx.h"

#include "JSONParserTest.h"

#include "JSONParser.h"

const int nCount = 8;
const int nRight = 5;

string arrJSON[nCount] = {
	// right ones
	"{}",
	"{\"a\":\"b\"}",
	"{\"a\":[\"b\",\"c\",\"d\"]}",
	"{\"a b\":[], \"c\":{\"d\":\"e f\", \"g\":\"h\"}}",
	"{\"a\":null, \"b\":true, \"c\":{\"d\":1234.6, \"e\":2}}",
	// wrong ones
	"  abc ",
	"{\"a\":\"b\"",
	"{\"a\":[2, 5, 9}"
};

int TestJSONParser()
{
	JSONParser parser;
	for (int i = 0; i < nRight; ++i)
	{
		map<string, void*>* pMapResult = parser.parse(arrJSON[i]);
		if (pMapResult == NULL)
		{
			return 1;
		}
	}
	for (int j = nRight; j < nCount; ++j)
	{
		map<string, void*>* pMapResult = parser.parse(arrJSON[j]);
		if (pMapResult != NULL)
		{
			return 2;
		}
	}
	return 0;
}