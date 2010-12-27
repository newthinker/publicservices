/*
 * main.cpp
 *
 *  Created on: Dec 19, 2010
 *      Author: michael
 */

#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <vector>
#include <unistd.h>
#include "geotiffio.h"
#include "xtiffio.h"
#include "HdfFile.h"
#include "HdfData.h"

#define FILE_SIZE_LEN 		16    	// 文件内容的长度
#define FILE_TIME_LEN 		64    	// 文件最后修改的时间
#define FILE_NAME_LEN 		256		// 文件名长度

//#define DEBUG

struct ImgInfo
{
	float fPixelSpacing;
	int nBands;
	std::string strProjection;
	std::string strDatum;
	std::string strZone;
	float fImgRangeXY[8];
	float fImgRangeLL[8];
	int nRows;
	int nCols;
	short* pData;

	std::string strFilename;		// 文件名
	std::string strFilepath;		// 文件路径
};

int ParseHDF(std::string file_name, ImgInfo* pImgData);

int OutputBand(ImgInfo* pImgData);

int ExtractFiles(std::string indirect, std::string outdirect);

char* DisplayFileAttr(struct stat file_info,std::string file_name);

int IsDir(std::string file_path);

void List(std::string strPath, std::string strSuffix, std::vector<std::string> &files);

void FillPath(std::string& file_path);

void SetUpTIFFDirectory(TIFF *tif, ImgInfo* pImgData);

void SetUpGeoKeys(GTIF *gtif, ImgInfo* pImgData);

void WriteImage(TIFF *tif, ImgInfo* pImgData, int bandID);

void ClassifyImage(ImgInfo* pImgData, std::string out_path);

int main(int argc,char* argv[])
{
	int ret = 0;
	bool bClassify = true;		// 是否进行分带保存

	// 测试
#ifdef TEST
	std::string filename = "/dps/workdir/HSI/377969/HJ1A-HSI-457-78-A1-20100820-L20000377969.H5";
	std::vector<std::string> filelist = std::vector<std::string>();

	ImgInfo* pImgInfo = new ImgInfo;
	std::string path = "/dps/workdir/HSI/TIF";
	ParseHDF(filename, pImgInfo);

	OutputBand(pImgInfo, path);

	if(pImgInfo!=NULL)
	{
		if(pImgInfo->pData!=NULL)
		{
			delete [] pImgInfo->pData;
			pImgInfo->pData = NULL;
		}

		delete pImgInfo;
		pImgInfo = NULL;
	}
#endif

	////////////////////////////////////////////////////////////
	std::string inPath, outPath;
	int count = 0;
	std::vector<std::string> filelist = std::vector<std::string>();

#ifdef DEBUG	// debug
	inPath = "/dps/workdir/HSI/product";
	outPath = "/dps/workdir/HSI/test";
#else		// release
	if(argc<2)
		return -1;
	inPath = argv[1];
	outPath = argv[2];
#endif

	// 解压压缩包
	ExtractFiles(inPath, outPath);

	// 搜索指定路径并将HDF5文件全路径保存
	filelist.clear();
	std::string suffix = "H5";
	List(outPath, suffix, filelist);
	// 输出搜索到的文件名
	count = filelist.size();
	for(int n=0; n<count; n++)
	{
		std::string fp = filelist[n];
		std::cout << fp << std::endl;
	}

	// 将所搜索到的文件转存为单波段的GeoTIFF文件
	for(int n=0; n<count; n++)
	{
		ImgInfo* pImgInfo = new ImgInfo;
		ret = ParseHDF(filelist[n], pImgInfo);
		if(ret!=0)
		{
			if(pImgInfo!=NULL)
			{
				if(pImgInfo->pData!=NULL)
				{
					delete [] pImgInfo->pData;
					pImgInfo->pData = NULL;
				}

				delete pImgInfo;
				pImgInfo = NULL;
			}
			continue;
		}

		// 输出各波段数据到GeoTiff
		OutputBand(pImgInfo);

		// 将影像数据按带号进行分类
		if(bClassify)
			ClassifyImage(pImgInfo, outPath);

		if(pImgInfo!=NULL)
		{
			if(pImgInfo->pData!=NULL)
			{
				delete [] pImgInfo->pData;
				pImgInfo->pData = NULL;
			}

			delete pImgInfo;
			pImgInfo = NULL;
		}
	}

	return 0;
}

/*
 * 在指定路径下递归搜索给定后缀的文件，然后将文件路径记录下来
 */
void List(std::string file_path, std::string suffix, std::vector<std::string> &files)
{
	DIR *db;              				// 保存 打开目录类型文件信息的 结构体
	std::string filename;        		// 文件名
	struct dirent *dir_info;			// 目录结构体

	if((db=opendir((char*)file_path.c_str()))==NULL)
	{
		perror("opendir error!");
		return;
	}

	while ((dir_info=readdir(db)))
	{
		if(strcmp(dir_info->d_name, ".")==0 ||
			strcmp(dir_info->d_name, "..")==0)
			continue;
		else
		{
			filename = file_path;
			FillPath(filename);
			filename += dir_info->d_name;

			if(IsDir(filename))		// 判断是否为路径
			{
				List(filename, suffix, files);	// 是路径就继续递归查找
			}
			else
			{
				std::string file_suffix;
				file_suffix.clear();
				if(!suffix.empty())		// 如果没有指定后缀，就将所有搜索到的文件全部放入容器中 [ZuoW,2010/12/27]
				{
				   size_t loc = filename.find_last_of('.');
				   file_suffix = filename.substr(loc+1, filename.length()-loc-1);
				}

				if(file_suffix==suffix)
				{
				   files.push_back(filename);
				}
			}
		}
	}

	if(closedir(db)==-1)
	{
        perror("closedir error!");
	}
}


/*
 * 简要描述: 从指定路径下将压缩文件解压到目标路径下
 * 输入参数: indirect-->输入路径
 *         	outdirect-->输出路径
 */
int ExtractFiles(std::string indirect, std::string outdirect)
{
	int ret = 0;
	std::string command;
	std::vector<std::string> files = std::vector<std::string>();

	files.clear();
	std::string suffix = "gz";

	// 判断输入/输出路径是否存在
	if(access(indirect.c_str(), R_OK)==-1)
		return -1;
	if(access(outdirect.c_str(), R_OK||W_OK)==-1)
	{
		command = "mkdir -p ";
		command += outdirect;
		ret = system(command.c_str());
		if(ret==-1)
			return -1;
	}

	// 搜索输入路径下的所有压缩包
	List(indirect, suffix, files);

	// 解压压缩包到指定路径下
	int count = files.size();
	for(int n=count-1; n>=0; n--)
	{
		std::string filepath = files[n];
		files.pop_back();

		command = "tar -zxf ";
		command += filepath;
		command += " -C ";
		command += outdirect;
		system(command.c_str());
	}

	return 0;
}


/*
 * 简要描述：解析HDF文件，并将影像属性数据和波段数据读取出来存放在结构体中
 * 输入参数：file_name-->输入的HDF文件名
 * 输出参数：pImgData-->影像数据结构体
 */
int ParseHDF(std::string file_name, ImgInfo* pImgData)
{
	// HDF解析对象
	CHdfFile hdf(file_name.c_str(), true);
	hid_t rid = hdf.GetRootGroup();
	H5T_order_t order = H5T_ORDER_LE;

	// 读取影像属性信息
	CHdfData<int> nAttribute;
	CHdfData<float> fAttributes;
	CHdfData<short> sBandData;
	hid_t att_id, sds_id;
	att_id = H5Gopen(rid, "/ImageAttributes");
	if(att_id>0)
	{
		char* cBands;
		hdf.ReadStrAttr(att_id, "Bands", &cBands);
		pImgData->nBands = atoi(cBands);

		hdf.ReadNumAttr(att_id, "ProductLines", nAttribute.nRank, &nAttribute.pDims,
				H5T_NATIVE_INT, &nAttribute.nSize, &nAttribute.pData, order);
		pImgData->nRows = *(nAttribute.pData);

		hdf.ReadNumAttr(att_id, "ProductSamples", nAttribute.nRank, &nAttribute.pDims,
				H5T_NATIVE_INT, &nAttribute.nSize, &nAttribute.pData, order);
		pImgData->nCols = *(nAttribute.pData);

	}

	// 读取影像投影信息
	att_id = H5Gopen(rid, "/MapInformation");
	if(att_id>0)
	{
		char* cTemp;
		hdf.ReadStrAttr(att_id, "Projection", &cTemp);
		pImgData->strProjection = cTemp;
		hdf.ReadStrAttr(att_id, "Utm-zone", &cTemp);
		pImgData->strZone = cTemp;
		hdf.ReadStrAttr(att_id, "Datum", &cTemp);
		pImgData->strDatum = cTemp;

		hdf.ReadNumAttr(att_id, "PixelSpacing", fAttributes.nRank, &fAttributes.pDims,
				H5T_NATIVE_FLOAT, &fAttributes.nSize, &fAttributes.pData, order);
		pImgData->fPixelSpacing = *(fAttributes.pData);

		hdf.ReadNumAttr(att_id, "ProductUpperLeftLat", fAttributes.nRank, &fAttributes.pDims,
				H5T_NATIVE_FLOAT, &fAttributes.nSize, &fAttributes.pData, order);
		pImgData->fImgRangeLL[0] = *(fAttributes.pData);
		hdf.ReadNumAttr(att_id, "ProductUpperLeftLong", fAttributes.nRank, &fAttributes.pDims,
				H5T_NATIVE_FLOAT, &fAttributes.nSize, &fAttributes.pData, order);
		pImgData->fImgRangeLL[1] = *(fAttributes.pData);
		hdf.ReadNumAttr(att_id, "ProductUpperRightLat", fAttributes.nRank, &fAttributes.pDims,
				H5T_NATIVE_FLOAT, &fAttributes.nSize, &fAttributes.pData, order);
		pImgData->fImgRangeLL[2] = *(fAttributes.pData);
		hdf.ReadNumAttr(att_id, "ProductUpperRightLong", fAttributes.nRank, &fAttributes.pDims,
				H5T_NATIVE_FLOAT, &fAttributes.nSize, &fAttributes.pData, order);
		pImgData->fImgRangeLL[3] = *(fAttributes.pData);
		hdf.ReadNumAttr(att_id, "ProductLowerRightLat", fAttributes.nRank, &fAttributes.pDims,
				H5T_NATIVE_FLOAT, &fAttributes.nSize, &fAttributes.pData, order);
		pImgData->fImgRangeLL[4] = *(fAttributes.pData);
		hdf.ReadNumAttr(att_id, "ProductLowerRightLong", fAttributes.nRank, &fAttributes.pDims,
				H5T_NATIVE_FLOAT, &fAttributes.nSize, &fAttributes.pData, order);
		pImgData->fImgRangeLL[5] = *(fAttributes.pData);
		hdf.ReadNumAttr(att_id, "ProductLowerLeftLat", fAttributes.nRank, &fAttributes.pDims,
				H5T_NATIVE_FLOAT, &fAttributes.nSize, &fAttributes.pData, order);
		pImgData->fImgRangeLL[6] = *(fAttributes.pData);
		hdf.ReadNumAttr(att_id, "ProductLowerLeftLong", fAttributes.nRank, &fAttributes.pDims,
				H5T_NATIVE_FLOAT, &fAttributes.nSize, &fAttributes.pData, order);
		pImgData->fImgRangeLL[7] = *(fAttributes.pData);

		hdf.ReadNumAttr(att_id, "ProductUpperLeftX", fAttributes.nRank, &fAttributes.pDims,
				H5T_NATIVE_FLOAT, &fAttributes.nSize, &fAttributes.pData, order);
		pImgData->fImgRangeXY[0] = *(fAttributes.pData);
		hdf.ReadNumAttr(att_id, "ProductUpperLeftY", fAttributes.nRank, &fAttributes.pDims,
				H5T_NATIVE_FLOAT, &fAttributes.nSize, &fAttributes.pData, order);
		pImgData->fImgRangeXY[1] = *(fAttributes.pData);
		hdf.ReadNumAttr(att_id, "ProductUpperRightX", fAttributes.nRank, &fAttributes.pDims,
				H5T_NATIVE_FLOAT, &fAttributes.nSize, &fAttributes.pData, order);
		pImgData->fImgRangeXY[2] = *(fAttributes.pData);
		hdf.ReadNumAttr(att_id, "ProductUpperRightY", fAttributes.nRank, &fAttributes.pDims,
				H5T_NATIVE_FLOAT, &fAttributes.nSize, &fAttributes.pData, order);
		pImgData->fImgRangeXY[3] = *(fAttributes.pData);
		hdf.ReadNumAttr(att_id, "ProductLowerRightX", fAttributes.nRank, &fAttributes.pDims,
				H5T_NATIVE_FLOAT, &fAttributes.nSize, &fAttributes.pData, order);
		pImgData->fImgRangeXY[4] = *(fAttributes.pData);
		hdf.ReadNumAttr(att_id, "ProductLowerRightY", fAttributes.nRank, &fAttributes.pDims,
				H5T_NATIVE_FLOAT, &fAttributes.nSize, &fAttributes.pData, order);
		pImgData->fImgRangeXY[5] = *(fAttributes.pData);
		hdf.ReadNumAttr(att_id, "ProductLowerLeftX", fAttributes.nRank, &fAttributes.pDims,
				H5T_NATIVE_FLOAT, &fAttributes.nSize, &fAttributes.pData, order);
		pImgData->fImgRangeXY[6] = *(fAttributes.pData);
		hdf.ReadNumAttr(att_id, "ProductLowerLeftY", fAttributes.nRank, &fAttributes.pDims,
				H5T_NATIVE_FLOAT, &fAttributes.nSize, &fAttributes.pData, order);
		pImgData->fImgRangeXY[7] = *(fAttributes.pData);
	}

	// 读取SDS
	int nLength = 0;
	nLength = pImgData->nBands * pImgData->nCols * pImgData->nRows;
	att_id = H5Gopen(rid, "/ImageData");
	if(att_id>0 && nLength>0)
	{
		pImgData->pData = new short[nLength];
		memset(pImgData->pData, 0, nLength*sizeof(short));

		hdf.ReadSDS(att_id, "BandData", sBandData.nRank, &sBandData.pDims,
			H5T_NATIVE_SHORT, &sBandData.nSize, &sBandData.pData, order);
		//sds attribute
		sds_id = H5Dopen(att_id, "BandData");
//		hdf.ReadStrAttr(sds_id, "Units", &m_Latitude_Unit);//units
//		hdf.ReadNumAttr(sds_id, "ValidRange", m_Latitude_ValidRange.nRank,
//			&m_Latitude_ValidRange.pDims, H5T_NATIVE_DOUBLE, &m_Latitude_ValidRange.nSize,
//			&m_Latitude_ValidRange.pData, order);//valid_range
//		hdf.ReadNumAttr(sds_id, "FillValue", m_Latitude_FillValue.nRank,
//			&m_Latitude_FillValue.pDims, H5T_NATIVE_DOUBLE, &m_Latitude_FillValue.nSize,
//			&m_Latitude_FillValue.pData, order);//_FillValue
//		hdf.ReadStrAttr(sds_id, "LongName", &m_Latitude_LongName);//long_name
		H5Dclose(sds_id);

		memcpy(pImgData->pData, sBandData.pData, nLength*sizeof(short));
	}

	// 解析输出路径
	size_t loc = file_name.find_last_of('/');
	pImgData->strFilepath = file_name.substr(0, loc+1);
	pImgData->strFilename = file_name.substr(loc+1, file_name.length()-loc-1);

	return 0;
}

/*
 * 简要描述：将影像分波段保存位GeoTIFF格式
 * 输入参数：pImgData-->影像数据，包括路径
 * 输出数据：生成GeoTIFF数据
 */
int OutputBand(ImgInfo* pImgData)
{
	std::string filename, filepath, command;

	int bands = pImgData->nBands;
	int cols = pImgData->nCols;
	int rows = pImgData->nRows;
	if(bands*cols*rows<=0 || pImgData->pData==NULL)
		return -1;

	// 获取输出路径和文件前缀
	filepath = pImgData->strFilepath;
	FillPath(filepath);
	size_t loc = pImgData->strFilename.find('.');
	filename = pImgData->strFilename.substr(0, loc);

	// 判断输出路径是否存在
	if((access(filepath.c_str(), W_OK))==-1)
	{
		command = "mkdir -p ";
		command = command + filepath;
		int ret = system(command.c_str());
		if(ret==-1)
			return -1;
	}

	filepath += filename;

	for(int n=0; n<bands; n++)
	{
		// 组织输出文件名
		filename.clear();
		std::stringstream ioStream;
		ioStream << filepath << '-' << n+1 << ".tif";
		filename = ioStream.str();

		// 输出到GeoTiff文件中
		TIFF* tif = (TIFF*)0;
		GTIF* gtif = (GTIF*)0;
		tif = XTIFFOpen((char*)filename.c_str(), "w");
		if(!tif)
		{
			if (tif) TIFFClose(tif);
			if (gtif) GTIFFree(gtif);

			return -1;
		}

		gtif = GTIFNew(tif);
		if(!gtif)
		{
			if (tif) TIFFClose(tif);
			if (gtif) GTIFFree(gtif);

			return -1;
		}

		SetUpTIFFDirectory(tif, pImgData);
		SetUpGeoKeys(gtif, pImgData);
		WriteImage(tif, pImgData, n);

		GTIFWriteKeys(gtif);
		GTIFFree(gtif);
		XTIFFClose(tif);
	}

	return 0;
}

/*
 * 简要描述：判断是否为一个路径
 * 输入参数： file_path-->输入路径名
 * 返回参数：0-->不是路径， 1-->是路径
 */
int IsDir(std::string file_path)
{
	struct stat buf;
	if(lstat(file_path.c_str(), &buf)<0)
	{
	   return 0;
	}
	if(S_ISDIR(buf.st_mode))
	{
	   return 1; //directory
	}

	return 0;
}


/*
 * 简要描述：显示文件属性信息
 * 输入参数：file_name-->输入文件名
 * 输出数据：file_info-->文件信息结构体
 * 返回参数：文件信息字符串
 */
char* DisplayFileAttr(struct stat file_info,std::string file_name)
{
	char *pFile_mtime_size;        		 		/*函数返回值*/
	off_t file_size;           					/*文件的大小 （int）*/
	char size_str[FILE_SIZE_LEN];       		/*文件的大小（字符串）*/
	char time_str[FILE_TIME_LEN];      		/*文件最后一次修改的时间*/
	char file_mtime_size[FILE_TIME_LEN]; 	/*文件最后修改时间和大小的字符串*/

	memset( &file_info,0,sizeof(file_info));   	/*将文件信息结构体置 0 */
	lstat( file_name.c_str(), &file_info );      		/*通过文件路径，把文件信息结构体填满*/

	strftime(time_str, sizeof(time_str), "%Y%m%d%H%M%S",localtime(&file_info.st_mtime)); /*将标准时间格式转成本地时间格式*/
	file_size=file_info.st_size;
	sprintf(size_str,"%d",(int)file_size);
	sprintf(file_mtime_size,"%s*%s",size_str,time_str);
	pFile_mtime_size=file_mtime_size;

	return pFile_mtime_size;
}

/*
 * 简要描述：判断输入路径后面是否有'/'，如果没有就加上
 * 输入参数：file_path-->输入路径
 */
void FillPath(std::string& file_path)
{
	if(IsDir(file_path) && file_path.at(file_path.length()-1)!='/')
		file_path += '/';
}


/*
 * 简要描述：生成TIFF文件的目录
 * 输入参数：tif-->TIFF对象
 * 			pImgData-->影像参数结构体
 */
void SetUpTIFFDirectory(TIFF *tif, ImgInfo* pImgData)
{
	double tiepoints[6] = {0,0,0, pImgData->fImgRangeXY[0], pImgData->fImgRangeXY[1], 0.0};
	double pixscale[3] = {pImgData->fPixelSpacing, pImgData->fPixelSpacing, 0};
	TIFFSetField(tif,TIFFTAG_IMAGEWIDTH,     pImgData->nCols);
	TIFFSetField(tif,TIFFTAG_IMAGELENGTH,    pImgData->nRows);
	TIFFSetField(tif,TIFFTAG_COMPRESSION,    COMPRESSION_LZW);
	TIFFSetField(tif,TIFFTAG_PHOTOMETRIC,    PHOTOMETRIC_MINISBLACK);
	TIFFSetField(tif,TIFFTAG_BITSPERSAMPLE,  16);	// short
	TIFFSetField(tif,TIFFTAG_ROWSPERSTRIP,   1L);
	TIFFSetField(tif,TIFFTAG_GEOTIEPOINTS, 6,tiepoints);
	TIFFSetField(tif,TIFFTAG_GEOPIXELSCALE, 3,pixscale);
}


/*
 * 简要描述：生成GeoTIFF文件的标签
 * 输入参数：gtif-->GeoTIFF对象
 * 			pImgData-->影像参数结构体
 */
void SetUpGeoKeys(GTIF *gtif, ImgInfo* pImgData)
{
	GTIFKeySet(gtif, GTModelTypeGeoKey, TYPE_SHORT, 1, ModelTypeProjected);
	GTIFKeySet(gtif, GTRasterTypeGeoKey, TYPE_SHORT, 1, RasterPixelIsArea);
	GTIFKeySet(gtif, GTCitationGeoKey, TYPE_ASCII, 0, "Corrected Satellite Data");
	GTIFKeySet(gtif, GeographicTypeGeoKey, TYPE_SHORT,   1, KvUserDefined);

	GTIFKeySet(gtif, GeogLinearUnitsGeoKey, TYPE_SHORT,   1, Linear_Meter);
	GTIFKeySet(gtif, GeogGeodeticDatumGeoKey, TYPE_SHORT,      1, KvUserDefined);
	GTIFKeySet(gtif, GeogEllipsoidGeoKey, TYPE_SHORT,      1, Ellipse_WGS_84);

	int value = 0;
	std::string strZone = pImgData->strZone;
	char cHemisphere = strZone.at(strZone.length()-1);
	std::string zonevalue = strZone.substr(0, strZone.length()-1);
	if(cHemisphere=='N')
	{
		value = PCS_WGS84_UTM_zone_1N;
	}
	else
	{
		value = PCS_WGS84_UTM_zone_1S;
	}
	value = value + atoi(zonevalue.c_str()) -1;
	GTIFKeySet(gtif, ProjectedCSTypeGeoKey, TYPE_SHORT, 1, value);
	GTIFKeySet(gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE, 1, 500000.0);
	GTIFKeySet(gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE, 1, 0.0);
	GTIFKeySet(gtif, ProjLinearUnitsGeoKey, TYPE_SHORT, 1, Linear_Meter);
}


/*
 * 简要描述：生成GeoTIFF文件
 * 输入参数：tif-->tif对象
 * 			pImgData-->波段数据
 * 			bandID-->需要输出的波段号
 */
void WriteImage(TIFF *tif, ImgInfo* pImgData, int bandID)
{
	short* buffer = new short[pImgData->nCols];

	for (int i=0;i<pImgData->nRows;i++)
	{
		memset(buffer, 0, pImgData->nCols*sizeof(short));

		int loc = bandID*pImgData->nCols*pImgData->nRows;
		loc += i*pImgData->nCols;
		memcpy(buffer, pImgData->pData+loc, pImgData->nCols*sizeof(short));

		if (!TIFFWriteScanline(tif, buffer, i, 0))
			TIFFError("WriteImage","failure in WriteScanline\n");
	}

	if(buffer!=NULL)
	{
		delete [] buffer;
		buffer = NULL;
	}
}


/*
 * 简要描述：对输入影像数据按分带号进行分类
 * 输入参数：
 */
void ClassifyImage(ImgInfo* pImgData, std::string out_path)
{
	std::string filepath, zone, command;
	int ret = 0;

	filepath = pImgData->strFilepath;
	zone = pImgData->strZone;

	// 判断带号是否合法
	char cHemisphere = zone.at(zone.length()-1);
	std::string temp = zone.substr(0, zone.length()-1);
	int zoneValue = atoi(temp.c_str());
	if(!(cHemisphere=='N' || cHemisphere=='n' || cHemisphere=='S' || cHemisphere=='s')
			|| !(zoneValue>0 && zoneValue<=120))
	{
		zone = "err";
	}

	// 生成输出路径
//	size_t loc = pImgData->strFilepath.find_last_of('/', pImgData->strFilepath.length()-2);
	filepath = out_path;
	FillPath(filepath);
	filepath += zone;

	// 如果分类目录不存在就创建
	if(access(filepath.c_str(), W_OK)==-1)
	{
		command = "mkdir -p ";
		command += filepath;
		ret = system(command.c_str());
		if(ret==-1)
			return;
	}

	// 如果是已经分类的文件就不进行操作 [ZuoW,2010/12/28]
	size_t loc = pImgData->strFilepath.find(filepath);
	if(loc!=std::string::npos)
		return;

	// 将文件夹复制到目标路径下
	// (注：改mv为cp，是因为cp可以覆盖合并，如果前后两次对同一文件进行了不同的操作，可将两次操作结果进行合并) [ZuoW,2010/12/28]
	command = "cp -r ";
	command += pImgData->strFilepath;
	command += "  ";
	command += filepath;
	system(command.c_str());

	// 删除原文件夹
	command = "rm -rf ";
	command += pImgData->strFilepath;
	system(command.c_str());
}

