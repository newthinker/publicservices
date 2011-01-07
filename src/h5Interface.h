/*
 * Copyright(c)2010, michael
 * All rights reserved
 *
 *  File  name : h5Interface.h
 *  Discription:
 *  Created  on: Dec 29, 2010
 *       Author: michael, zuow11@gmail.com
 */

#ifndef H5INTERFACE_H_
#define H5INTERFACE_H_

#include <hdf5.h>
#include <stdlib.h>
#include "BaseDefine.h"
#include "CException.h"

// 宏定义：SDS结构体
typedef struct tag_cHDFData
{
	char* pData;
	int nRank;
	hsize_t* pDims;
	int nSize;
}cHDFData;

typedef struct tag_ucHDFData
{
	unsigned char* pData;
	int nRank;
	hsize_t* pDims;
	int nSize;
}ucHDFData;

typedef struct tag_sHDFData
{
	short* pData;
	int nRank;
	hsize_t* pDims;
	int nSize;
}sHDFData;

typedef struct tag_usHDFData
{
	unsigned short* pData;
	int nRank;
	hsize_t* pDims;
	int nSize;
}usHDFData;

typedef struct tag_nHDFData
{
	int* pData;
	int nRank;
	hsize_t* pDims;
	int nSize;
}nHDFData;

typedef struct tag_unHDFData
{
	unsigned int* pData;
	int nRank;
	hsize_t* pDims;
	int nSize;
}unHDFData;

typedef struct tag_fHDFData
{
	float* pData;
	int nRank;
	hsize_t* pDims;
	int nSize;
}fHDFData;

typedef struct tag_dHDFData
{
	double* pData;
	int nRank;
	hsize_t* pDims;
	int nSize;
}dHDFData;

/*
 * 定义开辟内存的宏，根据数据类型进行申请内存
 */
#define MemAlloc(rank, dims, size, ptr, type)																\
		type* type##_Memory_Alloc(int rank, hsize_t dims[], int *size, type **ptr)								\
{																											\
	int n = 1;																								\
	for(int i = 0; i < rank; i++)																			\
	{																										\
		n *= (int)dims[i];																					\
	}																										\
	n *= sizeof(type);																					\
	if (size != NULL)																						\
	{																										\
		*size = n;																							\
	}																										\
	if ((*ptr = malloc(n*sizeof(type))) == NULL)															\
	{																										\
		CException(E, "memory alloc failed"); 																\
	}																										\
	return *ptr;																								\
}

/*
 * 函数描述：打开或者是创建HDF文件:bOpen=0,创建；open=1,打开；
 */
hid_t iH5Open(char* filename, BOOL bOpen);

/*
 * 函数描述：关闭HDF文件对象
 */
void iH5Close();

/*
 * 函数描述：读字符型属性数据
 * 输入参数：loc_id 群组id
 * 			name 属性名称
 * 			value 属性值
 * 			bDisp 是否显示，用于调试
 */
void iH5ReadStrAttr(hid_t loc_id, const char *name, char **value, BOOL bDisp);

/*
 * 函数描述：写字符型属性数据
 * 输入参数：loc_id 群组id
 * 			name 属性名称
 * 			value 属性值
 * 			bDisp 是否显示，用于调试
 */
void iH5WriteStrAttr(hid_t loc_id, const char* name, char* value, BOOL bDisp);

/*
 * 宏定义：读数字型属性数据
 */
#define ReadNumAttr(loc_id, name, rank, dims, type_id, size, data, type, order, bDisp) 			\
		void type##_ReadNumAttr(hid_t loc_id, const char *name, int rank, hsize_t **dims, 		\
				hid_t type_id, int *size, type **data, H5T_order_t order, BOOL bDisp)			\
{																								\
	hid_t space_id, attr_id;																	\
	if (name == NULL)																			\
	{																							\
		CException(E, "attribute name is empty");										\
	}																							\
	if ((attr_id = H5Aopen_name(loc_id, name)) < 0)												\
	{																							\
		CException(E, "open attribute failed", name);									\
	}																							\
	if ((type_id = H5Aget_type(attr_id)) < 0)													\
	{																							\
		CException(E, "get attribute type failed", name);								\
	}																							\
	if ((order = H5Tget_order(type_id)) < 0)													\
	{																							\
		CException(E, "get order failed", name);											\
	}																							\
	if ((space_id = H5Aget_space(attr_id)) < 0)													\
	{																							\
		CException(E, "get data space size failed", name);								\
	}																							\
	if ((rank = H5Sget_simple_extent_ndims(space_id)) < 0)										\
	{																							\
		CException(E, "get data space dimension failed", name);							\
	}																							\
	if ((*dims = (hsize_t*)malloc(rank*sizeof(hsize_t))) == NULL)													\
	{																							\
		CException(E, "memory alloc failed", name);										\
	}																							\
	if (H5Sget_simple_extent_dims(space_id, *dims, NULL) < 0)									\
	{																							\
		free(*dims);																		\
		*dims = NULL;																			\
		CException(E, "get dimension failed", name);										\
	}																							\
																						\
	if ((*data = type##_Memory_Alloc(rank, *dims, NULL, data)) == NULL)									\
	{																							\
		free(*dims);																		\
		*dims = NULL;																			\
		CException(E, "memory alloc failed", name);										\
	}																								\
	if (H5Aread(attr_id, type_id, *data) < 0)													\
	{																							\
		free(*dims);																		\
		free(*data);																		\
		*dims = NULL;																			\
		*data = NULL;																			\
		CException(E, "read attribute failed", name);									\
	}																							\
																								\
	H5Sclose(space_id);																			\
	H5Aclose(attr_id);																			\
																								\
	if (size != NULL)																			\
	{																							\
		int n = 1;																				\
		for(int i = 0; i < rank; i++)															\
		{																						\
			n *= (int)((*dims)[i]);																\
		}																						\
		n *= sizeof(type);																			\
		*size = n;																				\
	}																							\
																						\
	if (bDisp)																					\
	{																							\
		int i, n = (int)(*(*dims));																\
		printf("\n\nnumber attribute: %s\n", name);												\
		printf("rank = %d\ndimension = %d", rank, (int)(*(*dims)));								\
		for(i = 1; i < rank; i++)																\
		{																						\
			printf(" x %d", (int)(*(*dims+i)));												\
			n *= (int)(*(*dims+i));																\
		}																						\
		printf("\ndata:\n");																	\
																								\
		H5T_class_t class_id;																	\
		if ((class_id = H5Tget_class(type_id)) < 0)												\
		{																						\
			CException(E, "get datatype failed", name);									\
		}																						\
																								\
		const char *pstrFmt;																	\
		switch(class_id)																		\
		{																						\
		case H5T_INTEGER:																		\
			{																					\
				pstrFmt = "%d ";																\
				break;																			\
			}																					\
		case H5T_FLOAT:																			\
			{																					\
				pstrFmt = "%.6e ";																\
				break;																			\
			}																					\
		default:																				\
			{																					\
				CException(W, "unknown datatype", name);									\
			}																					\
		}																						\
																								\
		for(i = 0; i < n; i++)																	\
		{																						\
			printf(pstrFmt, *(*data+i));														\
		}																						\
		printf("\n");																			\
																								\
		if (order == H5T_ORDER_LE)																\
		{																						\
			printf("Little endian\n");															\
		}																						\
		else if (order == H5T_ORDER_BE)															\
		{																						\
			printf("Big endian\n");																\
		}																						\
		else																					\
		{																						\
			printf("Unknown endian\n");															\
		}																						\
		printf("\n\n");																			\
	}																							\
}

/*
 * 宏定义：写数字型属性数据
 */
#define WriteNumAttr(loc_id, name, rank, dims, type_id, size, data, type, order, bDisp)		\
		void type##_WriteNumAttr(hid_t loc_id, const char *name, int rank, hsize_t *dims, 		\
				hid_t type_id, int *size, type *data, H5T_order_t order, BOOL bDisp)		\
{																							\
	hid_t spaceid, typeid, attrid;														\
	if (name == NULL)																		\
	{																						\
		CException(E, "attribute name is empty");											\
	}																						\
	if (dims == NULL)																		\
	{																						\
		CException(E, "dimemsion is null", name);											\
	}																						\
	if ((spaceid = H5Screate(H5S_SIMPLE)) < 0)												\
	{																						\
		CException(E, "create attribute space failed", name);								\
	}																						\
	if ((typeid = H5Tcopy(type_id)) < 0)														\
	{																						\
		CException(E, "copy attribute datatype failed", name);								\
	}																						\
	if (H5Tset_order(typeid, order) < 0)													\
	{																						\
		CException(E, "set order failed", name); 											\
	}																						\
	if (H5Sset_extent_simple(spaceid, rank, dims, NULL) < 0)								\
	{																						\
		CException(E, "set data space size failed", name);									\
	}																						\
	if ((attrid = H5Acreate(loc_id, name, typeid, spaceid, H5P_DEFAULT)) < 0)			\
	{																						\
		CException(E, "create attribute failed", name); 									\
	}																						\
	if (H5Awrite(attrid, typeid, data) < 0)												\
	{																						\
		CException(E, "write attribute failed", name); 										\
	}																						\
																							\
	H5Sclose(spaceid);																		\
	H5Aclose(attrid);																		\
																							\
	if (size != NULL)																		\
	{																						\
		*size = 1;																			\
		for(int i = 0; i < rank; i++)														\
		{																					\
			*size *= (int)dims[i];															\
		}																					\
		*size *= sizeof(type);																	\
	}																						\
	if (bDisp)																				\
	{																						\
		int i, n = (int)(*dims);															\
		printf("\n\nnumber attribute: %s\n", name);											\
		printf("rank = %d\ndimension = %d", rank, (int)(*dims));							\
		for(i = 1; i < rank; i++)															\
		{																					\
			printf(" x %d", (int)(*dims+i));												\
			n *= (int)(*dims+i);															\
		}																					\
		printf("\ndata:\n");																\
		H5T_class_t class_id;																\
		if ((class_id = H5Tget_class(typeid)) < 0) 										\
		{																					\
			CException(E, "get datatype failed", name); 									\
		}																					\
																							\
		const char *pstrFmt;																\
		switch(class_id)																	\
		{																					\
		case H5T_INTEGER:																	\
			{																				\
				pstrFmt = "%d ";															\
				break;																		\
			}																				\
		case H5T_FLOAT:																		\
			{																				\
				pstrFmt = "%.6e ";															\
				break;																		\
			}																				\
		default:																			\
			{																				\
				CException(E, "unknown datatype", name);									\
			}																				\
		}																					\
																							\
		for(i = 0; i < n; i++)																\
		{																					\
			printf(pstrFmt, *data+i);													\
		}																					\
		printf("\n");																		\
																							\
		if (order == H5T_ORDER_LE)															\
		{																					\
			printf("Little endian\n");														\
		}																					\
		else if (order == H5T_ORDER_BE)														\
		{																					\
			printf("Big endian\n");															\
		}																					\
		else																				\
		{																					\
			printf("Unknown endian\n");														\
		}																					\
																							\
		printf("\n\n");																		\
	}																						\
}

/*
 * 函数描述：读多维字符串型属性：未测试！2008-10-21
 * 输入参数：loc_id 群组id
 * 			name 属性名称
 * 			rank 多维数组维度
 * 			dims 各维度数组长度
 * 			data 属性值
 * 			bDisp 是否显示
 */
void iH5ReadVLStrAttr(hid_t loc_id, const char *name, int rank, hsize_t *dims,
		char **data, BOOL bDisp);

/*
 * 函数描述：写多维字符串型属性
 * 输入参数：loc_id 群组id
 * 			name 属性名称
 * 			rank 多维数组维度
 * 			dims 各维度数组长度
 * 			data 属性值
 * 			bDisp 是否显示
 */
void iH5WriteVLStrAttr(hid_t loc_id, const char *name, int rank, hsize_t *dims,
		char *data[], BOOL bDisp);

/*
 * 函数描述：写多维字符串型SDS
 * 输入参数：loc_id 群组id
 * 			name 属性名称
 * 			rank 多维数组维度
 * 			dims 各维度数组长度
 * 			data 数据首指针
 * 			bDisp 是否显示
 */
void iH5WriteVLStrSDS(hid_t loc_id, const char *name, int rank, hsize_t *dims,
		char *data[], BOOL bDisp);

/*
 * 宏定义：读科学数据集
 */
#define ReadSDS(loc_id, name, rank, dims, type_id, size, data, type, order, bDisp)		\
void type##_ReadSDS(hid_t loc_id, const char *name, int rank, hsize_t **dims,					\
	hid_t type_id, int *size, type **data, H5T_order_t order, BOOL bDisp)				\
{																						\
	hid_t dataspace_id, dataset_id;														\
	if (name == NULL)																	\
	{																					\
		CException(E, "dataset name is empty");											\
	}																					\
	if ((dataset_id = H5Dopen(loc_id, name)) < 0)										\
	{																					\
		CException(E, "open dataset failed", name);									\
	}																					\
	if ((type_id = H5Dget_type(dataset_id)) < 0)										\
	{																					\
		CException(E, "get dataset type failed", name);									\
	}																					\
	if ((order = H5Tget_order(type_id)) < 0)											\
	{																					\
		CException(E, "get order failed", name);										\
	}																					\
	if ((dataspace_id = H5Dget_space(dataset_id)) < 0)									\
	{																					\
		CException(E, "get data space failed", name);									\
	}																					\
	if ((rank = H5Sget_simple_extent_ndims(dataspace_id)) < 0)							\
	{																					\
		CException(E, "get data space dimension failed", name);							\
	}																					\
	if ((*dims = malloc(rank*sizeof(hsize_t))) == NULL)											\
	{																					\
		CException(E, "memory alloc failed", name);										\
	}																					\
	if (H5Sget_simple_extent_dims(dataspace_id, *dims, NULL) < 0)						\
	{																					\
		free(*dims);																	\
		*dims = NULL;																	\
		CException(E, "get dimension failed", name);									\
	}																					\
																						\
	if ((*data = type##_Memory_Alloc(rank, *dims, NULL, data)) == NULL)							\
	{																					\
		free(*dims);																	\
		*dims = NULL;																	\
		CException(E, "memory alloc failed", name);										\
	}																					\
																						\
	if (H5Dread(dataset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, *data) < 0)			\
	{																					\
		free(*dims);																	\
		free(*data);																	\
		*dims = NULL;																	\
		*data = NULL;																	\
		CException(E, "read dataset failed", name);										\
	}																					\
																						\
	H5Sclose(dataspace_id);																\
	H5Dclose(dataset_id);																\
																						\
	if (size != NULL)																	\
	{																					\
		int n = 1;																		\
		for(int i = 0; i < rank; i++)													\
		{																				\
			n *= (int)((*dims)[i]);														\
		}																				\
		n *= sizeof(type);																\
		*size = n;																		\
	}																					\
																						\
	if (bDisp)																			\
	{																					\
		int i, n = (int)(*(*dims));														\
		printf("\n\nDataset name : %s\n", name);										\
		printf("rank = %d\ndimension = %d", rank, (int)(*(*dims)));						\
		for(i = 1; i < rank; i++)														\
		{																				\
			printf(" x %d", (int)(*(*dims+i)));											\
			n *= (int)(*(*dims+i));														\
		}																				\
		printf("\ndata:\n");															\
																						\
		H5T_class_t class_id;															\
		if ((class_id = H5Tget_class(type_id)) < 0)										\
		{																				\
			CException(E, "get datatype failed", name);									\
		}																				\
																						\
		const char *pstrFmt;															\
		switch(class_id)																\
		{																				\
		case H5T_INTEGER:																\
			{																			\
				pstrFmt = "%d ";														\
				break;																	\
			}																			\
		case H5T_FLOAT:																	\
			{																			\
				pstrFmt = "%.6e ";														\
				break;																	\
			}																			\
		default:																		\
			{																			\
				CException(E, "unknown datatype", name);									\
			}																			\
		}																				\
																						\
		for(i = 0; i < n; i++)															\
		{																				\
			printf(pstrFmt, *(*data+i));												\
		}																				\
		printf("\n");																	\
																						\
																						\
		if (order == H5T_ORDER_LE)														\
		{																				\
			printf("Little endian\n");													\
		}																				\
		else if (order == H5T_ORDER_BE)													\
		{																				\
			printf("Big endian\n");														\
		}																				\
		else																			\
		{																				\
			printf("Unknown endian\n");													\
		}																				\
																						\
		printf("\n\n");																	\
	}																					\
}

/*
 * 宏定义：写科学数据集
 */
#define WriteSDS(loc_id, name, rank, dims, type_id, size, data, type, order, bDisp)			\
	void type##_WriteSDS(hid_t loc_id, const char *name, int rank, hsize_t *dims,			\
			hid_t type_id, int *size, type *data, H5T_order_t order, BOOL bDisp)			\
{																							\
	hid_t dataspace_id, dataset_id, typeid;												\
	if (name == NULL)																		\
	{																						\
		CException(E, "dataset name is empty");												\
	}																						\
	if ((dataspace_id = H5Screate_simple(rank, dims, NULL)) < 0)							\
	{																						\
		CException(E, "create dataset space failed", name);									\
	}																						\
	if ((typeid = H5Tcopy(type_id)) < 0)													\
	{																						\
		CException(E, "copy datatype failed", name);										\
	}																						\
	if (H5Tset_order(typeid, order) < 0)													\
	{																						\
		CException(E, "set order failed", name);											\
	}																						\
	if ((dataset_id = H5Dcreate(loc_id, name, typeid, dataspace_id, H5P_DEFAULT)) < 0)		\
	{																						\
		CException(E, "create dataset failed", name);										\
	}																						\
	if ((H5Dwrite(dataset_id, typeid, H5S_ALL, H5S_ALL, H5P_DEFAULT, data)) < 0)			\
	{																						\
		CException(E, "write dataset failed", name);										\
	}																						\
																							\
	H5Sclose(dataspace_id);																	\
	H5Dclose(dataset_id);																	\
																							\
	if (size != NULL)																		\
	{																						\
		*size = 1;																			\
		for(int i = 0; i < rank; i++)														\
		{																					\
			*size *= (int)dims[i];															\
		}																					\
		*size *= sizeof(type);																\
	}																						\
																							\
	if(bDisp)																			\
	{																					\
		int i, n = (int)(*dims);														\
		printf("\n\nDataset name : %s\n", name);										\
		printf("rank = %d\ndimension = %d", rank, (int)(*dims));						\
		for(i = 1; i < rank; i++)														\
		{																				\
			printf(" x %d", (int)(*dims+i));											\
			n *= (int)(*dims+i);														\
		}																				\
		printf("\ndata:\n");															\
																						\
		H5T_class_t class_id;															\
		if ((class_id = H5Tget_class(typeid)) < 0)										\
		{																				\
			CException(E, "get datatype failed", name);									\
		}																				\
																						\
		const char *pstrFmt;															\
		switch(class_id)																\
		{																				\
		case H5T_INTEGER:																\
			{																			\
				pstrFmt = "%d ";														\
				break;																	\
			}																			\
		case H5T_FLOAT:																	\
			{																			\
				pstrFmt = "%.6e ";														\
				break;																	\
			}																			\
		default:																		\
			{																			\
				CException(E, "unknown datatype", name);									\
			}																			\
		}																				\
																						\
		for(i = 0; i < n; i++)															\
		{																				\
			printf(pstrFmt, *data+i);												\
		}																				\
		printf("\n");																	\
																						\
																						\
		if (order == H5T_ORDER_LE)														\
		{																				\
			printf("Little endian\n");													\
		}																				\
		else if (order == H5T_ORDER_BE)													\
		{																				\
			printf("Big endian\n");														\
		}																				\
		else																			\
		{																				\
			printf("Unknown endian\n");													\
		}																				\
																						\
		printf("\n\n");																	\
	}																					\
}

#endif /* H5INTERFACE_H_ */
