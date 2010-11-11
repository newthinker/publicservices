/*
 *  Class ParamList is for parsing param file.
 *
 *  Created on: Nov 4, 2010
 *      Author: Michael.Cho
 *      		zuow11@gmail.com
 */

#ifndef PARAMLIST_H_
#define PARAMLIST_H_

#include <stddef.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#define DEBUG

// define the node's item
typedef struct
{
    std::string name ;
    std::string value ;
    std::string comment ;
} PARAMETER_ITEM ;

// define the list's link node
typedef struct p_list PARAMETER_LIST ;
struct p_list
{
    PARAMETER_ITEM item ;
    PARAMETER_LIST *next ;
} ;

class ParamList
{
public:
  ParamList();
  virtual	~ParamList();

  int Remove ( char* name ) ;                     // remove a parameter item from the list
  int MakeList ( char* paramfile ) ;      		// make a parameter list from a parameter file
  int MakeList (std::string paramfile);
  void Output () ;                                        // print a parameter list
  void Clear () ;                                         // remove all the parameter item
  int GetCount();							// get the nodes' count of the list

  /// read in
  int GetValue ( char* name, char* sv ) ;			// get value from an expression
  int GetValue ( char* name, char* sv[], int number ) ;
  int GetValue ( char* name, int* iv, int number = 1) ;
  int GetValue ( char* name, long* lv, int number = 1) ;
  int GetValue ( char* name, float* fv, int number = 1) ;
  int GetValue ( char* name, double* dv, int number = 1) ;

  /// modify
  int SetValue(char* name, char* sv, char* comment) ;		// set value
  template <class T>
  int SetValue(char* name, T* value, int number, char* comment)
  {
	std::stringstream ioStream;
	for(int num=0; num<number; num++)
	{
		if(num==0 && number>1)
			ioStream << '[';

		ioStream << *(value+num);

		if(num==(number-1) && number>1)
			ioStream << ']';
		else if(num!=(number-1) && number>1)
			ioStream << ',';
	}

	std::string strValue = ioStream.str();
	int ret = _reset(name, (char*)strValue.c_str(), comment);
  	if(ret!=0)
  		return -1;

  	return 0;
  }

  int AddValue(char* name, char* sv, char* comment);
  template <class T>
  int AddValue(char* name, T* value, int number, char* comment=NULL)
  {
	std::stringstream ioStream;
	for(int num=0; num<number; num++)
	{
		if(num==0 && number>1)
			ioStream << '[';

		ioStream << *(value+num) << ',';

		if(num==(number-1) && number>1)
			ioStream << ']';
	}

	std::string strValue = ioStream.str();
	int ret = _add(name, strValue.c_str(), comment);
	if(ret!=0)
		return -1;

	return 0;
  }

  /// write out
  int  MakeFile ( char* paramfile );              // make a parameter file from a parameter list

private:
  int _add(char* name, char* value, char* comment ) ;		// add a new item in the link list
  int _add(std::string name, std::string value, std::string comment=NULL);
  int _reset(char* name, char* value, char* comment);		// reset the node with 'name'
  int _reset(std::string name, std::string value, std::string comment=NULL);
  void _search ( char* name, p_list* &ptrNode);	// search the selected node ptr
  p_list* _search (char* name);

  void _sort ( int sortmode ) ;           // item sort: 0 --- ascend, 1 --- descend

private:
  PARAMETER_LIST *head ;	// head of the link
  PARAMETER_LIST *tail ;	// tail of the link
};

#endif /* PARAMLIST_H_ */
