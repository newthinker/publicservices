//============================================================================
// Name        : ParamList.cpp
// Author      : ZuoW
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
using namespace std;
#include <string.h>

#include "ParamList.h"

int main()
{

	ParamList pl;
	string strFile = "/home/aero/dps/workdir/FY3A/IRAS_QC_RSData_XCONF.xcfg";		//CCD2/LEVEL3/374/AP_eph_data.txt";
	pl.MakeList(strFile);

	pl.Output();

	double dValue[6];	int number=6;
	pl.GetValue("fSecondPowerCoef", dValue, number);

	for(int num=0; num<number; num++)
	{
		cout<<dValue[num] << endl;
		dValue[num] += 1;
	}

	pl.SetValue("fSecondPowerCoef", dValue, number, NULL);
	pl.GetValue("fSecondPowerCoef", dValue, number);

	for(int num=0; num<number; num++)
	{
		cout<<dValue[num] << endl;
		dValue[num] += 1;
	}

	pl.MakeFile("/dps/workdir/FY3A/test.txt");

/*	p_list* ptr = pl._search("fSecondPowerCoef");

	if(ptr!=NULL)
		cout << ptr->item.value << endl;
*/
/*	int a[3][3] = {{1,2,3}, {4,5,6}, {7,8,9}};
	int* b;		int (*c)[3];	int* d[3];

//	cout<<a<<','<< b<<','<<c<<endl;
	b = a[0];
//	cout<<*(b)<<','<<*(b+1)<<','<<*(b+2)<<endl;
//	d[0] = a[0];	d[1] = a[1];	d[2] = a[2];
//	cout<<*(d[0])<<','<<*(d[1])<<','<<*(d[2])<<endl;
//	cout<<*(d+1)<<','<<*(d[0]+1)<<','<<*(d+2)<<endl;
//	cout<<a[0][1]<<','<<&(a[1][0])<<','<<&(a[2][0])<<endl;
//	cout<<&(a[0])<<','<<&(a[1])<<','<<&(a[2])<<endl;
*/
	return 0;
}
