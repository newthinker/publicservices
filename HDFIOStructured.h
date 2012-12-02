/************************************************************************************************************************************************************************
 * 版本号：0 . 8
 * 简要描述：CHDFIOStructured主要完成从HDF5文件读取数据集、属性和向HDF5文件写入数据集、属性的功能
 * 编写者：
 * 编写日期：2007/08/15
 * .上次修改日期：2007/09/16
 * 修改日志：(2007/08/24)增加了对数据集、属性的覆盖写和对数据集的追加写的功能，修改了输出信息的显示
 *                       (2007/08/31)增加了获取数据集大小函数，调整了类的某些结构
 *                       (2007/09/16)增加了写私有属性函数的重载函数以支持多维私有属性，同时增强类的健壮性，在文件打开或者创建失败时，不允许
 *                                              进行更进一步的操作
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

/*数据集属性结构*/
typedef struct{
	char* units;
	unsigned short valid_range[2];
	unsigned short fill_value;
	char* long_name;
	float slope;
	float intercept;
}DSET_ATTR;


/*HDF输入输出类*/
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
	
	/************************************************初始化可扩展数据集函数（可以覆盖原有数据集）***************************************************/
	int initSDSEX(const char* dset_name,int rank,const hsize_t* dims,hid_t type_id,int dim_ex);
	
	/******************************************************************连接数据集函数************************************************************************/
	int connSDSEX(const char* dset_name);

	/********************************************************************属性读取函数***********************************************************************/
	int ReadGlbAttrFromHDF(const char* attr_name,hid_t type_id,void* buf);
	int ReadPrtAttrFromHDF(const char* attr_name,hid_t type_id,void* buf);
	int ReadSDSAttrFromHDF(const char* attr_name,hid_t type_id,void* buf,const char* dset_name);
	
	/************************************************************属性写入（创建、覆盖）函数**************************************************************/
	int WriteGlbAttrToHDF(const char* attr_name,hid_t type_id,const void* buf,hsize_t size);
	int WritePrtAttrToHDF(const char* attr_name,hid_t type_id,const void* buf,hsize_t size);
	int WriteSDSAttrToHDF(const char* attr_name,hid_t type_id,const void* buf,hsize_t size,const char* dset_name);
	int WritePrtAttrToHDF(const char* attr_name,hid_t type_id,const void* buf,int rank,hsize_t* dims);
	
	/*******************************************************************获取数据集大小函数/获取扫描线总数函数*********************************************/
	hsize_t getSDSSize(const char* dset_name); 
	int getFrameNum(void);
	
	/**********************************************************数据集一次性读取写入函数********************************************************************/
	int ReadSDSFromHDF(const char* dset_name,hid_t type_id,void* buf);
	int WriteSDSToHDF(const char* dset_name,int rank,const hsize_t* dims,hid_t type_id,const void* buf);
	
	/******************************************************数据集以及依附于该数据集的属性一起写入函数**************************************************/
	int WriteUnionToHDF(const char* dset_name,int rank,const hsize_t* dims,hid_t type_id,const void* buf,DSET_ATTR* dset_attr);
	
	/******************************************************************数据集部分读取、追加写入函数*********************************************************/
	int ReadSDSFromHDFEX(void* buf,hsize_t data_num,hsize_t data_begin=0,int dim_main=0);
	int WriteSDSToHDFEX(const void* buf,hsize_t data_num,hsize_t* data_begin=NULL);
	
	/*******************************************************************读写表函数******************************************************************************/
	int ReadVDataFromHDF(const char* table_name,CCompound& cdRef,void* record,hsize_t record_num,hsize_t start=0);
	int WriteVDataToHDF(const char* table_name,CCompound& cdRef,const void * record,hsize_t record_num);
	
	/***********************************************************************************************************/
	int DeleteSDS(const char* dset_name);
	const char* getFileName();
	
};
int maintest();
#endif /*HDFIOSTRUCTURED_H_*/
