#include "HDFIOStructured.h"

using namespace std;


HDFIOStructured::HDFIOStructured()
{
	cdtype_info.cd_instance=0;
	file_id=-1;
}

HDFIOStructured::~HDFIOStructured()
{
	if(cdtype_info.cd_instance!=0)
	{
		char** f_name=const_cast<char**>(cdtype_info.field_name);
		delete [] f_name;
		delete [] cdtype_info.field_offset;
		delete [] cdtype_info.field_size;
		delete [] cdtype_info.field_type;
	}
	if(dset_id>0)
		H5Dclose(dset_id);

}
int HDFIOStructured::initHDFIOStructured(const char* file_name)
{
	fileName = file_name;
	fstream fl;
	fl.open(fileName.c_str(),ios_base::in);
	
	if(fl.is_open())
	{
		cout<<"HDFÎÄ¼þÃû£º"<<fileName.c_str()<<endl;
		fl.close();
		if((file_id=H5Fopen(fileName.c_str(), H5F_ACC_RDWR, H5P_DEFAULT ))<0)
		{
			cerr<<"´íÎó:HDFÎÄ¼þ²»´æÔÚ!"<<endl;
			return ABNORMAL;
		}
		isFirst=false;
	}
	else
	{
		cout<<"HDFÎÄ¼þÃû£º"<<fileName.c_str()<<endl;
		if((file_id=H5Fcreate(fileName.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT ))<0)
		{
			cerr<<"´íÎó£ºÉú³ÉHDFÎÄ¼þÊ§°Ü!"<<endl;
			return ABNORMAL;
		}
		isFirst=true;
	}

	dset_id=-1;
	return NORMAL;
}

void HDFIOStructured::closeFile()
{
	if(file_id>0)
		H5Fclose(file_id);
}

int HDFIOStructured::initFile()
{
	hid_t grp_id;
	
	if(file_id<0)
	{
		cerr<<"´íÎó:²úÉú·Ç·¨HDFFileID!"<<endl;
		return ABNORMAL;
	}
	
	if(isTable)
		grp_id=H5Gcreate(file_id, "GROUP_TABLE",0);
	else
	if(GROUP_SDS != "/")
		grp_id=H5Gcreate(file_id, "GROUP_SDS",0);
	
	if(grp_id<0)
	{
		cerr<<"´íÎó£º·Ç·¨HDF5GroupID!"<<endl;
		H5Gclose(grp_id);
		return ABNORMAL;
	}
	H5Gclose(grp_id);
	return NORMAL;

}

int HDFIOStructured::DeleteSDS(const char* dset_name)
{
	hid_t grp_id;
	
	if(file_id<0)
	{
		cerr<<"´íÎó:·Ç·¨FileID!"<<endl;
		return ABNORMAL;
	}
	
	if((grp_id=H5Gopen(file_id,"GROUP_SDS"))<0)
	{
		cerr<<"´íÎó:·Ç·¨GroupID!"<<endl;
		return ABNORMAL;
	}
	
	if((status=H5Gunlink(grp_id,dset_name))<0)
	{
		cerr<<"´íÎó:GroupID²»°üº¬´ËSDS!"<<endl;
		H5Gclose(grp_id);
		return ABNORMAL;
	}
	
	return NORMAL;
}

int HDFIOStructured::initSDSEX(const char* dset_name,int rank,const hsize_t* dims,hid_t type_id,int dim_ex)
{
	int index;
	hid_t grp_id=-1;
	hid_t spc_id=-1;
	hid_t para_id=-1;
	hsize_t* psize=NULL,*pmsize,*pchunk;
	
	if(file_id<0)
	{
		cerr<<"´íÎó:·Ç·¨FileID!"<<endl;
		return ABNORMAL;
	}
	
	if(isFirst)
	{
		isTable=false;
		isFirst=false;
		status= initFile();
		if(status<0)
			return ABNORMAL;
	}
	if(dset_id>0)
		H5Dclose(dset_id);
	
	try
	{
		if(dset_name==NULL)
			throw "´íÎó£ºSDSÃûÎª¿Õ!"  ;
		if((grp_id=H5Gopen(file_id,GROUP_SDS))<0)
			throw "´íÎó:´ò¿ªHDFÎÄ¼þÊ§°Ü!" ;
		if(H5LTfind_dataset(grp_id,dset_name)==1)
		{
			return NORMAL;
		}
		
		if(type_id<=0 || rank<=0 || dims==NULL || dim_ex<=0 || dim_ex>rank)
			throw "´íÎó£º·Ç·¨Êý¾ÝÀàÐÍ»òÎ¬¶È!"  ;
		
		psize=new hsize_t[rank];
		pmsize=new hsize_t[rank];
		pchunk=new hsize_t[rank];
		for(index=0;index<rank;index++)
			pchunk[index]=pmsize[index]=psize[index]=dims[index];
		psize[dim_ex-1]=0;
		pchunk[dim_ex-1]=100;
		pmsize[dim_ex-1]=H5S_UNLIMITED;
		if((spc_id=H5Screate_simple(rank,psize,pmsize))<0)
			throw "´íÎó:ÉêÇëÄÚ´æÊ§°Ü!"  ;
			
		if((para_id=H5Pcreate(H5P_DATASET_CREATE))<0)
			throw "´íÎó:Éú³ÉSDSÊ§°Ü!"  ;
		status=H5Pset_chunk(para_id,rank,pchunk);
		if(status<0)
			throw "´íÎó!"  ;
		if((dset_id=H5Dcreate(grp_id,dset_name,type_id,spc_id,para_id))<0)
			throw "´íÎó:Éú³ÉSDSÊ§°Ü!"  ;
		delete [] psize;
		delete [] pmsize;
		delete [] pchunk;
		H5Gclose(grp_id);
		H5Sclose(spc_id);
		H5Pclose(para_id);
	}
	catch(const char* msg)
	{
		cerr<<msg<<endl;
		if(psize!=NULL)
		{
			delete [] psize;
			delete [] pmsize;
			delete [] pchunk;
		}
		if(grp_id>0)
			H5Gclose(grp_id);
		if(spc_id>0)
			H5Sclose(spc_id);
		if(para_id>0)
			H5Pclose(para_id);

		return ABNORMAL;
	}

	return NORMAL;
}

int HDFIOStructured::connSDSEX(const char* dset_name)
{
	hid_t grp_id=-1;
	
	if(file_id<0)
	{
		cerr<<"´íÎó:·Ç·¨FileID!"<<endl;
		return ABNORMAL;
	}
	
	if(isFirst)
	{
		isTable=false;
		isFirst=false;
		status= initFile();
		if(status<0)
			return ABNORMAL;
	}
	if(dset_id>0)
		H5Dclose(dset_id);
	dsetname=dset_name;
	
	try
	{
		if(dset_name==NULL)
			throw "´íÎó£ºSDSÎª¿Õ!"  ;
		if((grp_id=H5Gopen(file_id,GROUP_SDS))<0)
			throw "´íÎó:´ò¿ªGroupIDÊ§°Ü!" ;
		if(H5LTfind_dataset(grp_id,dset_name)!=1)
			throw "´íÎó:ÎÄ¼þÖÐ²»°üº¬´ËSDS!" ;
		
		if((dset_id=H5Dopen(grp_id,dset_name))<0)
			throw "´íÎó:´ò¿ªSDSÊ§°Ü!";
		H5Gclose(grp_id);
	}
	catch(const char* msg)
	{
		cerr<<msg<<endl;
		if(grp_id>0)
			H5Gclose(grp_id);

		return ABNORMAL;
	}

	return NORMAL;
}

int HDFIOStructured::WriteSDSToHDF(const char* dset_name,int rank,const hsize_t* dims,hid_t type_id,const void* buf)
{
	hid_t grp_id=-1,spc_id=-1,spc_id2=-1;
	
	if(file_id<0)
	{
		cerr<<"´íÎó£º·Ç·¨FileID!"<<endl;
		return ABNORMAL;
	}
	
	if(isFirst)
	{
		isTable=false;
		isFirst=false;
		status= initFile();
		if(status<0)
			return ABNORMAL;
	}
	
	try
	{
		if((grp_id=H5Gopen(file_id,GROUP_SDS))<0)
			throw "´íÎó:´ò¿ªGroupIDÊ§°Ü!" ;
		if((spc_id2=H5Screate_simple(rank,dims,dims))<0)
			throw "´íÎó:ÉêÇëÄÚ´æÊ§°Ü!" ;
		if(H5LTfind_dataset(grp_id,dset_name)==1)
		{
			if((dset_id=H5Dopen(grp_id,dset_name))<0)
				throw "´íÎó:´ò¿ªSDSÊ§°Ü!" ;
			if((spc_id=H5Dget_space(dset_id))<0)
				throw "´íÎó:´ò¿ªSDSÊ§°Ü!" ;
		}
		else
		{
			spc_id=spc_id2;
			if((dset_id=H5Dcreate(grp_id,dset_name,type_id,spc_id2,H5P_DEFAULT))<0)
				throw "´íÎó:Éú³ÉSDSÊ§°Ü!";
		}
		if((status=H5Dwrite(dset_id,type_id,spc_id2,spc_id,H5P_DEFAULT,buf))<0)
			throw "´íÎó:Êä³öSDSÊ§°Ü!";
		H5Gclose(grp_id);
		H5Sclose(spc_id2);
		if(spc_id!=spc_id2)
			H5Sclose(spc_id);
	}
	catch(const char* msg)
	{
		if(grp_id>0)
			H5Gclose(grp_id);
		if(spc_id>0 && spc_id2!=spc_id)
			H5Sclose(spc_id);
		if(spc_id2>0)
			H5Sclose(spc_id2);
		return ABNORMAL;
	}

	return NORMAL;

}

int HDFIOStructured::WriteSDSToHDFEX(const void* buf,hsize_t data_num,hsize_t* data_begin)
{
	int index,rank,dim_ex=-1;
	hid_t type_id=-1,spc_id=-1,spc_id2=-1;
	hsize_t* dims=NULL,*mdims=NULL,*start,*count;

	try
	{
		  if(data_begin==NULL)
			 throw "´íÎó:Êý¾ÝÍ·Îª¿Õ!" ;
		  if((type_id=H5Dget_type(dset_id))<0)
		   throw "´íÎó:Êý¾ÝÀàÐÍ·Ç·¨!";
	    if((spc_id=H5Dget_space(dset_id))<0)
		   throw "´íÎó:ÉêÇë¿Õ¼äÊ§°Ü!";
	    if((rank=H5Sget_simple_extent_ndims(spc_id))<0)
		   throw "´íÎó:»ñÈ¡Î¬¶ÈÊ§°Ü!";
	
	    dims=new hsize_t[rank];
	    mdims=new hsize_t[rank];
	    start=dims;
	    count=mdims;
	    if((status=H5Sget_simple_extent_dims(spc_id,dims,mdims))<0)
		   throw "´íÎó:»ñÈ¡×´Ì¬Ê§°Ü!";
	    for(index=0;index<rank;index++)
	    {
		    if(mdims[index]==H5S_UNLIMITED)
			    dim_ex=index;
		    else
			    dims[index]=0;
	    }
	    if(dim_ex==-1)
	     throw "´íÎó!";
	    if(*data_begin+data_num>dims[dim_ex])
	    {
	     mdims[dim_ex]=*data_begin+data_num;
	     if((status=H5Dextend(dset_id,mdims))<0)
	      throw "´íÎó!";
	    }
	    start[dim_ex]=*data_begin;
	    count[dim_ex]=data_num;
	    
	    spc_id=H5Dget_space(dset_id);
	    status=H5Sselect_hyperslab(spc_id, H5S_SELECT_SET, start, NULL, count, NULL);

	    if((spc_id2=H5Screate_simple(rank,count,count))<0)
	     throw "´íÎó!";
	    if((status=H5Dwrite(dset_id,type_id,spc_id2,spc_id,H5P_DEFAULT,buf))<0)
	     throw "´íÎó!";
	    *data_begin+=data_num;
		delete [] dims;
		delete [] mdims;
		H5Tclose(type_id);
		H5Sclose(spc_id);
		H5Sclose(spc_id2);
	}
	catch(const char* msg)
	{
		cerr<<msg<<endl;
		if(dims!=NULL)
		 delete [] dims;
		if(mdims!=NULL)
	   delete [] mdims;
		if(type_id>0)
			H5Tclose(type_id);
		if(spc_id>0)
			H5Sclose(spc_id);
		if(spc_id2>0)
			H5Sclose(spc_id2);
		return ABNORMAL;
	}

	return NORMAL;
}

int HDFIOStructured::ReadSDSFromHDFEX(void* buf,hsize_t data_num,hsize_t data_begin,int dim_main)
{
	int rank,index,dim_ex;
	hsize_t* dims=NULL,*mdims=NULL,*start,*count;
	hid_t spc_id=-1,type_id=-1,spc_id2=-1;
	
	try
	{
		if((type_id=H5Dget_type(dset_id))<0)
		 throw "´íÎó:»ñÈ¡Êý¾ÝÀàÐÍÊ§°Ü!";
	  if((spc_id=H5Dget_space(dset_id))<0)
		 throw "´íÎó:»ñÈ¡¿Õ¼äÊ§°Ü!";
	  if((rank=H5Sget_simple_extent_ndims(spc_id))<0)
		  throw "´íÎó:»ñÈ¡Î¬¶ÈÊ§°Ü!";
	
	  dims=new hsize_t[rank];
	  mdims=new hsize_t[rank];
	  start=dims;
	  count=mdims;
	  if((status=H5Sget_simple_extent_dims(spc_id,dims,mdims))<0)
		  throw "´íÎó:»ñÈ¡×´Ì¬Ê§°Ü!";
	  for(index=0;index<rank;index++)
	  {
		  if(mdims[index]==H5S_UNLIMITED)
			  dim_ex=index;
		  else
			  dims[index]=0;
	  }
	  if(dim_ex==-1)
	  {
	    if(dim_main<=0 && dim_main>=rank)
	    	throw "´íÎó:·Ç·¨Î¬¶È!" ;
	    start[dim_main-1]=data_begin;
	    if((count[dim_main-1]-=data_begin)>data_num)
	    	count[dim_main-1]=data_num;
	  }
	  else
	  {
		  if((count[dim_ex]=dims[dim_ex]-data_begin)>data_num)
			  count[dim_ex]=data_num;
		  start[dim_ex]=data_begin;
	  }
	    
	  status=H5Sselect_hyperslab(spc_id, H5S_SELECT_SET, start, NULL, count, NULL);
	  if((spc_id2=H5Screate_simple(rank,count,count))<0)
	   throw "´íÎó:ÉêÇë¿Õ¼äÊ§°Ü!";
	  if((status=H5Dread(dset_id,type_id,spc_id2,spc_id,H5P_DEFAULT,buf))<0)
	   throw "´íÎó:¶ÁSDSÊ§°Ü!";
		delete [] dims;
		delete [] mdims;
		H5Tclose(type_id);
		H5Sclose(spc_id);
		H5Sclose(spc_id2);
	}
	catch(const char* msg)
	{
		cerr<<msg<<endl;
		if(dims!=NULL)
		{
			delete [] dims;
			delete [] mdims;
		}
		if(type_id>0)
			H5Tclose(type_id);
		if(spc_id>0)
			H5Sclose(spc_id);
		if(spc_id2>0)
			H5Sclose(spc_id2);

		return ABNORMAL;
	}
	return NORMAL;
}

int HDFIOStructured::ReadSDSFromHDF(const char* dset_name,hid_t type_id,void* buf)
{
	hid_t grp_id;
	
	if(file_id<0)
	{
		cerr<<"´íÎó:·Ç·¨FileID!"<<endl;
		return ABNORMAL;
	}
	
	if(isFirst)
	{
		isTable=false;
		isFirst=false;
		status= initFile();
		if(status<0)
			return ABNORMAL;
	}

	if((grp_id=H5Gopen(file_id,GROUP_SDS))<0)
	{
		return ABNORMAL;
	}
	if(H5LTfind_dataset(grp_id,dset_name)!=1)
	{
		return ABNORMAL;
	}
	
	status=H5LTread_dataset(grp_id,dset_name,type_id,buf);
	
	if(status<0)
	{
		return ABNORMAL;
	}
	H5Gclose(grp_id);

	return NORMAL;
	
}

int HDFIOStructured::WriteGlbAttrToHDF(const char* attr_name,hid_t type_id,const void* buf,hsize_t size)
{
	
	
	hid_t dSpace_id=-1;
	hid_t attr_id=-1;
	hid_t obj_id=-1;
	hsize_t dims[1];
	
	if(file_id<0)
	{
		cerr<<"´íÎó:·Ç·¨FileID!"<<endl;
		return ABNORMAL;
	}
	
	dims[0]=size;
	if(isFirst)
	{
		isTable=false;
		isFirst=false;
		status= initFile();
		if(status<0)
			return ABNORMAL;
	}
	try
	{
		if(type_id == H5T_C_S1)
		{
			H5LTset_attribute_string( file_id,"/",attr_name,(const char*)buf);	
		}
		else
		{
			dSpace_id=H5Screate_simple(1,dims,dims);
			if(dSpace_id<0)
				throw "´íÎó:ÉêÇë¿Õ¼äÊ§°Ü!" ;
			obj_id=H5Gopen(file_id,"/");
			if(obj_id<0)
				throw "´íÎó:´ò¿ªÎÄ¼þÊ§°Ü!" ;
			if((attr_id=H5Aopen_name(obj_id,attr_name))<=0)
			{
				if((attr_id=H5Acreate(obj_id,attr_name,type_id,dSpace_id,H5P_DEFAULT))<0)
					throw "´íÎó:Éú³ÉSDSÊôÐÔÊ§°Ü!" ;
			}
			if((status=H5Awrite(attr_id,type_id,buf))<0)
				throw "´íÎó:Ð´ÈëSDSÊôÐÔÊ§°Ü!" ;
			H5Gclose(obj_id);
			H5Sclose(dSpace_id);
			H5Aclose(attr_id);
		}
	}
	catch(const char* msg)
	{
		cerr<<msg<<endl;
		if(obj_id>0)
			H5Gclose(obj_id);
		if(dSpace_id>0)
			H5Sclose(dSpace_id);
		if(attr_id>0)
			H5Aclose(attr_id);
		return ABNORMAL;
	}

	return NORMAL;
}

int HDFIOStructured::WritePrtAttrToHDF(const char* attr_name,hid_t type_id,const void* buf,hsize_t size)
{
	hid_t dSpace_id=-1;
	hid_t attr_id=-1;
	hid_t obj_id=-1;
	hsize_t dims[1];
	
	if(file_id<0)
	{
		return ABNORMAL;
	}
	
	dims[0]=size;
	if(isFirst)
	{
		isTable=false;
		isFirst=false;
		status= initFile();
		if(status<0)
			return ABNORMAL;
	}
	try
	{
		if(type_id == H5T_C_S1)
		{
			H5LTset_attribute_string( file_id,"/",attr_name,(const char*)buf);	
		}
		else
		{
			dSpace_id=H5Screate_simple(1,dims,dims);
			if(dSpace_id<0)
				throw "´íÎó:ÉêÇë¿Õ¼äÊ§°Ü!" ;

			obj_id=H5Gopen(file_id,GROUP_SDS);
			if(obj_id<0)
				throw "´íÎó:´ò¿ªÎÄ¼þÊ§°Ü!" ;
			if(H5LT_find_attribute(obj_id,attr_name)==1)
			{
				if((status=H5Adelete(obj_id,attr_name))<0)
					throw "´íÎó:É¾³ýÊôÐÔÊ§°Ü!" ;
			}
			if((attr_id=H5Acreate(obj_id,attr_name,type_id,dSpace_id,H5P_DEFAULT))<0)
				throw "´íÎó:ÉêÇëÊôÐÔ¿Õ¼äÊ§°Ü!" ;
			if((status=H5Awrite(attr_id,type_id,buf))<0)
				throw "´íÎó:Ð´ÊôÐÔÊ§°Ü!" ;
			H5Gclose(obj_id);
			H5Sclose(dSpace_id);
			H5Aclose(attr_id);
		}
	}
	catch(const char* msg)
	{
		cerr<<msg<<endl;
		if(obj_id>0)
			H5Gclose(obj_id);
		if(dSpace_id>0)
			H5Sclose(dSpace_id);
		if(attr_id>0)
			H5Aclose(attr_id);
		return ABNORMAL;
	}

	return NORMAL;
}

int HDFIOStructured::WritePrtAttrToHDF(const char* attr_name,hid_t type_id,const void* buf,int rank,hsize_t* dims)
{
	hid_t dSpace_id=-1;
	hid_t attr_id=-1;
	hid_t obj_id=-1;

	if(file_id<0)
	{
		return ABNORMAL;
	}
	
	if(isFirst)
	{
		isTable=false;
		isFirst=false;
		status= initFile();
		if(status<0)
			return ABNORMAL;
	}
	
	try
	{
		if(type_id == H5T_C_S1)
		{
			H5LTset_attribute_string( file_id,"/",attr_name,(const char*)buf);	
		}
		else
		{
			dSpace_id=H5Screate_simple(rank,dims,dims);
			if(dSpace_id<0)
				throw "´íÎó:ÉêÇë¿Õ¼äÊ§°Ü!" ;
			obj_id=H5Gopen(file_id,GROUP_SDS);
			if(obj_id<0)
				throw "´íÎó:´ò¿ªSDSÊ§°Ü!" ;
			if(H5LT_find_attribute(obj_id,attr_name)==1)
			{
				if((status=H5Adelete(obj_id,attr_name))<0)
					throw "´íÎó:É¾³ýSDSÊ§°Ü!" ;
			}
			if((attr_id=H5Acreate(obj_id,attr_name,type_id,dSpace_id,H5P_DEFAULT))<0)
				throw "Ê§°Ü:Éú³ÉSDSÊ§°Ü!" ;
			if((status=H5Awrite(attr_id,type_id,buf))<0)
				throw "Ê§°Ü:Ð´SDSÊ§°Ü!" ;
			H5Gclose(obj_id);
			H5Sclose(dSpace_id);
			H5Aclose(attr_id);
		}
	}
	catch(const char* msg)
	{
		cerr<<msg<<endl;
		if(obj_id>0)
			H5Gclose(obj_id);
		if(dSpace_id>0)
			H5Sclose(dSpace_id);
		if(attr_id>0)
			H5Aclose(attr_id);
		return ABNORMAL;
	}
	return NORMAL;
}

int HDFIOStructured::WriteSDSAttrToHDF(const char* attr_name,hid_t type_id,const void* buf,hsize_t size,const char* dset_name)
{
	hid_t dSpace_id=-1;
	hid_t attr_id=-1;
	hid_t obj_id=-1;
	hid_t grp_id=-1;
	hsize_t dims[1];
	
	if(file_id<0)
	{
		return ABNORMAL;
	}
	
	dims[0]=size;
	if(isFirst)
	{
		isTable=false;
		isFirst=false;
		status= initFile();
		if(status<0)
			return ABNORMAL;
	}
	if(dset_name==NULL)
	{
		return ABNORMAL;
	}
	
	try
	{
		if(type_id == H5T_C_S1)
		{
			H5LTset_attribute_string( file_id,dset_name,attr_name,(const char*)buf);	
		}
		else
		{
			dSpace_id=H5Screate_simple(1,dims,dims);
			if(dSpace_id<0)
				throw "Ê§°Ü:ÉêÇë¿Õ¼äÊ§°Ü!"  ;
			grp_id=H5Gopen(file_id,GROUP_SDS);
			if(grp_id<0)
				throw "´íÎó:Á´½Ó¿Õ¼äÊ§°Ü!" ;
			obj_id=H5Dopen(grp_id,dset_name);
			if(obj_id<0)
				throw "´íÎó:´ò¿ªSDSÊ§°Ü!" ;
			if(H5LT_find_attribute(obj_id,attr_name)==1)
			{
				if((status=H5Adelete(obj_id,attr_name))<0)
					throw "´íÎó:É¾³ýSDSÊ§°Ü!" ;
			}
			if((attr_id=H5Acreate(obj_id,attr_name,type_id,dSpace_id,H5P_DEFAULT))<0)
				throw "´íÎó:Éú³ÉSDSÊ§°Ü!" ;
			if((status=H5Awrite(attr_id,type_id,buf))<0)
				throw "´íÎó:Êä³öSDSÊ§°Ü!" ;
			H5Gclose(grp_id);
			H5Dclose(obj_id);
			H5Aclose(attr_id);
		}
	}
	catch(const char* msg)
	{
		cerr<<msg<<endl;
		if(grp_id>0)
			H5Gclose(grp_id);
		if(obj_id>0)
			H5Dclose(obj_id);
		if(dSpace_id>0)
			H5Sclose(dSpace_id);
		if(attr_id>0)
			H5Aclose(attr_id);
		return ABNORMAL;
	}
	return NORMAL;
}

int HDFIOStructured::ReadGlbAttrFromHDF(const char* attr_name,hid_t type_id,void* buf)
{
	hid_t attr_id;
	hid_t obj_id;

	if(file_id<0)
	{
		return ABNORMAL;
	}
	
	if(isFirst)
	{
		isTable=false;
		isFirst=false;
		status= initFile();
		if(status<0)
			return ABNORMAL;
	}

	obj_id=H5Gopen(file_id,"/");
	if(obj_id<0)
	{
		return ABNORMAL;
	}
	
	if((attr_id=H5Aopen_name(obj_id,attr_name))<0)
	{
		return ABNORMAL;
	}

	if((status=H5Aread(attr_id,type_id,buf))<0)
	{
		return ABNORMAL;
	}
	H5Aclose(attr_id);
	return NORMAL;
}

int HDFIOStructured::ReadPrtAttrFromHDF(const char* attr_name,hid_t type_id,void* buf)
{
	hid_t attr_id;
	hid_t obj_id;

	if(file_id<0)
	{
		cerr<<"ERROR:       ,    !"<<endl;
		return ABNORMAL;
	}
	
	if(isFirst)
	{
		isTable=false;
		isFirst=false;
		status= initFile();
		if(status<0)
			return ABNORMAL;
	}

	obj_id=H5Gopen(file_id,GROUP_SDS);
	if(obj_id<0)
	{
		return ABNORMAL;
	}
	
	if((attr_id=H5Aopen_name(obj_id,attr_name))<0)
	{
		return ABNORMAL;
	}

	if((status=H5Aread(attr_id,type_id,buf))<0)
	{
		return ABNORMAL;
	}
	H5Aclose(attr_id);

	return NORMAL;
}

int HDFIOStructured::ReadSDSAttrFromHDF(const char* attr_name,hid_t type_id,void* buf,const char* dset_name)
{
	hid_t grp_id;
	hid_t attr_id;
	hid_t obj_id;

	if(file_id<0)
	{
		return ABNORMAL;
	}
	
	if(isFirst)
	{
		isTable=false;
		isFirst=false;
		status= initFile();
		if(status<0)
			return ABNORMAL;
	}
	if(dset_name==NULL)
	{
		return ABNORMAL;
	}
	
	grp_id=H5Gopen(file_id,GROUP_SDS);
	if(grp_id<0)
	{
		return ABNORMAL;
	}
	obj_id=H5Dopen(grp_id,dset_name);
	if(obj_id<0)
	{
		return ABNORMAL;
	}
	
	if((attr_id=H5Aopen_name(obj_id,attr_name))<0)
	{
		return ABNORMAL;
	}

	if((status=H5Aread(attr_id,type_id,buf))<0)
	{
		return ABNORMAL;
	}
	H5Dclose(obj_id);
	H5Gclose(grp_id);
	H5Aclose(attr_id);
	return NORMAL;
}

int HDFIOStructured::WriteVDataToHDF(const char* table_name,CCompound& cdRef,const void* record,hsize_t record_num)
{
	hid_t cdtype_id=cdRef.ReturnTypeID();
	hid_t obj_id;
	hid_t grp_id=-1;
	unsigned int index=0;
	bool isExist=false;
	
	if(file_id<0)
	{
		return ABNORMAL;
	}
	
	if(isFirst)
	{
		isTable=true;
		isFirst=false;
		status= initFile();
		if(status<0)
			return ABNORMAL;
	}
	else
	{
		if((grp_id=H5Gopen(file_id,"/GROUP_TABLE"))<0)
		{
			return ABNORMAL;
		}
		if(H5LTfind_dataset(grp_id,table_name)==1)
			isExist=true;
	}
	
	if(cdtype_info.cd_instance!=cdRef.instance_id)
	{
		cdtype_info.cd_instance=cdRef.instance_id;
		cdtype_info.members=H5Tget_nmembers(cdtype_id);
		cdtype_info.chunk_size=20;
		cdtype_info.fill_data=NULL;
		cdtype_info.compress=0;
		cdtype_info.type_size=H5Tget_size(cdtype_id);
		cdtype_info.field_offset=new size_t[cdtype_info.members];
		cdtype_info.field_size=new size_t[cdtype_info.members];	
		cdtype_info.field_type=new hid_t[cdtype_info.members];
		cdtype_info.field_name=const_cast<const char**>(new char*[cdtype_info.members]);
		char** f_name=const_cast< char**>(cdtype_info.field_name);
		
		for(;index<cdtype_info.members;index++)
		{
			cdtype_info.field_offset[index]=H5Tget_member_offset(cdtype_id,index);
			obj_id=H5Tget_member_type(cdtype_id,index);
			cdtype_info.field_size[index]=H5Tget_size(obj_id);
			cdtype_info.field_type[index]=obj_id;
			f_name[index]=H5Tget_member_name(cdtype_id,index);
		}
	}

	if(grp_id==-1)
	{
		grp_id=H5Gopen(file_id,"/GROUP_TABLE");
		if(grp_id<0)
		{
			return ABNORMAL;
		}
	}
	if(isExist==false)
		status=H5TBmake_table(table_name,grp_id,table_name,cdtype_info.members,record_num,cdtype_info.type_size,cdtype_info.field_name,
				cdtype_info.field_offset,cdtype_info.field_type,cdtype_info.chunk_size,cdtype_info.fill_data,cdtype_info.compress,record);
	else
		status=H5TBappend_records(grp_id,table_name,record_num,cdtype_info.type_size,cdtype_info.field_offset,cdtype_info.field_size,record);

	if(status<0)
	{
		return ABNORMAL;
	}
	H5Gclose(grp_id);
	return NORMAL;
}

int HDFIOStructured::ReadVDataFromHDF(const char* table_name,CCompound& cdRef,void* record,hsize_t record_num,hsize_t start)
{
	hid_t cdtype_id=cdRef.ReturnTypeID();
	hid_t obj_id;
	hid_t grp_id=-1;
	unsigned int index=0;
	
	if(file_id<0)
	{
		return ABNORMAL;
	}
	
	if(isFirst)
	{
		isTable=true;
		isFirst=false;
		status= initFile();
		if(status<0)
			return ABNORMAL;
	}
	if((grp_id=H5Gopen(file_id,"/GROUP_TABLE"))<0)
	{
		return ABNORMAL;
	}
	if(H5LTfind_dataset(grp_id,table_name)!=1)
	{
		return ABNORMAL;
	}
	
	if(cdtype_info.cd_instance!=cdRef.instance_id)
	{
		cdtype_info.cd_instance=cdRef.instance_id;
		cdtype_info.members=H5Tget_nmembers(cdtype_id);
		cdtype_info.chunk_size=20;
		cdtype_info.fill_data=NULL;
		cdtype_info.compress=0;
		cdtype_info.type_size=H5Tget_size(cdtype_id);
		cdtype_info.field_offset=new size_t[cdtype_info.members];
		cdtype_info.field_size=new size_t[cdtype_info.members];	
		cdtype_info.field_type=new hid_t[cdtype_info.members];
		cdtype_info.field_name=const_cast<const char**>(new char*[cdtype_info.members]);
		char** f_name=const_cast< char**>(cdtype_info.field_name);
		
		for(;index<cdtype_info.members;index++)
		{
			cdtype_info.field_offset[index]=H5Tget_member_offset(cdtype_id,index);
			obj_id=H5Tget_member_type(cdtype_id,index);
			cdtype_info.field_size[index]=H5Tget_size(obj_id);
			cdtype_info.field_type[index]=obj_id;
			f_name[index]=H5Tget_member_name(cdtype_id,index);
		}
	}

	status=H5TBread_records(grp_id,table_name,start,record_num,cdtype_info.type_size,cdtype_info.field_offset,
			cdtype_info.field_size,record);
	if(status<0)
	{
		return ABNORMAL;
	}
	H5Gclose(grp_id);
	return NORMAL;
}

int HDFIOStructured::WriteUnionToHDF(const char* dset_name,int rank,const hsize_t* dims,hid_t type_id,const void* buf,DSET_ATTR* dset_attr)
{
	hsize_t size;
	
	status=WriteSDSToHDF(dset_name,rank,dims,type_id,buf);
	if(status<0)
	{
		return ABNORMAL;
	}
	size=strlen(dset_attr->units);
	status=WriteSDSAttrToHDF("units",H5T_NATIVE_CHAR,dset_attr->units,size,dset_name);
	if(status<0)
	{
		return ABNORMAL;
	}
	status=WriteSDSAttrToHDF("valid range",H5T_NATIVE_USHORT,dset_attr->valid_range,2,dset_name);
	if(status<0)
	{
		return ABNORMAL;
	}
	status=WriteSDSAttrToHDF("Fill Value",H5T_NATIVE_USHORT,&(dset_attr->fill_value),1,dset_name);
	if(status<0)
	{
		return ABNORMAL;
	}
	size=strlen(dset_attr->long_name);
	status=WriteSDSAttrToHDF("long name",H5T_NATIVE_CHAR,dset_attr->long_name,size,dset_name);
	if(status<0)
	{
		return ABNORMAL;
	}
	status=WriteSDSAttrToHDF("Slope",H5T_NATIVE_FLOAT,&(dset_attr->slope),1,dset_name);
	if(status<0)
	{
		return ABNORMAL;
	}
	status=WriteSDSAttrToHDF("Intercept",H5T_NATIVE_FLOAT,&(dset_attr->intercept),1,dset_name);
	if(status<0)
	{
		return ABNORMAL;
	}
	return NORMAL;
}

hsize_t HDFIOStructured::getSDSSize(const char* dset_name)
{
	int rank,index=0;
	hid_t grp_id=-1;
	hid_t dataset_id;
	hid_t spc_id;
	hid_t type_id;
	hsize_t size,*pSize;
	
	if(file_id<0)
	{
		return ABNORMAL;
	}
	
	if((grp_id=H5Gopen(file_id,GROUP_SDS))<0)
	{
		return 0;
	}
	if(H5LTfind_dataset(grp_id,dset_name)!=1)
	{
		return 0;
	}
	if((dataset_id=H5Dopen(grp_id,dset_name))<0)
	{
		return 0;
	}
	spc_id=H5Dget_space(dataset_id);
	type_id=H5Dget_type(dataset_id);
	rank=H5Sget_simple_extent_ndims(spc_id);
	pSize=new hsize_t[rank];
	H5Sget_simple_extent_dims(spc_id,pSize,NULL);
	size=H5Tget_size(type_id);
	
	for(;index<rank;index++)
		size*=pSize[index];
	
	H5Gclose(grp_id);
	H5Dclose(dataset_id);
	H5Tclose(type_id);
	H5Sclose(spc_id);
	return size;
}

int HDFIOStructured::getFrameNum()
{
	hid_t grp_id;
	hid_t attr_id;
	int nscans;
	
	if(file_id<0)
	{
		return 0;
	}
	if((grp_id=H5Gopen(file_id,"/"))<0)
	{
		return 0;
	}
	if((status=H5LT_find_attribute(grp_id,"Number Of Scans"))<0)
	{
		return 0;
	}
	if((attr_id=H5Aopen_name(grp_id,"Number Of Scans"))<0)
	{
		return 0;
	}
	if((status=H5Aread(attr_id,H5T_NATIVE_INT,&nscans))<0)
	{
		return 0;
	}
	
	H5Gclose(grp_id);
	H5Aclose(attr_id);
	return nscans;
}

const char* HDFIOStructured::getFileName()
{
	return fileName.c_str();
}
