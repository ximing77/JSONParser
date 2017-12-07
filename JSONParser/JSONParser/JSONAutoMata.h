#ifndef JSONAUTOMATA_H
#define JSONAUTOMATA_H

#include <vector>

using namespace std;

typedef enum JSONAutoMataState
{
	JSONAutoMataState_None = 0,
	JSONAutoMataState_Space,
	JSONAutoMataState_Escape,
	JSONAutoMataState_Dic_Begin,
	JSONAutoMataState_Dic_End,
	JSONAutoMataState_Dic_Key_Begin,
	JSONAutoMataState_Dic_Key_End,
	JSONAutoMataState_Dic_Value_Begin,
	JSONAutoMataState_Dic_Value_End,
	JSONAutoMataState_Arr_Begin,
	JSONAutoMataState_Arr_End,
	JSONAutoMataState_Arr_Elem_Seg,
	JSONAutoMataState_String_Begin,
	JSONAutoMataState_String_End,
	JSONAutoMataState_Error_Dic_Imcomplete,
	JSONAutoMataState_Error_Arr_Imcomplete,
	JSONAutoMataState_Error_Key_Imcomplete,
	JSONAutoMataState_Error_Value_Imcomplete,
	JSONAutoMataState_Error_String_Imcomplete,
	JSONAutoMataState_Error_Other,
	JSONAutoMataState_Fin
}JSONAutoMataState;

class __declspec(dllexport) JSONAutoMata
{
public:
	JSONAutoMataState eState;
	vector<JSONAutoMataState> stkState;

public:
	JSONAutoMata();
	~JSONAutoMata();

	int clear();
	JSONAutoMataState feed(char cInput);
	JSONAutoMataState finish();
};

#endif