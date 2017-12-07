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
	// ������ټ�������
	if(eState >= JSONAutoMataState_Error_Dic_Imcomplete && eState <= JSONAutoMataState_Error_Other)
	{
		return eState;
	}
	
	if(nTopPos > -1 && stkState.at(nTopPos) == JSONAutoMataState_Escape) // ת���ַ�ֱ�Ӽ���
	{
		stkState.pop_back();
		eState = JSONAutoMataState_None;
		return eState;
	}
	else if(nTopPos > -1 && stkState.at(nTopPos) == JSONAutoMataState_String_Begin && !(cInput == '"')) // �ַ��������ݲ���
	{
		if(eState == '\\') // �ַ�����ת���ַ�
		{
			eState = JSONAutoMataState_Escape;
			stkState.push_back(JSONAutoMataState_Escape);
		}
		else // ��ͨ�ַ�
		{
			eState = JSONAutoMataState_None;
		}
		return eState;
	}

	switch(cInput)
	{
	case ' ': // �ո�
		return JSONAutoMataState_Space;
		break;
	case '\\': // ת���ַ�
		eState = JSONAutoMataState_Escape;
		stkState.push_back(JSONAutoMataState_Escape);
		break;
	case '{':
		if(eState == JSONAutoMataState_Dic_Key_Begin) // keyֻ�����ַ����������нṹ
		{
			eState = JSONAutoMataState_Error_Key_Imcomplete;
			break;
		}
		eState = JSONAutoMataState_Dic_Begin;
		stkState.push_back(JSONAutoMataState_Dic_Begin);
		break;
	case '}':
		eState = JSONAutoMataState_Dic_End;
		if(nTopPos > -1 && stkState.at(nTopPos) == JSONAutoMataState_Dic_Value_Begin) // ���һ��value
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
		if(eState == JSONAutoMataState_Dic_Key_Begin) // keyֻ�����ַ����������нṹ
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
		else // ��ͨ�ַ���
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
		if(eState == JSONAutoMataState_Dic_Key_Begin) // keyֻ�����ַ����������нṹ
		{
			eState = JSONAutoMataState_Error_Key_Imcomplete;
			break;
		}
		eState = JSONAutoMataState_Dic_Value_Begin;
		stkState.push_back(eState);
		break;
	case ',':
		if(nTopPos > -1 && stkState.at(nTopPos) == JSONAutoMataState_Dic_Value_Begin) // value����
		{
			eState = JSONAutoMataState_Dic_Value_End;
			stkState.pop_back();
		}
		else if(nTopPos > -1 && stkState.at(nTopPos) == JSONAutoMataState_Arr_Begin) // elem�ָ�
		{
			eState = JSONAutoMataState_Arr_Elem_Seg;
		}
		else // ������
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