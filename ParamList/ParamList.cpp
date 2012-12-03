/*
 *
 *  Created on: Nov 4, 2010
 *      Author: aero
 *      		zuow11@gmail.com
 */

//#include <iostream>
#include <string.h>
#include <stdlib.h>
#include "ParamList.h"

ParamList::ParamList()
{
  // TODO Auto-generated constructor stub
  head = NULL;
  tail = NULL;
}

ParamList::~ParamList()
{
  // TODO Auto-generated destructor stub
  if(head!=NULL)
  {
    delete head;
    head = NULL;
  }

  if(tail!=NULL)
  {
    delete tail;
    tail = NULL;
  }
}

int ParamList::MakeList(char* paramfile)
{
  std::ifstream inFile;
  std::string prefix;
  std::string buffer;
  int numNode = 0;      	// count of node
  std::string nodeName, nodeValue, nodeComment;
  bool bArray = false;		// flag of block and array

  inFile.open(paramfile);
  if(!inFile.good())
  {
	  std::cout<<"bad file"<<std::endl;
	  return -1;
  }

  while(!inFile.eof())
  {
	  std::string::size_type loc1, loc2;
	  loc1 = loc2 = 0;
	  char szBuffer[1024] = {0};

	  inFile.getline(szBuffer, 1024);
	  if(szBuffer[0]=='#')
		  continue;

	  buffer += szBuffer;
	  if(!buffer.empty()&&buffer.at(buffer.length()-1)=='\r')
		  buffer.erase(buffer.length()-1, 1);

	  // find char '}'
	  loc1 = buffer.find('}', 0);
	  if(loc1!=std::string::npos)		// over the end of a block
	  {
		  prefix.clear();
		  buffer.clear();
	  }

	  loc1 = buffer.find('{', 0);
	  if(loc1!=std::string::npos)		// get the tag name
	  {
		  loc2 = buffer.find('=', 0);
		  prefix = buffer.substr(0, loc2);
		  buffer = buffer.substr(loc1+1, buffer.length()-loc1-1);

		  if(!nodeName.empty())		// clear the nodeName
			  nodeName.clear();
	  }

	  // get the name, value and comment
	  loc1 = buffer.find('=', 0);
	  if(loc1!=std::string::npos)
	  {
		nodeComment.clear();	// clear the comment

		loc2 = buffer.find('[', 0);
		if(loc2!=std::string::npos)
		{
			bArray = true;
		}

		loc2 = buffer.find(';', 0);
		if(loc2!=std::string::npos)
		{
			// get the node name
			if(!prefix.empty())		// add the prefix
			{
				nodeName = prefix;
				nodeName += '.';
			}
			nodeName += buffer.substr(0, loc1);		// name

			// get the value
			if(bArray)
			{
				loc1 = buffer.find('[', 0);
				loc2 = buffer.find(']', 0);
				if(loc1!=std::string::npos && loc2!=std::string::npos)
					nodeValue = buffer.substr(loc1, loc2-loc1+1);

				bArray = false;
			}
			else
			{
				nodeValue = buffer.substr(loc1+1, loc2-loc1-1);
			}

			// delete the blank in the name
			while(true)
			{
				loc1 = nodeName.find(" ", 0);
				if(loc1!=std::string::npos)
					nodeName.erase(loc1, 1);
				else
					break;
			}

			// get the comment
			loc1 = buffer.find("#", 0);
			if(loc1!=std::string::npos)
			{
				nodeComment += buffer.substr(loc1+1, buffer.length()-loc1-1);
			}

			int ret = _add(nodeName, nodeValue, nodeComment);
			if(ret==0)		// add a node successful
				numNode++;

			nodeName.clear();
			nodeValue.clear();
			nodeComment.clear();
			buffer.clear();
		}
	  }

	  // get the comment in the block and array
	  loc1 = buffer.find('#', 0);
	  if(loc1!=std::string::npos && bArray)
	  {
		  nodeComment += buffer.substr(loc1+1, buffer.length()-loc1-1);
		  nodeComment += '\0';

		  buffer = buffer.substr(0, loc1);
	  }
  }

  inFile.close();

  return 0;
}

int ParamList::MakeList(std::string paramfile)
{
	return MakeList((char*)paramfile.c_str());
}

/*
* delete the node at given name
* interface function [ZuoW,2010/11/4]
*/
int ParamList::Remove(char* name)
{
	// search the list and find the 'name' node
	p_list* curNode = head;
	p_list* preNode = head;

	std::string nodeName = name;
	while(curNode!=NULL)
	{
		int ret = nodeName.compare(curNode->item.name);
		if(ret==0)	// find the node
		{
			preNode->next = curNode->next;		// set it's next as pre's next
			delete curNode;		curNode = NULL;

			return 0;
		}

		preNode = curNode;
		curNode = curNode->next;
	}
	
	return -1;
}

/*
* destroy the list
* interface function [ZuoW,2010/11/4]
*/
void ParamList::Clear()
{
	p_list* curNode = head;
	if(head==NULL)
		return;

	while(head->next!=NULL)
	{
		head->next = curNode->next;
		
		delete curNode;
	}
	
	// delete the head node
	delete head;
	head->next = NULL;
	head = NULL;
	
	return ;
}

/*
* output the list data
* interface function [ZuoW,2010/11/4]
*/
void ParamList::Output()
{
	int count = 0;
	p_list* ptrNode = head->next;
	while(ptrNode)
	{
		std::cout<< ptrNode->item.name << '=' << ptrNode->item.value << std::endl;
		ptrNode = ptrNode->next;
		count++;
	}
	std::cout<<"node count:" << count <<std::endl;

	return;
}

/*
* get the node count of the list
* interface function [ZuoW,2010/11/4]
*/
int ParamList::GetCount()
{
	int count = 0;
	
	p_list* ptrNode = head->next;
	while(ptrNode)
	{
		ptrNode = ptrNode->next;
		count++;
	}
	
	return count;
}

/*
* add a node(name, value) into the list
* member function [ZuoW, 2010/11/4]
*/
int ParamList::_add(char* name, char* value, char* comment)
{
	p_list* ptrNode = new p_list;
	if(ptrNode==NULL)
		return -1;
	
	ptrNode->next = NULL;
	ptrNode->item.name = name;
	ptrNode->item.value = value;
	ptrNode->item.comment = comment;
	
	if(head==NULL)		// when there is no node in the link
	{
		head = new p_list;
		head->next = NULL;
		tail = head;
		
		head->next = ptrNode;
	}
	
	tail->next = ptrNode;		// add current node into the list
	tail = ptrNode;				// set the current node as the end node
	tail->next = NULL;
	
	return 0;
}

int ParamList::_add(std::string name, std::string value, std::string comment)
{
	return( _add((char*)name.c_str(), (char*)value.c_str(), (char*)comment.c_str()) );
}

/*
 * reset the node with 'name'
 * member function [ZuoW,2010/11/11]
 */
int ParamList::_reset(char* name, char* value, char* comment)
{
	p_list* ptrNode = _search(name);
	if(ptrNode==NULL)
		return -1;

	ptrNode->item.value = value;
	if(comment!=NULL)
		ptrNode->item.comment = comment;

	return 0;
}

int ParamList::_reset(std::string name, std::string value, std::string comment)
{
	return ( _reset((char*)name.c_str(), (char*)value.c_str(), (char*)comment.c_str()) );
}

/*
 * search the node with the selected 'name'
 * member function [ZuoW,2010/11/9]
 */
void ParamList::_search(char* name, p_list* &ptrNode)
{
	// run over the list and find the 'name' node
	p_list* curNode = head;
	p_list* preNode = head;

	while(curNode!=NULL)
	{
		std::string nodeName = name;
		int ret = nodeName.compare(curNode->item.name);
		if(ret==0)	// find the node
		{
			ptrNode = curNode;

			return;
		}

		preNode = curNode;
		curNode = curNode->next;
	}

	return;
}

p_list* ParamList::_search(char* name)
{
	// run over the list and find the 'name' node
	p_list* curNode = head->next;
	p_list* preNode = head;

	std::string nodeName = name;
	while(curNode!=NULL)
	{
		int ret = nodeName.compare(curNode->item.name);
		if(ret==0)	// find the node
		{
			return curNode;
		}

		preNode = curNode;
		curNode = curNode->next;
	}

	return NULL;
}

/*
 * get the value with the selected 'name' and char type
 * interface function [ZuoW,2010/11/9]
 */
int ParamList::GetValue(char* name, char* sv)
{
	// search the node
	p_list* ptrNode = _search(name);
	if(ptrNode==NULL)
		return -1;

	// delete the array flag
	std::string value = ptrNode->item.value;
	if(value.at(0)=='[' && value.at(value.length()-1)==']')
		value = value.substr(1, value.length()-2);
	// get the first item when there are one more items in the value domain
	std::string::size_type loc = value.find(',', 0);
	if(loc!=std::string::npos)
		value = value.substr(0, loc);
	// delete the string flag if there are
	if(value.at(0)=='"' && value.at(value.length()-1)=='"')
	{
		value.erase(0, 1);
		value.erase(value.length()-1, 1);
	}


	if(sv!=NULL)
		strncpy(sv, value.c_str(), sizeof(sv));
	else
	{
		sv = new char[value.length()+1];
		strncpy(sv, value.c_str(), value.length());
	}

	return 0;
}

/*
 * get the value with the selected 'name and char array type
 * interface function [ZuoW,2010/11/9]
 */
int ParamList::GetValue(char* name, char* sv[], int number)
{
	// search the node
	p_list* ptrNode = _search(name);
	if(ptrNode==NULL)
		return -1;

	// delete the '[]'
	std::string value = ptrNode->item.value;
	if(value[0]=='[' && value[value.length()-1]==']')
	{
		value.erase(0, 1);
		value.erase(value.length()-1, 1);
	}

	for(int num=0; num<number; num++)
	{
		std::string::size_type loc = value.find(',', 0);
		std::string subValue;
		subValue.clear();
		if(loc!=std::string::npos)
		{
			subValue = value.substr(0, loc);
			value = value.substr(loc+1, value.length()-loc-1);
		}
		else
		{
			subValue = value;
		}

		// delete the '"'
		if(subValue[0]=='"' && subValue[subValue.length()-1]=='"')
		{
			subValue.erase(0, 1);
			subValue.erase(subValue.length()-1, 1);
		}
		strncpy(*(sv+num), subValue.c_str(), sizeof(sv+num));
	}

	return 0;
}

/*
 * get the value with the selected 'name' and int array type
 * interface function [ZuoW,2010/11/9]
 */
int ParamList::GetValue(char* name, int* iv, int number)
{
	// search the node
	p_list* ptrNode = _search(name);
	if(ptrNode==NULL)
		return -1;

	// delete the '[]'
	std::string value = ptrNode->item.value;
	if(value[0]=='[' && value[value.length()-1]==']')
	{
		value.erase(0, 1);
		value.erase(value.length()-1, 1);
	}

	for(int num=0; num<number; num++)
	{
		std::string::size_type loc = value.find(',', 0);
		std::string subValue;
		subValue.clear();
		if(loc!=std::string::npos)
		{
			subValue = value.substr(0, loc);
			value = value.substr(loc+1, value.length()-loc-1);
		}
		else
		{
			subValue = value;
		}

		// delete the '"'
		if(subValue[0]=='"' && subValue[subValue.length()-1]=='"')
		{
			subValue.erase(0, 1);
			subValue.erase(subValue.length()-1, 1);
		}

		*(iv+num) = atoi(subValue.c_str());
	}

	return 0;
}

/*
 * get the value with the selected 'name' and int array type
 * interface function [ZuoW,2010/11/9]
 */
int ParamList::GetValue(char* name, long* lv, int number)
{
	// search the node
	p_list* ptrNode = _search(name);
	if(ptrNode==NULL)
		return -1;

	// delete the '[]'
	std::string value = ptrNode->item.value;
	if(value[0]=='[' && value[value.length()-1]==']')
	{
		value.erase(0, 1);
		value.erase(value.length()-1, 1);
	}

	for(int num=0; num<number; num++)
	{
		std::string::size_type loc = value.find(',', 0);
		std::string subValue;
		subValue.clear();
		if(loc!=std::string::npos)
		{
			subValue = value.substr(0, loc);
			value = value.substr(loc+1, value.length()-loc-1);
		}
		else
		{
			subValue = value;
		}

		// delete the '"'
		if(subValue[0]=='"' && subValue[subValue.length()-1]=='"')
		{
			subValue.erase(0, 1);
			subValue.erase(subValue.length()-1, 1);
		}

		*(lv+num) = atol(subValue.c_str());
	}

	return 0;
}

/*
 * get the value with the selected 'name' and float array type
 * interface function [ZuoW,2010/11/9]
 */
int ParamList::GetValue(char* name, float* fv, int number)
{
	// search the node
	p_list* ptrNode = _search(name);
	if(ptrNode==NULL)
		return -1;

	// delete the '[]'
	std::string value = ptrNode->item.value;
	if(value[0]=='[' && value[value.length()-1]==']')
	{
		value.erase(0, 1);
		value.erase(value.length()-1, 1);
	}

	for(int num=0; num<number; num++)
	{
		std::string::size_type loc = value.find(',', 0);
		std::string subValue;
		subValue.clear();
		if(loc!=std::string::npos)
		{
			subValue = value.substr(0, loc);
			value = value.substr(loc+1, value.length()-loc-1);
		}
		else
		{
			subValue = value;
		}

		// delete the '"'
		if(subValue[0]=='"' && subValue[subValue.length()-1]=='"')
		{
			subValue.erase(0, 1);
			subValue.erase(subValue.length()-1, 1);
		}

		*(fv+num) = atof(subValue.c_str());
	}

	return 0;
}

/*
 * get the value with the selected 'name' and double array type
 * interface function [ZuoW,2010/11/9]
 */
int ParamList::GetValue(char* name, double* dv, int number)
{
	// search the node
	p_list* ptrNode = _search(name);
	if(ptrNode==NULL)
		return -1;

	// delete the '[]'
	std::string value = ptrNode->item.value;
	if(value[0]=='[' && value[value.length()-1]==']')
	{
		value.erase(0, 1);
		value.erase(value.length()-1, 1);
	}

	for(int num=0; num<number; num++)
	{
		std::string::size_type loc = value.find(',', 0);
		std::string subValue;
		subValue.clear();
		if(loc!=std::string::npos)
		{
			subValue = value.substr(0, loc);
			value = value.substr(loc+1, value.length()-loc-1);
		}
		else
		{
			subValue = value;
		}

		// delete the '"'
		if(subValue[0]=='"' && subValue[subValue.length()-1]=='"')
		{
			subValue.erase(0, 1);
			subValue.erase(subValue.length()-1, 1);
		}

		*(dv+num) = atof(subValue.c_str());
	}

	return 0;
}

/*
 * set value with selected 'name' and char type
 * interface function [ZuoW,2010/11/10]
 */
int ParamList::SetValue(char* name, char* sv, char* comment)
{
	// search the node
	p_list* ptrNode = _search(name);
	if(ptrNode==NULL)
		return -1;

	std::stringstream ioStream;

	ioStream << *sv ;

	ptrNode->item.value = ioStream.str();
	if(sizeof(comment)>0)
		ptrNode->item.comment = comment;

	return 0;
}

/*
 * add a node to the link list
 * interface function [ZuoW,2010/11/11]
 */
int ParamList::AddValue(char* name, char* sv, char* comment)
{
	int ret = _add(name, sv, comment);
	if(ret!=0)
		return -1;

	return 0;
}

/*
 * output the list into a file with the name 'paramfile'
 * interface function [ZuoW,2010/11/11]
 */
int ParamList::MakeFile(char* paramfile)
{
	std::string name, value, comment, prefix;
	std::string::size_type loc;
	std::ofstream outFile;
	bool headBlock, tailBlock, bArray;		// block's head and, flag and array's flag
	headBlock = tailBlock = bArray = false;
	int nBlockItem = 0;				// number of block's item

	outFile.open(paramfile);
	if(!outFile)
	{
		std::cout << "Open the output file failed:%s" << paramfile <<std::endl;
		return -1;
	}

	p_list* ptrNode = head->next;
	while(ptrNode!=NULL)
	{
		name = ptrNode->item.name;
		value = ptrNode->item.value;
		comment = ptrNode->item.comment;

		// block
		loc = name.find('.', 0);
		if(loc!=std::string::npos)
		{
			std::string temp = name.substr(0, loc);
			if(temp.compare(prefix)!=0)
			{
				headBlock = true;
				if(!prefix.empty())
					tailBlock = true;

				nBlockItem = 1;
			}
			else
				nBlockItem++;

			prefix = name.substr(0, loc);
			name = name.substr(loc+1, name.length()-loc-1);
		}
		else if(!prefix.empty())
		{
			tailBlock = true;
			prefix.clear();

			nBlockItem = 0;
		}

		// output the prefix
		if(tailBlock)
		{
			outFile << '}' << std::endl;
			tailBlock = false;
		}

		if(headBlock)
		{
			outFile << std::endl;
			outFile << prefix << '=' << std::endl;
			outFile << '{' << std::endl;

			headBlock = false;
		}

		// output the name
		if(nBlockItem>0)	// aligment in the block
			outFile << '\t';
		outFile << name << '=';

		// output the value
		if(value.at(0)=='[' && value.at(value.length()-1)==']')
		{
			bArray = true;
			outFile << std::endl << '[' << std::endl;
			value.erase(0, 1);
			value.erase(value.length()-1, 1);
		}

		outFile << value ;

		if(bArray)
		{
			outFile << std::endl << ']';
			bArray = false;
		}
		outFile << ';';

		// output the comment
		if(comment.length()>0)
			outFile << '#' << comment << std::endl;
		else
			outFile << std::endl;

		ptrNode = ptrNode->next;

		name.clear();
		value.clear();
		comment.clear();
	}

	// output the last prefix
	if(nBlockItem>1)
	{
		outFile << '}' << std::endl;
		tailBlock = false;
	}
	outFile.close();

	if(ptrNode!=NULL)
	{
		delete ptrNode;
		ptrNode = NULL;
	}

	return 0;
}

