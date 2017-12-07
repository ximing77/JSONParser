#include "stdafx.h"

#include "JSONAutoMata.h"

JSONAutoMata::JSONAutoMata()
{
	eState = JSONAutoMataState_None;
	stkState = vector<JSONAutoMataState>();
}

JSONAutoMata::~JSONAutoMata()
{

}

int JSONAutoMata::clear()
{
	eState = JSONAutoMataState_None;
	stkState.clear();

	return 0;
}

JSONAutoMataState JSONAutoMata::feed(char cInput)
{
	int nTopPos = (int)(stkState.size() - 1);
	// 出错后不再继续处理
	if(eState >= JSONAutoMataState_Error_Dic_Imcomplete && eState <= JSONAutoMataState_Error_Other)
	{
		return eState;
	}
	
	if(nTopPos > -1 && stkState.at(nTopPos) == JSONAutoMataState_Escape) // 转义字符直接继续
	{
		stkState.pop_back();
		eState = JSONAutoMataState_None;
		return eState;
	}
	else if(nTopPos > -1 && stkState.at(nTopPos) == JSONAutoMataState_String_Begin && !(cInput == '"')) // 字符串内内容不管
	{
		if(eState == '\\') // 字符串内转义字符
		{
			eState = JSONAutoMataState_Escape;
			stkState.push_back(JSONAutoMataState_Escape);
		}
		else // 普通字符
		{
			eState = JSONAutoMataState_None;
		}
		return eState;
	}

	switch(cInput)
	{
	case ' ': // 空格
		return JSONAutoMataState_Space;
		break;
	case '\\': // 转义字符
		eState = JSONAutoMataState_Escape;
		stkState.push_back(JSONAutoMataState_Escape);
		break;
	case '{':
		if(eState == JSONAutoMataState_Dic_Key_Begin) // key只能是字符串，不能有结构
		{
			eState = JSONAutoMataState_Error_Key_Imcomplete;
			break;
		}
		eState = JSONAutoMataState_Dic_Begin;
		stkState.push_back(JSONAutoMataState_Dic_Begin);
		break;
	case '}':
		eState = JSONAutoMataState_Dic_End;
		if(nTopPos > -1 && stkState.at(nTopPos) == JSONAutoMataState_Dic_Value_Begin) // 最后一个value
		{
			stkState.pop_back();
			--nTopPos;
		}
		if(nTopPos > -1 && stkState.at(nTopPos) == JSONAutoMataState_Dic_Begin)
		{
			stkState.pop_back();
		}
		else
		{
			eState = JSONAutoMataState_Error_Dic_Imcomplete;
		}
		break;
	case '[':
		if(eState == JSONAutoMataState_Dic_Key_Begin) // key只能是字符串，不能有结构
		{
			eState = JSONAutoMataState_Error_Key_Imcomplete;
			break;
		}
		eState = JSONAutoMataState_Arr_Begin;
		stkState.push_back(JSONAutoMataState_Arr_Begin);
		break;
	case ']':
		eState = JSONAutoMataState_Arr_End;
		if(nTopPos > -1 && stkState.at(nTopPos) == JSONAutoMataState_Arr_Begin)
		{
			stkState.pop_back();
		}
		else
		{
			eState = JSONAutoMataState_Error_Arr_Imcomplete;
		}
		break;
	case '"':
		if(nTopPos > -1 && stkState.at(nTopPos) == JSONAutoMataState_Dic_Key_Begin)
		{
			eState = JSONAutoMataState_Dic_Key_End;
			stkState.pop_back();
		}
		else if(eState == JSONAutoMataState_Dic_Begin || eState == JSONAutoMataState_Dic_Value_End)
		{
			eState = JSONAutoMataState_Dic_Key_Begin;
			stkState.push_back(eState);
		}
		else // 普通字符串
		{
			if(nTopPos > -1 && stkState.at(nTopPos) == JSONAutoMataState_String_Begin)
			{
				eState = JSONAutoMataState_String_End;
				stkState.pop_back();
			}
			else
			{
				eState = JSONAutoMataState_String_Begin;
				stkState.push_back(JSONAutoMataState_String_Begin);
			}
		}
		break;
	case ':':
		if(eState == JSONAutoMataState_Dic_Key_Begin) // key只能是字符串，不能有结构
		{
			eState = JSONAutoMataState_Error_Key_Imcomplete;
			break;
		}
		eState = JSONAutoMataState_Dic_Value_Begin;
		stkState.push_back(eState);
		break;
	case ',':
		if(nTopPos > -1 && stkState.at(nTopPos) == JSONAutoMataState_Dic_Value_Begin) // value结束
		{
			eState = JSONAutoMataState_Dic_Value_End;
			stkState.pop_back();
		}
		else if(nTopPos > -1 && stkState.at(nTopPos) == JSONAutoMataState_Arr_Begin) // elem分割
		{
			eState = JSONAutoMataState_Arr_Elem_Seg;
		}
		else // 不完整
		{
			eState = JSONAutoMataState_Error_Value_Imcomplete;
		}
		break;
	default:
		eState = JSONAutoMataState_None;
	}
	return eState;
}

JSONAutoMataState JSONAutoMata::finish()
{
	if(stkState.size() == 0)
	{
		return JSONAutoMataState_Fin;
	}
	return JSONAutoMataState_Error_Other;
}