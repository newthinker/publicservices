/*
 * Copyright(c)2010, michael
 * All rights reserved
 *
 *  File  name : main.c
 *  Discription:
 *  Created  on: Dec 29, 2010
 *       Author: michael, zuow11@gmail.com
 */

#include "h5Interface.h"

#ifdef DEBUG
int main()
{
	char* filename = "/dps/workdir/HSI/test/46N/440659/HJ1A-HSI-28-64-A1-20101211-L20000440659.H5";
	hid_t rid = iH5Open(filename, TRUE);
	H5T_order_t order = H5T_ORDER_LE;

	// 读取影像属性信息
	hid_t att_id;
	att_id = H5Gopen(rid, "/ImageAttributes");
	if(att_id>0)
	{
		char* cBands;
		iH5ReadStrAttr(att_id, "Bands", &cBands, TRUE);

		sHDFSDS sHdfSDS;

		short_ReadNumAttr(att_id, "ProductLines", sHdfSDS.nRank, &sHdfSDS.pDims,
				H5T_NATIVE_INT, &sHdfSDS.nSize, &sHdfSDS.pData, order, TRUE);
//
//		hdf.ReadNumAttr(att_id, "ProductSamples", nAttribute.nRank, &nAttribute.pDims,
//				H5T_NATIVE_INT, &nAttribute.nSize, &nAttribute.pData, order);
	}
	return 0;
}
#endif
