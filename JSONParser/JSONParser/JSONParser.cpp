#include "stdafx.h"

#include "JSONParser.h"

#include <sstream>


JSONParser::JSONParser()
{
	objJSONParser = new JSONAutoMata();
}

JSONParser::~JSONParser()
{
	if(objJSONParser)
	{
		delete objJSONParser;
	}
}

map<string, void*>* JSONParser::parse(string strJSON)
{
	JSONParserWrapper refWrapper;
	JSONParserWrapper refNewWrapper;
	JSONParserWrapper refNewSecondWrapper;

	map<string, void*>* tmpMap = NULL;
	void* pNewBack = NULL;
	stringstream* ssBuffer = NULL;
	void* pCurrentBack = NULL;
	vector<void*>* pVecNewBack = NULL;
	void* pNewSecondBack = NULL;
	char* pcPlaceHolder = NULL;
	void* pBack = NULL;
	vector<void*>* pVecArr = NULL;

	if(objJSONParser)
	{
		objJSONParser->clear();
		vector<JSONParserWrapper> vecConstructor = vector<JSONParserWrapper>();
		// ѭ������ʱ��¼����
		JSONAutoMataState ePriorState = JSONAutoMataState_None;
		double fAugments = 0.0;
		// ��������JSON�ַ���
		JSONAutoMataState eState = JSONAutoMataState_None;
		char cCurrent = '\0';
		for(string::iterator strItr = strJSON.begin(); strItr != strJSON.end(); ++ strItr)
		{
			cCurrent = (*strItr);
			eState = objJSONParser->feed(cCurrent);
			// ���ݷ���״̬������
			switch(eState)
			{
			case JSONAutoMataState_Space:
				if(vecConstructor.size() == 0)
				{
					break;
				}
				refWrapper = vecConstructor.back();
				if(refWrapper.eType == JSONParserWrapperType_StringBuffer) // ��д�ַ������ո�ԭ������
				{
					ssBuffer = (stringstream*)refWrapper.pData;
					(*ssBuffer) << cCurrent;
				}
				break;
			case JSONAutoMataState_Dic_Begin:
				ePriorState = JSONAutoMataState_Dic_Begin;
				tmpMap = new map<string, void*>();
				vecConstructor.push_back(JSONParserWrapper(JSONParserWrapperType_Map_Void, tmpMap));
				break;
			case JSONAutoMataState_Dic_End:
				if(vecConstructor.size() == 0)
				{
					return NULL;
				}
				ePriorState = JSONAutoMataState_Dic_End;
				refWrapper = vecConstructor.back();
				// ����δ����Ļ������ͣ�Ӧ�����ֵ�����һ��value
				if(refWrapper.eType == JSONParserWrapperType_Bool ||
					refWrapper.eType == JSONParserWrapperType_Int ||
					refWrapper.eType == JSONParserWrapperType_Double ||
					refWrapper.eType == JSONParserWrapperType_Null) 
				{
					vecConstructor.pop_back(); // ������ǰֵ
					vecConstructor.pop_back(); // ����ռλ��

					vecConstructor.push_back(refWrapper); // ����ǰֵ��Ϊvalue�����ȥ
				}

				// �����ֵ����һ��key-value��δ��������
				if(vecConstructor.size() > 2) // ��������
				{
					refWrapper = vecConstructor.back();
					refNewWrapper = vecConstructor.at(vecConstructor.size() - 2);
					refNewSecondWrapper = vecConstructor.at(vecConstructor.size() - 3);
					// ��һ���ȴ�����ļ�ֵ��
					if(refNewWrapper.eType == JSONParserWrapperType_String && 
						refNewSecondWrapper.eType == JSONParserWrapperType_Map_Void)
					{
						map<string, void*>* pMap = (map<string, void*>*)refNewSecondWrapper.pData;
						string strKey = string((*((string*)refNewWrapper.pData)));
						delete ((string*)refNewWrapper.pData);

						pMap->insert(pair<string, void*>(strKey, refWrapper.pData));

						vecConstructor.pop_back();
						vecConstructor.pop_back();

						refWrapper = vecConstructor.back();
					}
				}

				if(vecConstructor.size() == 1) // �������ֵ�
				{
					refWrapper = vecConstructor.back();
					tmpMap = (map<string, void*>*)refWrapper.pData;
					return tmpMap;
				}
				else if(vecConstructor.size() > 1)
				{
					refNewWrapper = vecConstructor.at(vecConstructor.size() - 2);
					if(refNewWrapper.eType == JSONParserWrapperType_Value_PlaceHolder) // ��Ҫ�滻value
					{
						refWrapper = vecConstructor.back();

						vecConstructor.pop_back();
						vecConstructor.pop_back();

						vecConstructor.push_back(refWrapper);
					}
				}
				break;
			case JSONAutoMataState_Dic_Key_Begin:
				if(vecConstructor.size() == 0)
				{
					return NULL;
				}
				ePriorState = JSONAutoMataState_Dic_Key_Begin;
				ssBuffer = new stringstream();
				vecConstructor.push_back(JSONParserWrapper(JSONParserWrapperType_StringBuffer, ssBuffer));
				break;
			case JSONAutoMataState_Dic_Key_End:
				if(vecConstructor.size() == 0)
				{
					return NULL;
				}
				ePriorState = JSONAutoMataState_Dic_Key_End;
				refWrapper = vecConstructor.back();
				if(refWrapper.eType == JSONParserWrapperType_StringBuffer)
				{
					stringstream* pssBuffer = (stringstream*)refWrapper.pData;
					string* pStrKey = new string(pssBuffer->str());
					// ɾ������Ҫ��stringstream
					vecConstructor.pop_back();
					delete pssBuffer;

					vecConstructor.push_back(JSONParserWrapper(JSONParserWrapperType_String, pStrKey)); // ������ɵ�key�����ȥ
				}
				else // ��������Ӧ�ñ���
				{
					return NULL;
				}
				break;
			case JSONAutoMataState_Dic_Value_Begin:
				if(vecConstructor.size() == 0)
				{
					return NULL;
				}
				ePriorState = JSONAutoMataState_Dic_Value_Begin;
				// value�����ݲ�����������ĳ�����ͣ�Ҳ������ĳ���ṹ������һ���ַ���ռλ
				vecConstructor.push_back(JSONParserWrapper(JSONParserWrapperType_Value_PlaceHolder, NULL));
				break;
			case JSONAutoMataState_Dic_Value_End:
				if(vecConstructor.size() == 0)
				{
					return NULL;
				}
				ePriorState = JSONAutoMataState_Dic_Value_End;
				// �ߵ�����ʱӦ���Ѿ������������滻����
				refWrapper = vecConstructor.back();
				vecConstructor.pop_back();
				// ����value�Ǹ����������
				fAugments = 0.0;

				// �����ǻ������͵��������������»�û�н�ֵ�滻��valueλ��
				if(refWrapper.eType == JSONParserWrapperType_Bool || 
					refWrapper.eType == JSONParserWrapperType_Int ||
					refWrapper.eType == JSONParserWrapperType_Double ||
					refWrapper.eType == JSONParserWrapperType_Null)
				{
					vecConstructor.pop_back();
				}

				refNewWrapper = vecConstructor.back(); // ȡ��key
				 // ȡ��map
				if(vecConstructor.size() > 1)
				{
					refNewSecondWrapper = vecConstructor.at(vecConstructor.size() - 2);
				}
				if(vecConstructor.size() > 1 && 
					refNewWrapper.eType == JSONParserWrapperType_String && 
					refNewSecondWrapper.eType == JSONParserWrapperType_Map_Void) // ��һ�����ֵ䣬���ж�Ӧkey�ڵȴ���
				{
					string strKey = string(*((string*)refNewWrapper.pData)); // ��keyתΪ����
					// ԭ�������key������������
					vecConstructor.pop_back();
					delete ((string*)refNewWrapper.pData);
					// ���뵽�ȴ���map��
					map<string, void*>* pMapNewBack = (map<string, void*>*)refNewSecondWrapper.pData;
					pMapNewBack->insert(pair<string, void*>(strKey, refWrapper.pData));
				}
				else // ���ڴ���
				{
					return NULL;
				}
				break;
			case JSONAutoMataState_String_Begin:
				if(vecConstructor.size() == 0)
				{
					return NULL;
				}
				ePriorState = JSONAutoMataState_String_Begin;
				// �ַ�����ʼ��ֻ��value��element�������������keyֱ����keybegin������
				ssBuffer = new stringstream();
				vecConstructor.push_back(JSONParserWrapper(JSONParserWrapperType_StringBuffer, ssBuffer));
				break;
			case JSONAutoMataState_String_End:
				if(vecConstructor.size() == 0)
				{
					return NULL;
				}
				ePriorState = JSONAutoMataState_String_End;
				refWrapper = vecConstructor.back();
				if(refWrapper.eType == JSONParserWrapperType_StringBuffer)
				{
					stringstream* pSSBuffer = (stringstream*)refWrapper.pData;
					string strResult = pSSBuffer->str();
					vecConstructor.pop_back();
					delete pSSBuffer;

					refWrapper = vecConstructor.back();
					// string��value��ֵ
					if(refWrapper.eType == JSONParserWrapperType_Value_PlaceHolder)
					{
						string* strValue = new string(strResult);
						refWrapper.eType = JSONParserWrapperType_String;
						refWrapper.pData = strValue;

						vecConstructor.pop_back();
						vecConstructor.push_back(refWrapper);
					}
					else if(refWrapper.eType == JSONParserWrapperType_Vector_Void) // string�������Ԫ�أ��ٷŻ�ȥ
					{
						string* strValue = new string(strResult);
						vecConstructor.push_back(JSONParserWrapper(JSONParserWrapperType_String, strValue));
					}
				}
				else // ������
				{
					return NULL;
				}
				break;
			case JSONAutoMataState_Arr_Begin:
				if(vecConstructor.size() == 0)
				{
					return NULL;
				}
				ePriorState = JSONAutoMataState_Arr_Begin;
				pVecArr = new vector<void*>();
				vecConstructor.push_back(JSONParserWrapper(JSONParserWrapperType_Vector_Void, pVecArr));
				break;
			case JSONAutoMataState_Arr_Elem_Seg:
				if(vecConstructor.size() == 0)
				{
					return NULL;
				}
				ePriorState = JSONAutoMataState_Arr_Elem_Seg;
				// һ��Ԫ�ؽ�����
				refWrapper = vecConstructor.back();
				vecConstructor.pop_back();
				// Ԫ���Ǹ����������
				fAugments = 0.0;

				refNewWrapper = vecConstructor.back();
				if(refNewWrapper.eType == JSONParserWrapperType_Vector_Void)
				{
					pVecArr = (vector<void*>*)refNewWrapper.pData;
					pVecArr->push_back(refWrapper.pData);
				}
				else // ������
				{
					return NULL;
				}
				break;
			case JSONAutoMataState_Arr_End:
				if(vecConstructor.size() == 0)
				{
					return NULL;
				}

				if(ePriorState == JSONAutoMataState_Arr_Begin) // ����������
				{
					ePriorState = JSONAutoMataState_Arr_End;

					refWrapper = vecConstructor.back();
					if(refWrapper.eType == JSONParserWrapperType_Vector_Void)
					{
						// ���value�����
						if(vecConstructor.size() > 1)
						{
							refNewWrapper = vecConstructor.at(vecConstructor.size() - 2);
							if(refNewWrapper.eType == JSONParserWrapperType_Value_PlaceHolder) // ������value������
							{
								vecConstructor.pop_back();
								vecConstructor.pop_back();

								vecConstructor.push_back(refWrapper);
							}
						}
					}
					else // ��Ҫ�����Ĳ�������
					{
						return NULL;
					}
				}
				else // �ǿ��������
				{
					ePriorState = JSONAutoMataState_Arr_End;
					// ��ʱǰ�滹��һ��Ԫ�صȴ����б�
					refWrapper = vecConstructor.back();
					vecConstructor.pop_back();
					// ����Ԫ���Ǹ����������
					fAugments = 0.0;

					refNewWrapper = vecConstructor.back();
					if(refNewWrapper.eType == JSONParserWrapperType_Vector_Void)
					{
						// �����һ��Ԫ�ط����б�
						pVecArr = (vector<void*>*)refNewWrapper.pData;
						pVecArr->push_back(refWrapper.pData);

						// ���value�����
						if(vecConstructor.size() > 1)
						{
							refWrapper = vecConstructor.back();
							refNewWrapper = vecConstructor.at(vecConstructor.size() - 2);
							if(refNewWrapper.eType == JSONParserWrapperType_Value_PlaceHolder) // ������value������
							{
								vecConstructor.pop_back();
								vecConstructor.pop_back();

								vecConstructor.push_back(refWrapper);
							}
						}
					}
					else // ��Ҫ�����Ĳ�������
					{
						return NULL;
					}
				}
				
				break;
			case JSONAutoMataState_None:
				if(vecConstructor.size() == 0)
				{
					break;
				}
				// �����ַ�������
				refWrapper = vecConstructor.back();
				if(refWrapper.eType == JSONParserWrapperType_StringBuffer) // �����ڴ����е��ַ���
				{
					stringstream* pSSBuffer = (stringstream*)refWrapper.pData;
					(*pSSBuffer) << cCurrent;
				}
				else if(ePriorState == JSONAutoMataState_Dic_Value_Begin || 
					ePriorState == JSONAutoMataState_Arr_Elem_Seg || 
					ePriorState == JSONAutoMataState_Arr_Begin) // ���ڴ����������������
				{
					if(refWrapper.eType == JSONParserWrapperType_Bool) // �����͵ķǵ�һ���ַ�
					{
						//ʲô������
					}
					else if(refWrapper.eType == JSONParserWrapperType_Null) // �����е�null
					{
						// ʲô������
					}
					else if(refWrapper.eType == JSONParserWrapperType_Int)
					{
						int* pInt = (int*)refWrapper.pData;
						if(fAugments > 0.0) // �Ǹ���������Ҫ����
						{
							double* pDouble = (double*)malloc(sizeof(double));
							(*pDouble) = ((double)(*pInt)) + fAugments * (cCurrent - '0'); // ��ӵ���С��λ
							fAugments *= 0.1; // ��һ��
							// ɾ��ԭ��������
							delete pInt;
							// ����µĸ�����
							refWrapper.eType = JSONParserWrapperType_Double;
							refWrapper.pData = pDouble;

							vecConstructor.pop_back();
							vecConstructor.push_back(refWrapper);
						}
						else
						{
							if(cCurrent == '.') // ����С����
							{
								fAugments = 0.1;
							}
							else // ��ͨ��������
							{
								int nNewValue = cCurrent - '0';
								(*pInt) = (*pInt) * 10 + nNewValue;
							}
						}
					}
					else if(refWrapper.eType == JSONParserWrapperType_Double)
					{
						double* pDouble = (double*)refWrapper.pData;
						(*pDouble) = (*pDouble) + fAugments * (cCurrent - '0');
						fAugments *= 0.1;
					}
					else // ��һ���ַ�
					{
						if(cCurrent == 't') // ����true
						{
							bool bBool = true;
							vecConstructor.push_back(JSONParserWrapper(JSONParserWrapperType_Bool, (void*)bBool));
						}
						else if(cCurrent == 'f') // ����false
						{
							bool bBool = false;
							vecConstructor.push_back(JSONParserWrapper(JSONParserWrapperType_Bool, (void*)bBool));
						}
						else if(cCurrent == 'n' || cCurrent == 'N') // null
						{
							//string* strNULL = new string("NULL");
							vecConstructor.push_back(JSONParserWrapper(JSONParserWrapperType_Null, NULL));
						}
						else if(cCurrent >= '0' && cCurrent <= '9') // ����
						{
							// ���ڲ����ж������ͻ��Ǹ����ͣ��Ȱ����͵���������������������С���㣬�ٸĳɸ�����
							int* pInt = (int*)malloc(sizeof(int));
							(*pInt) = (int)(cCurrent - '0');
							vecConstructor.push_back(JSONParserWrapper(JSONParserWrapperType_Int, pInt));
						}
					}
				}
				break;
			case JSONAutoMataState_Escape:
				if(vecConstructor.size() == 0)
				{
					break;
				}
				refWrapper = vecConstructor.back();
				if(refWrapper.eType == JSONParserWrapperType_StringBuffer) // �����ڴ����е��ַ���
				{
					stringstream* pSSBuffer = (stringstream*)refWrapper.pData;
					(*pSSBuffer) << cCurrent;
				}
				break;
			}
		}

		ePriorState = objJSONParser->finish();
	}

	return NULL;
}

string JSONParser::dissemble(map<string, JSONParserWrapper> *mpJSONObj)
{
	string strResult = "";
	stringstream ssBuffer;

	for(map<string, JSONParserWrapper>::iterator mpItr = mpJSONObj->begin(); mpItr != mpJSONObj->end(); ++ mpItr)
	{
		string strKey = mpItr->first;
		JSONParserWrapper objWrapper = mpItr->second;
		string strValue = "";
		switch(objWrapper.eType)
		{
		case JSONParserWrapperType_String:
			ssBuffer = stringstream();
			ssBuffer << "\"" << *(string*)objWrapper.pData << "\"";
			strValue = ssBuffer.str();
			break;
		case JSONParserWrapperType_Int:
			ssBuffer = stringstream();
			ssBuffer << *(int*)objWrapper.pData;
			strValue = ssBuffer.str();
			break;
		case JSONParserWrapperType_Double:
			ssBuffer = stringstream();
			ssBuffer << *(double*)objWrapper.pData;
			strValue = ssBuffer.str();
			break;
		}
	}

	return strResult;
}

string JSONParser::vecDissemble(vector<JSONParserWrapper>* vcJSONObj)
{
	string strResult = "";

	return strResult;
}

void* JSONParser::mapSafeAccess(map<string, void*>* mpJSON, string strKey)
{
	void* result = NULL;
	if(mpJSON && mpJSON->size() > 0)
	{
		map<string, void*>::iterator mapItr = mpJSON->find(strKey);
		if(mapItr != mpJSON->end())
		{
			result = mapItr->second;
		}
	}
	return result;
}

int JSONParser::mapDelete(map<string, void*>* mpJSON)
{
	for(map<string, void*>::iterator mapItr = mpJSON->begin(); mapItr != mpJSON->end(); ++mapItr)
	{
		void* pValue = mapItr->second;
		if(typeid(pValue) == typeid(string))
		{
			string* pStrValue = (string*)pValue;
			delete pStrValue;
		}
		else if(typeid(pValue) == typeid(map<string, void*>))
		{
			map<string, void*>* pMapValue = (map<string, void*>*)pValue;
			mapDelete(pMapValue);
		}
		else if(typeid(pValue) == typeid(vector<void*>))
		{
			vector<void*>* pVecValue = (vector<void*>*)pValue;
			vecDelete(pVecValue);
		}
	}

	delete mpJSON;

	return 0;
}

// private

int JSONParser::vecDelete(vector<void*>* vecJSON)
{
	for(vector<void*>::iterator vecItr = vecJSON->begin(); vecItr != vecJSON->end(); ++ vecItr)
	{
		void* pValue = *vecItr;
		if(typeid(pValue) == typeid(string))
		{
			string* pStrValue = (string*)pValue;
			delete pStrValue;
		}
		else if(typeid(pValue) == typeid(map<string, void*>))
		{
			map<string, void*>* pMapValue = (map<string, void*>*)pValue;
			mapDelete(pMapValue);
		}
		else if(typeid(pValue) == typeid(vector<void*>))
		{
			vector<void*>* pVecValue = (vector<void*>*)pValue;
			vecDelete(pVecValue);
		}
	}

	delete vecJSON;

	return 0;
}