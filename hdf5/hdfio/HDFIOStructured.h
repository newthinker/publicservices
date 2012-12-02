/************************************************************************************************************************************************************************
 * �汾�ţ�0 . 8
 * ��Ҫ������CHDFIOStructured��Ҫ��ɴ�HDF5�ļ���ȡ���ݼ������Ժ���HDF5�ļ�д�����ݼ������ԵĹ���
 * ��д�ߣ�
 * ��д���ڣ�2007/08/15
 * .�ϴ��޸����ڣ�2007/09/16
 * �޸���־��(2007/08/24)�����˶����ݼ������Եĸ���д�Ͷ����ݼ���׷��д�Ĺ��ܣ��޸��������Ϣ����ʾ
 *                       (2007/08/31)�����˻�ȡ���ݼ���С���������������ĳЩ�ṹ
 *                       (2007/09/16)������д˽�����Ժ��������غ�����֧�ֶ�ά˽�����ԣ�ͬʱ��ǿ��Ľ�׳�ԣ����ļ��򿪻��ߴ���ʧ��ʱ��������
 *                                              ���и���һ���Ĳ���
 * ***********************************************************************************************************************************************************************/
#ifndef HDFIOSTRUCTURED_H_
#define HDFIOSTRUCTURED_H_
#include "CompoundType.h"
#include <hdf5.h>
#include <H5LT.h>
#include <H5TA.h>
//#include <zlib.h>
//#include <zconf.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string.h>

#define GROUP_SDS "/"
#define GROUP_TABLE "/" 

using namespace std;

/*���ݼ����Խṹ*/
typedef struct{
	char* units;
	unsigned short valid_range[2];
	unsigned short fill_value;
	char* long_name;
	float slope;
	float intercept;
}DSET_ATTR;


/*HDF���������*/
class HDFIOStructured
{
private:
	string fileName;
	string dsetname;
	int initFile();
	
public:
	HDFIOStructured();
	virtual ~HDFIOStructured();
	
private:
	bool isTable;
	bool isFirst;
	hid_t       file_id; 
	hid_t       dset_id;
	herr_t      status;
	CTYPE_INFO  cdtype_info;

public:
	int initHDFIOStructured(const char* file_name);
	void closeFile();
	
	/************************************************��ʼ������չ���ݼ����������Ը���ԭ�����ݼ���***************************************************/
	int initSDSEX(const char* dset_name,int rank,const hsize_t* dims,hid_t type_id,int dim_ex);
	
	/******************************************************************�������ݼ�����************************************************************************/
	int connSDSEX(const char* dset_name);

	/********************************************************************���Զ�ȡ����***********************************************************************/
	int ReadGlbAttrFromHDF(const char* attr_name,hid_t type_id,void* buf);
	int ReadPrtAttrFromHDF(const char* attr_name,hid_t type_id,void* buf);
	int ReadSDSAttrFromHDF(const char* attr_name,hid_t type_id,void* buf,const char* dset_name);
	
	/************************************************************����д�루���������ǣ�����**************************************************************/
	int WriteGlbAttrToHDF(const char* attr_name,hid_t type_id,const void* buf,hsize_t size);
	int WritePrtAttrToHDF(const char* attr_name,hid_t type_id,const void* buf,hsize_t size);
	int WriteSDSAttrToHDF(const char* attr_name,hid_t type_id,const void* buf,hsize_t size,const char* dset_name);
	int WritePrtAttrToHDF(const char* attr_name,hid_t type_id,const void* buf,int rank,hsize_t* dims);
	
	/*******************************************************************��ȡ���ݼ���С����/��ȡɨ������������*********************************************/
	hsize_t getSDSSize(const char* dset_name); 
	int getFrameNum(void);
	
	/**********************************************************���ݼ�һ���Զ�ȡд�뺯��********************************************************************/
	int ReadSDSFromHDF(const char* dset_name,hid_t type_id,void* buf);
	int WriteSDSToHDF(const char* dset_name,int rank,const hsize_t* dims,hid_t type_id,const void* buf);
	
	/******************************************************���ݼ��Լ������ڸ����ݼ�������һ��д�뺯��**************************************************/
	int WriteUnionToHDF(const char* dset_name,int rank,const hsize_t* dims,hid_t type_id,const void* buf,DSET_ATTR* dset_attr);
	
	/******************************************************************���ݼ����ֶ�ȡ��׷��д�뺯��*********************************************************/
	int ReadSDSFromHDFEX(void* buf,hsize_t data_num,hsize_t data_begin=0,int dim_main=0);
	int WriteSDSToHDFEX(const void* buf,hsize_t data_num,hsize_t* data_begin=NULL);
	
	/*******************************************************************��д����******************************************************************************/
	int ReadVDataFromHDF(const char* table_name,CCompound& cdRef,void* record,hsize_t record_num,hsize_t start=0);
	int WriteVDataToHDF(const char* table_name,CCompound& cdRef,const void * record,hsize_t record_num);
	
	/***********************************************************************************************************/
	int DeleteSDS(const char* dset_name);
	const char* getFileName();
	
};
int maintest();
#endif /*HDFIOSTRUCTURED_H_*/
