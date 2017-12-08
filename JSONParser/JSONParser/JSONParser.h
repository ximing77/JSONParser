#ifndef JSONPARSER_H
#define JSONPARSER_H

#include "JSONAutoMata.h"
#include <map>

typedef enum JSONParserWraperType
{
	JSONParserWrapperType_None = 0,
	JSONParserWrapperType_Value_PlaceHolder,
	JSONParserWrapperType_Vector,
	JSONParserWrapperType_Map,
	JSONParserWrapperType_StringBuffer,
	JSONParserWrapperType_String,
	JSONParserWrapperType_Bool,
	JSONParserWrapperType_Int,
	JSONParserWrapperType_Double,
	JSONParserWrapperType_Null
}JSONParserWrapperType;

class JSONParserWrapper
{
public:
	JSONParserWrapperType eType;
	void* pData;
public:
	JSONParserWrapper()
	{
		eType = JSONParserWrapperType_None;
		pData = NULL;
	}
	JSONParserWrapper(JSONParserWrapperType enumType, void* pointerData)
	{
		eType = enumType;
		pData = pointerData;
	}
	~JSONParserWrapper()
	{
		pData = NULL; // ���ﲻ�ͷţ���Ϊֻ�Ǹ�����������ʵ�����ݶ���������ʱ����������
	}
};

class JSONParser
{
private:
	JSONAutoMata* objJSONParser;
public:
	JSONParser();
	~JSONParser();
	map<string, void*>* parse(string strJSON); // ������ʹ��
	map<string, JSONParserWrapper>* wrapperParse(string strJSON);
	string dissemble(map<string, JSONParserWrapper> *mpJSONObj);
	void* mapSafeAccess(map<string, JSONParserWrapper>* mpJSON, string strKey);
	void* mapSafeAccess(map<string, JSONParserWrapper>* mpJSON, string strKey, JSONParserWraperType dataType);
	void* mapSafeAccess(map<string, void*>* mpJSON, string strKey); // ������ʹ��
	int mapDelete(map<string, void*>* mpJSON); // ������ʹ��
	int mapDelete(map<string, JSONParserWrapper>* mpJSON);
private:
	int vecDelete(vector<void*>* vecJSON); // ������ʹ��
	int vecDelete(vector<JSONParserWrapper>* vecJSON);
	string vecDissemble(vector<JSONParserWrapper>* vcJSONObj);
};

#endif