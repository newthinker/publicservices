/*
 * Copyright(c)2010, michael
 * All rights reserved
 *
 *  File  name : h5Interface.c
 *  Discription: 基于FY3A中王彬的一个C++格式HDF读写接口，重新进行封装的一个C格式的HDF读写接口
 *  Created  on: Dec 29, 2010
 *       Author: michael, zuow11@gmail.com
 */

#include <unistd.h>
#include <string.h>
#include "h5Interface.h"

static hid_t file_id;
static hid_t root_group_id;

MemAlloc(rank, dims, size, ptr, char)
MemAlloc(rank, dims, size, ptr, UCHAR)
MemAlloc(rank, dims, size, ptr, short)
MemAlloc(rank, dims, size, ptr, USHORT)
MemAlloc(rank, dims, size, ptr, int)
MemAlloc(rank, dims, size, ptr, UINT)
MemAlloc(rank, dims, size, ptr, float)
MemAlloc(rank, dims, size, ptr, double)
MemAlloc(rank, dims, size, ptr, long)
MemAlloc(rank, dims, size, ptr, ULONG)

ReadNumAttr(loc_id, name, rank, dims, type_id, size, data, short, order, bDisp)
ReadNumAttr(loc_id, name, rank, dims, type_id, size, data, USHORT, order, bDisp)
ReadNumAttr(loc_id, name, rank, dims, type_id, size, data, int, order, bDisp)
ReadNumAttr(loc_id, name, rank, dims, type_id, size, data, UINT, order, bDisp)
ReadNumAttr(loc_id, name, rank, dims, type_id, size, data, long, order, bDisp)
ReadNumAttr(loc_id, name, rank, dims, type_id, size, data, ULONG, order, bDisp)
ReadNumAttr(loc_id, name, rank, dims, type_id, size, data, float, order, bDisp)
ReadNumAttr(loc_id, name, rank, dims, type_id, size, data, double, order, bDisp)

WriteNumAttr(loc_id, name, rank, dims, type_id, size, data, short, order, bDisp)
WriteNumAttr(loc_id, name, rank, dims, type_id, size, data, USHORT, order, bDisp)
WriteNumAttr(loc_id, name, rank, dims, type_id, size, data, int, order, bDisp)
WriteNumAttr(loc_id, name, rank, dims, type_id, size, data, UINT, order, bDisp)
WriteNumAttr(loc_id, name, rank, dims, type_id, size, data, long, order, bDisp)
WriteNumAttr(loc_id, name, rank, dims, type_id, size, data, ULONG, order, bDisp)
WriteNumAttr(loc_id, name, rank, dims, type_id, size, data, float, order, bDisp)
WriteNumAttr(loc_id, name, rank, dims, type_id, size, data, double, order, bDisp)

ReadSDS(loc_id, name, rank, dims, type_id, size, data, char, order, bDisp)
ReadSDS(loc_id, name, rank, dims, type_id, size, data, UCHAR, order, bDisp)
ReadSDS(loc_id, name, rank, dims, type_id, size, data, short, order, bDisp)
ReadSDS(loc_id, name, rank, dims, type_id, size, data, USHORT, order, bDisp)
ReadSDS(loc_id, name, rank, dims, type_id, size, data, int, order, bDisp)
ReadSDS(loc_id, name, rank, dims, type_id, size, data, UINT, order, bDisp)
ReadSDS(loc_id, name, rank, dims, type_id, size, data, long, order, bDisp)
ReadSDS(loc_id, name, rank, dims, type_id, size, data, ULONG, order, bDisp)
ReadSDS(loc_id, name, rank, dims, type_id, size, data, float, order, bDisp)
ReadSDS(loc_id, name, rank, dims, type_id, size, data, double, order, bDisp)

WriteSDS(loc_id, name, rank, dims, type_id, size, data, char, order, bDisp)
WriteSDS(loc_id, name, rank, dims, type_id, size, data, UCHAR, order, bDisp)
WriteSDS(loc_id, name, rank, dims, type_id, size, data, short, order, bDisp)
WriteSDS(loc_id, name, rank, dims, type_id, size, data, USHORT, order, bDisp)
WriteSDS(loc_id, name, rank, dims, type_id, size, data, int, order, bDisp)
WriteSDS(loc_id, name, rank, dims, type_id, size, data, UINT, order, bDisp)
WriteSDS(loc_id, name, rank, dims, type_id, size, data, long, order, bDisp)
WriteSDS(loc_id, name, rank, dims, type_id, size, data, ULONG, order, bDisp)
WriteSDS(loc_id, name, rank, dims, type_id, size, data, float, order, bDisp)
WriteSDS(loc_id, name, rank, dims, type_id, size, data, double, order, bDisp)

hid_t iH5Open(char* filename, BOOL bOpen)
{
	if (filename == NULL)
	{
		CException(E, "HDF5 filename is empty!");
	}

	if (!bOpen)
	{
		if ((file_id = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)) < 0)
		{
			CException(E, "create HDF5 file failed:%s", filename);
		}
	}
	else
	{
		int ret = access(filename, F_OK);
		if ( ret!= 0)
		{
			if ((file_id = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)) < 0)
			{
				CException(E, "create HDF5 file failed:%s", filename);
			}
		}
		else
		{
			if ((file_id = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0)
			{
				CException(E, "open HDF5 file failed:%s", filename);
			}
		}
	}

	if ((root_group_id = H5Gopen(file_id, "/")) < 0)
	{
		CException(E, "open root group failed");
	}

	return root_group_id;
}

void iH5Close()
{
	if (root_group_id > 0)
	{
		if (H5Gclose(root_group_id) < 0)
		{
			CException(E, "close root group failed");
		}
	}

	if (file_id > 0)
	{
		if (H5Fclose(file_id) < 0)
		{
			CException(E, "close HDF5 file failed");
		}
	}

}

void iH5ReadStrAttr(hid_t loc_id, const char *name, char **value, BOOL bDisp)
{
	hid_t type_id, attr_id;
	H5T_class_t class_id;

	if (name == NULL)
	{
		CException(E, "string attribute name is empty", name);		//字符串属性名称为空
	}

	if ((attr_id = H5Aopen_name(loc_id, name)) < 0)
	{
		CException(E, "open string attribute failed", name);	//打开字符串属性失败
	}

	if ((type_id = H5Aget_type(attr_id)) < 0)
	{
		CException(E, "get string attribute type failed", name);	//取得字符串属性类型失败
	}

	if ((class_id = H5Tget_class(type_id)) < 0)
	{
		CException(E, "get class type failed", name);		//H5Tget_class()失败
	}

	if (class_id == H5T_STRING)
	{
		int n = (int)H5Tget_size(type_id);
		if ((*value = (char*)malloc((n + 1)*sizeof(char))) == NULL)
		{
			CException(E, "memory alloc failed");	//内存分配错误
		}
		H5Aread(attr_id, type_id, *value);
		(*value)[n] = 0;
	}
	else
	{
		CException(E, "datatype wrong");	//数据类型错误
	}

	//显示
	if (bDisp)
	{
		printf("\n\nstring attribute: %s\n", name);
		printf("string value = %s\n\n",*value);
	}
}

void iH5WriteStrAttr(hid_t loc_id, const char* name, char* value, BOOL bDisp)
{
	hid_t space_id, type_id, attr_id;

	if (name == NULL)
	{
		CException(E, "attribute name is empty"); //属性名称为空
	}

	if (value == NULL)
	{
		CException(E, "attribute value is null");//属性值为空
	}

	if (strlen(value) <= 0)
	{
		CException(W, "size must be positive, [%s]\n", name);
	}

	space_id = H5Screate(H5S_SCALAR);

	type_id = H5Tcopy(H5T_C_S1);
			  H5Tset_size(type_id, strlen(value));
			  H5Tset_strpad(type_id,H5T_STR_NULLTERM);

	if ((attr_id = H5Acreate(loc_id, name, type_id, space_id, H5P_DEFAULT)) < 0)
	{
		CException(E, "create string attribute failed", name); //创建字符串属性失败
	}

	if (H5Awrite(attr_id, type_id, value) < 0)
	{
		CException(E, "write string attribute failed", name); //写字符串属性失败
	}

	H5Sclose(space_id);
	H5Tclose(type_id);
	H5Aclose(attr_id);

	// 显示
	if(bDisp)
	{
		printf("\n\nstring attribute: %s\n", name);
		printf("string value = %s\n\n", value);
	}
}

void iH5ReadVLStrAttr(hid_t loc_id, const char *name, int rank, hsize_t *dims,
		char **data, BOOL bDisp)
{
	hid_t space_id, attr_id, type_id;

	if (name == NULL)
	{
		CException(E, "attribute name is empty");//属性名称为空
	}

	if ((attr_id = H5Aopen_name(loc_id, name)) < 0)
	{
		CException(E, "open attribute failed", name);//打开属性失败
	}

	if ((type_id = H5Aget_type(attr_id)) < 0)
	{
		CException(E, "get string attribute type failed", name);//取得字符串属性类型失败
	}

	if ((space_id = H5Aget_space(attr_id)) < 0)
	{
		CException(E, "get data space size failed", name);//取得数据空间失败
	}

	if ((rank = H5Sget_simple_extent_ndims(space_id)) < 0)
	{
		CException(E, "get data space dimension failed", name);//取得数据空间维度失败
	}

	if ((dims = malloc(rank*sizeof(hsize_t))) == NULL)
	{
		CException(E, "memory alloc failed", name);//内存分配错误
	}

	if (H5Sget_simple_extent_dims(space_id, dims, NULL) < 0)
	{
		free(dims);
		dims = NULL;
		CException(E, "get dimension failed", name);//取得各个维度尺寸错误
	}

	*data = char_Memory_Alloc(rank, dims, NULL, data);
	if (*data == NULL)
	{
		free(dims);
		dims = NULL;
		CException(E, "memory alloc failed", name);//内存分配错误
	}

	if (H5Aread(attr_id, type_id, *data) < 0)
	{
		free(dims);
		free(data);
		dims = NULL;
		*data = NULL;
		CException(E, "read attribute failed", name);//读取属性失败
	}

	H5Sclose(space_id);
	H5Aclose(attr_id);

	//disp
	if (bDisp)
	{
		int i, n = (int)(*dims);
		printf("\n\nstring attribute: %s\n", name);
		printf("rank = %d\ndimension = %d", rank, (int)(*dims));
		for(i = 1; i < rank; i++)
		{
			printf(" x %d", (int)(*dims+i));
			n *= (int)(*dims+i);
		}
		printf("\ndata:\n");

		for(hsize_t i = 0; i < *dims; i++)
		{
			printf("%s \n", *data+i);
		}
		printf("\n\n");
	}
}

void iH5WriteVLStrAttr(hid_t loc_id, const char *name, int rank, hsize_t *dims,
		char *data[], BOOL bDisp)
{
	hid_t space_id, type_id, attr_id;

	if (name == NULL)
	{
		CException(E, "attribute name is empty"); //属性名称为空
	}

	if ((space_id = H5Screate_simple(rank, dims, NULL)) < 0)
	{
		CException(E, "create dataset space failed", name);//创建数据集空间失败
	}

	if ((type_id = H5Tcopy(H5T_C_S1)) < 0)
	{
		CException(E, "copy datatype failed", name);//复制数据类型失败
	}

	if (H5Tset_size(type_id, H5T_VARIABLE) < 0)
	{
		CException(E, "set data type size failed", name);//设置数据类型长度：变长
	}

	if (H5Tset_strpad(type_id,H5T_STR_NULLTERM) < 0)
	{
		CException(E, "set string end failed", name);//字符串以NULL结尾
	}

	if ((attr_id = H5Acreate(loc_id, name, type_id, space_id, H5P_DEFAULT)) < 0)
	{
		CException(E, "create string attribute failed", name); //创建字符串属性失败
	}

	if (H5Awrite(attr_id, type_id, data) < 0)
	{
		CException(E, "write string attribute failed", name); //写字符串属性失败
	}

	H5Sclose(space_id);
	H5Tclose(type_id);
	H5Aclose(attr_id);

	// 显示
	if(bDisp)
	{
		int i, n = (int)(*dims);
		printf("\n\nstring attribute: %s\n", name);
		printf("rank = %d\ndimension = %d", rank, (int)(*dims));
		for(i = 1; i < rank; i++)
		{
			printf(" x %d", (int)(*dims+i));
			n *= (int)(*dims+i);
		}
		printf("\ndata:\n");

		for(hsize_t i = 0; i < *dims; i++)
		{
			printf("%s \n", *data+i);
		}
		printf("\n\n");
	}
}

void iH5WriteVLStrSDS(hid_t loc_id, const char *name, int rank, hsize_t *dims,
		char *data[], BOOL bDisp)
{
	hid_t space_id, type_id, dataset_id, props_id;

	if (name == NULL)
	{
		CException(E, "attribute name is empty"); //属性名称为空
	}

	if ((space_id = H5Screate_simple(rank, dims, NULL)) < 0)
	{
		CException(E, "create dataset space failed", name);//创建数据集空间失败
	}

	if ((type_id = H5Tcopy(H5T_C_S1)) < 0)
	{
		CException(E, "copy datatype failed", name);//复制数据类型失败
	}

	if (H5Tset_size(type_id, H5T_VARIABLE) < 0)
	{
		CException(E, "H5Tset_size() failed", name);//设置数据类型长度：变长
	}

	if (H5Tset_strpad(type_id,H5T_STR_NULLTERM) < 0)
	{
		CException(E, "H5Tset_strpad() failed", name);//字符串以NULL结尾
	}

	if ((props_id = H5Pcreate(H5P_DATASET_CREATE)) < 0)
	{
		CException(E, "create datatype properties failed", name);//创建数据类型属性
	}

	if ((dataset_id = H5Dcreate(loc_id, name, type_id, space_id, props_id)) < 0)
	{
		CException(E, "create dataset failed", name);//创建数据集失败
	}

	if ((H5Dwrite(dataset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, data)) < 0)
	{
		CException(E, "write dataset failed", name);//写数据集失败
	}

	H5Dclose(dataset_id);
	H5Tclose(type_id);
	H5Pclose(props_id);
	H5Sclose(space_id);

	// 是否显示
	if(bDisp)
	{
		int i, n = (int)(*dims);
		printf("\n\nstring attribute: %s\n", name);
		printf("rank = %d\ndimension = %d", rank, (int)(*dims));
		for(i = 1; i < rank; i++)
		{
			printf(" x %d", (int)(*dims+i));
			n *= (int)(*dims+i);
		}
		printf("\ndata:\n");

		for(hsize_t i = 0; i < *dims; i++)
		{
			printf("%s \n", *data+i);
		}
		printf("\n\n");
	}
}

