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
		// 循环用临时记录变量
		JSONAutoMataState ePriorState = JSONAutoMataState_None;
		double fAugments = 0.0;
		// 遍历整个JSON字符串
		JSONAutoMataState eState = JSONAutoMataState_None;
		char cCurrent = '\0';
		for(string::iterator strItr = strJSON.begin(); strItr != strJSON.end(); ++ strItr)
		{
			cCurrent = (*strItr);
			eState = objJSONParser->feed(cCurrent);
			// 根据返回状态来处理
			switch(eState)
			{
			case JSONAutoMataState_Space:
				if(vecConstructor.size() == 0)
				{
					break;
				}
				refWrapper = vecConstructor.back();
				if(refWrapper.eType == JSONParserWrapperType_StringBuffer) // 在写字符串，空格原样保留
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
				// 存在未处理的基本类型，应该是字典的最后一个value
				if(refWrapper.eType == JSONParserWrapperType_Bool ||
					refWrapper.eType == JSONParserWrapperType_Int ||
					refWrapper.eType == JSONParserWrapperType_Double ||
					refWrapper.eType == JSONParserWrapperType_Null) 
				{
					vecConstructor.pop_back(); // 弹出当前值
					vecConstructor.pop_back(); // 弹出占位符

					vecConstructor.push_back(refWrapper); // 将当前值作为value保存进去
				}

				// 处理字典最后一对key-value尚未加入的情况
				if(vecConstructor.size() > 2) // 数量满足
				{
					refWrapper = vecConstructor.back();
					refNewWrapper = vecConstructor.at(vecConstructor.size() - 2);
					refNewSecondWrapper = vecConstructor.at(vecConstructor.size() - 3);
					// 是一个等待处理的键值对
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

				if(vecConstructor.size() == 1) // “根”字典
				{
					refWrapper = vecConstructor.back();
					tmpMap = (map<string, void*>*)refWrapper.pData;
					return tmpMap;
				}
				else if(vecConstructor.size() > 1)
				{
					refNewWrapper = vecConstructor.at(vecConstructor.size() - 2);
					if(refNewWrapper.eType == JSONParserWrapperType_Value_PlaceHolder) // 需要替换value
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
					// 删掉不需要的stringstream
					vecConstructor.pop_back();
					delete pssBuffer;

					vecConstructor.push_back(JSONParserWrapper(JSONParserWrapperType_String, pStrKey)); // 将新完成的key加入进去
				}
				else // 不正常，应该报错
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
				// value的内容不定，可能是某个类型，也可能是某个结构，先用一个字符来占位
				vecConstructor.push_back(JSONParserWrapper(JSONParserWrapperType_Value_PlaceHolder, NULL));
				break;
			case JSONAutoMataState_Dic_Value_End:
				if(vecConstructor.size() == 0)
				{
					return NULL;
				}
				ePriorState = JSONAutoMataState_Dic_Value_End;
				// 走到这里时应该已经将具体数据替换好了
				refWrapper = vecConstructor.back();
				vecConstructor.pop_back();
				// 处理value是浮点数的情况
				fAugments = 0.0;

				// 处理是基本类型的情况，这种情况下还没有将值替换到value位置
				if(refWrapper.eType == JSONParserWrapperType_Bool || 
					refWrapper.eType == JSONParserWrapperType_Int ||
					refWrapper.eType == JSONParserWrapperType_Double ||
					refWrapper.eType == JSONParserWrapperType_Null)
				{
					vecConstructor.pop_back();
				}

				refNewWrapper = vecConstructor.back(); // 取出key
				 // 取出map
				if(vecConstructor.size() > 1)
				{
					refNewSecondWrapper = vecConstructor.at(vecConstructor.size() - 2);
				}
				if(vecConstructor.size() > 1 && 
					refNewWrapper.eType == JSONParserWrapperType_String && 
					refNewSecondWrapper.eType == JSONParserWrapperType_Map_Void) // 上一层是字典，且有对应key在等待中
				{
					string strKey = string(*((string*)refNewWrapper.pData)); // 将key转为对象
					// 原来保存的key不用再留着了
					vecConstructor.pop_back();
					delete ((string*)refNewWrapper.pData);
					// 加入到等待的map中
					map<string, void*>* pMapNewBack = (map<string, void*>*)refNewSecondWrapper.pData;
					pMapNewBack->insert(pair<string, void*>(strKey, refWrapper.pData));
				}
				else // 存在错误
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
				// 字符串开始，只有value或element的情况会遇到，key直接在keybegin处理了
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
					// string是value的值
					if(refWrapper.eType == JSONParserWrapperType_Value_PlaceHolder)
					{
						string* strValue = new string(strResult);
						refWrapper.eType = JSONParserWrapperType_String;
						refWrapper.pData = strValue;

						vecConstructor.pop_back();
						vecConstructor.push_back(refWrapper);
					}
					else if(refWrapper.eType == JSONParserWrapperType_Vector_Void) // string是数组的元素，再放回去
					{
						string* strValue = new string(strResult);
						vecConstructor.push_back(JSONParserWrapper(JSONParserWrapperType_String, strValue));
					}
				}
				else // 有问题
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
				// 一个元素结束了
				refWrapper = vecConstructor.back();
				vecConstructor.pop_back();
				// 元素是浮点数的情况
				fAugments = 0.0;

				refNewWrapper = vecConstructor.back();
				if(refNewWrapper.eType == JSONParserWrapperType_Vector_Void)
				{
					pVecArr = (vector<void*>*)refNewWrapper.pData;
					pVecArr->push_back(refWrapper.pData);
				}
				else // 有问题
				{
					return NULL;
				}
				break;
			case JSONAutoMataState_Arr_End:
				if(vecConstructor.size() == 0)
				{
					return NULL;
				}

				if(ePriorState == JSONAutoMataState_Arr_Begin) // 空数组的情况
				{
					ePriorState = JSONAutoMataState_Arr_End;

					refWrapper = vecConstructor.back();
					if(refWrapper.eType == JSONParserWrapperType_Vector_Void)
					{
						// 检查value的情况
						if(vecConstructor.size() > 1)
						{
							refNewWrapper = vecConstructor.at(vecConstructor.size() - 2);
							if(refNewWrapper.eType == JSONParserWrapperType_Value_PlaceHolder) // 数组是value的内容
							{
								vecConstructor.pop_back();
								vecConstructor.pop_back();

								vecConstructor.push_back(refWrapper);
							}
						}
					}
					else // 需要结束的不是数组
					{
						return NULL;
					}
				}
				else // 非空数组结束
				{
					ePriorState = JSONAutoMataState_Arr_End;
					// 此时前面还有一个元素等待入列表
					refWrapper = vecConstructor.back();
					vecConstructor.pop_back();
					// 处理元素是浮点数的情况
					fAugments = 0.0;

					refNewWrapper = vecConstructor.back();
					if(refNewWrapper.eType == JSONParserWrapperType_Vector_Void)
					{
						// 将最后一个元素放入列表
						pVecArr = (vector<void*>*)refNewWrapper.pData;
						pVecArr->push_back(refWrapper.pData);

						// 检查value的情况
						if(vecConstructor.size() > 1)
						{
							refWrapper = vecConstructor.back();
							refNewWrapper = vecConstructor.at(vecConstructor.size() - 2);
							if(refNewWrapper.eType == JSONParserWrapperType_Value_PlaceHolder) // 数组是value的内容
							{
								vecConstructor.pop_back();
								vecConstructor.pop_back();

								vecConstructor.push_back(refWrapper);
							}
						}
					}
					else // 需要结束的不是数组
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
				// 处理字符串内容
				refWrapper = vecConstructor.back();
				if(refWrapper.eType == JSONParserWrapperType_StringBuffer) // 是正在处理中的字符串
				{
					stringstream* pSSBuffer = (stringstream*)refWrapper.pData;
					(*pSSBuffer) << cCurrent;
				}
				else if(ePriorState == JSONAutoMataState_Dic_Value_Begin || 
					ePriorState == JSONAutoMataState_Arr_Elem_Seg || 
					ePriorState == JSONAutoMataState_Arr_Begin) // 正在处理的其他基本类型
				{
					if(refWrapper.eType == JSONParserWrapperType_Bool) // 布尔型的非第一个字符
					{
						//什么都不做
					}
					else if(refWrapper.eType == JSONParserWrapperType_Null) // 处理中的null
					{
						// 什么都不做
					}
					else if(refWrapper.eType == JSONParserWrapperType_Int)
					{
						int* pInt = (int*)refWrapper.pData;
						if(fAugments > 0.0) // 是浮点数，需要处理
						{
							double* pDouble = (double*)malloc(sizeof(double));
							(*pDouble) = ((double)(*pInt)) + fAugments * (cCurrent - '0'); // 添加的是小数位
							fAugments *= 0.1; // 加一步
							// 删除原来的整数
							delete pInt;
							// 添加新的浮点数
							refWrapper.eType = JSONParserWrapperType_Double;
							refWrapper.pData = pDouble;

							vecConstructor.pop_back();
							vecConstructor.push_back(refWrapper);
						}
						else
						{
							if(cCurrent == '.') // 遇到小数点
							{
								fAugments = 0.1;
							}
							else // 普通整数继续
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
					else // 第一个字符
					{
						if(cCurrent == 't') // 布尔true
						{
							bool bBool = true;
							vecConstructor.push_back(JSONParserWrapper(JSONParserWrapperType_Bool, (void*)bBool));
						}
						else if(cCurrent == 'f') // 布尔false
						{
							bool bBool = false;
							vecConstructor.push_back(JSONParserWrapper(JSONParserWrapperType_Bool, (void*)bBool));
						}
						else if(cCurrent == 'n' || cCurrent == 'N') // null
						{
							//string* strNULL = new string("NULL");
							vecConstructor.push_back(JSONParserWrapper(JSONParserWrapperType_Null, NULL));
						}
						else if(cCurrent >= '0' && cCurrent <= '9') // 数字
						{
							// 由于不能判断是整型还是浮点型，先按整型的来，如果处理过程中遇到小数点，再改成浮点型
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
				if(refWrapper.eType == JSONParserWrapperType_StringBuffer) // 是正在处理中的字符串
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