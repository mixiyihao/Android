//============================================================================
// Name        : AXML.cpp
// Author      : mixi
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <errno.h>
#include <string>
#include"AXMLFile.h"
using namespace std;

static int getFileStartAndLength(int fd, off_t *start_, size_t *length_) {
	off_t start, end;
	size_t length;
	start = lseek(fd, 0L, SEEK_CUR);
	end = lseek(fd, 0L, SEEK_END);
	(void) lseek(fd, start, SEEK_SET);
	if (start == (off_t) -1 || end == (off_t) -1) {
		cout << "could not determine length of file" << endl;
		return -1;
	}
	length = end - start;
	if (length == 0) {
		cout << "file is empty" << endl;
		return -1;
	}
	*start_ = start;
	*length_ = length;
	return 0;
}

int sysMapFileInShmemWritableReadOnly(int fd, struct MemMapping *pMap) {
	off_t start;
	size_t length;
	void *memPtr;
	if (getFileStartAndLength(fd, &start, &length) < 0) {
		return -1;
	}
	memPtr = malloc(length);
	if (read(fd, memPtr, length) < 0) {
		cout << "the read axml file error" << endl;
		return -1;
	}
	pMap->baseAddr = pMap->addr = memPtr;
	pMap->baseLength = pMap->length = length;
	return 1;
}
void baseAXML(void *base, struct AXMLFile *axmlFile) {
	cout << "base = " << base << endl;
	axmlFile->base = (u1 *) base;
	axmlFile->header = (struct Header *) base;
	int size = sizeof(struct Header);
	axmlFile->stringContent = (struct StringContent *) (axmlFile->base + size);
}
/**
 * release
 */
void sysReleaseShmem(struct MemMapping *pMap) {
	if (pMap->baseAddr == NULL && pMap->baseLength == 0)
		return;
	if (pMap->baseAddr != NULL) {
		free(pMap->baseAddr);
		pMap->baseAddr = NULL;
	}
	pMap->baseLength = 0;
}
/**
 * print the the head
 */
void printfHeade(struct Header *header) {
	cout << "the  file magic = " << header->magic << endl;
	cout << "the  file size = " << header->fileSize << "kb" << endl;
	cout << "the  stringChunkTag = " << header->stringChunkTag << endl;
	cout << "the  stringChunksize = " << header->stringChunksize << endl;
	cout << "string context offset=" << header->stringContextOffset << endl;
	cout << "string size =" << header->stringSize << endl;
	cout << "style offset =" << header->styleOffset << endl;
	cout << "style size =" << header->styleSize << endl;
	cout << "unknown = " << header->unknown << endl;
	cout << "------------------header end !----------------------" << endl;
}
/**
 * pring single the string content
 */
void printfS(u4 index, struct AXMLFile *file) {
	char *p1 = getStringByIndex(index, file);
	printf("%s \n", p1);
}
/**
 * print the string content
 */
void printfStringContent(struct AXMLFile *axmlFile) {
	u4 index = 0;
	for (index = 0; index < axmlFile->header->stringSize; index++) {
		printfS(index, axmlFile);
	}
}

/**
 * get the xml content point
 */
u1* getXMLContentPoint(struct AXMLFile *axmlFile) {
	return (u1*) (axmlFile->base + axmlFile->header->stringChunksize + 0x08);

}

struct ResouceChunk * getResouceChunk(u1 *base, struct AXMLFile *axmlFile) {
	return (struct ResouceChunk*) base;
}
struct ResourceId *getResourceIdList(u1 *base, struct AXMLFile *axmlFile) {

	return (struct ResourceId *) (base + sizeof(struct ResouceChunk));
}

void printResouceList(struct ResouceIdList * list) {
	u4 i;
	for (i = 0; i < (list->resourceIdSize - 8) / 4; i++) {
		printf("%x\n", list->resourId[i]);
	}
}
/**
 * parse Namespace chunk
 */
void printNamespaceChunk(struct StartNamespaceChunk *chunk,
		struct AXMLFile * file) {
	cout << chunk->lineNumber << " " << getStringByIndex(chunk->prefix, file)
			<< " " << getStringByIndex(chunk->uri, file) << endl;
}
void printStartChunk(struct StartTagChunk *chunk,struct AXMLFile *file){
	static int secondNumber =2;
	//print stack chunk
	/**
	u4 startTagChunk; //flag = 0x00100102
	u4 chunkSize; //size
	u4 lineNumber;
	u4 value; //0xffffffff
	u4 nameSpaceUri; //0xffffffff   -1
	u4 name;
	u4 value2; //0x00140014
	u4 attrValueNum; //attribute number
	u4 duiqi; //0x00 00 00 00
	 */
	/*
	printf("startTagChunk =  %08x\n",chunk->startTagChunk);
	printf("lineNumber =  %08x\n",chunk->lineNumber);
	printf("chunk size = %08x\n",chunk->chunkSize);
	printf("value = %08x\n",chunk->value);
	printf("nameSpaceUri = %08x\n",chunk->nameSpaceUri);
	printf("value2 = %08x\n",chunk->value2);
	printf("name = %08x\n",chunk->name);
	printf("attrValueNum = %08x\n",chunk->attrValueNum);
	printf("duiqi = %08x\n",chunk->duiqi);
	*/

	char *startName = getStringByIndex(chunk->name,file);
	cout<<"<"<<startName<<endl;
	//char * starUri = getStringByIndex(chunk->nameSpaceUri,file);
	//cout<<starUri<<endl;
	if(secondNumber == 2){
			//如果是第二次 那么
		secondNumber++;
		cout<<"xmlns:android http://schemas.android.com/apk/res/android"<<endl;
	}
	u4 i=0;
	//cout<<"-----chunk the value count:-----"<<chunk->attrValueNum<<endl;
	char attrcontent[512];
	for(i=0;i<chunk->attrValueNum;i++){
		//cout<<"----------start"<<i<<"------------"<<endl;
		Attribute attribute = chunk->attribute[i];
		u4 namespaceUri = attribute.nameSpaceUri;
		u4 name = attribute.name;
		char *prefix = NULL;
		if(namespaceUri != 0xffffffff){
			prefix = getStringByIndex(namespaceUri-1,file);
			//printf("%s\n",strNamespaceuri);
		}
		char * strname = getStringByIndex(name,file);
		//cout<<strname<<endl;
		u1 type = attribute.type >>24;
		//printf("%d",type);
		char buffer[256];
		switch(type){
		case 16:
			snprintf(buffer,256,"%d",attribute.data);
			break;
		case 3:{
			char *p = getStringByIndex(attribute.stringValue,file);
			snprintf(buffer,256,"%s",p);
		}
			break;
		case 18:{
			if(attribute.data == 0){
				memcpy(buffer,"false\0",6);
			}else{
				memcpy(buffer,"true\0",5);
			}
		}
		break;
		case 1:{
			if(attribute.data >>24 == 1){
				snprintf(buffer,256,"@android:%08x",attribute.data);
			}else{
				snprintf(buffer,256,"@%08x",attribute.data);
			}
		}
		break;
		case 17:{
			snprintf(buffer,256,"0x%08x",attribute.data);
		}
		break;
		default :
			printf("%d\n",type);
			snprintf(buffer,256,"%08x-%08x",attribute.stringValue,attribute.data);
			break;

		}
		//printf("%s",buffer);
		if(prefix == NULL){
			snprintf(attrcontent,sizeof(attrcontent),"%s = \"%s\"",strname,buffer);
		}else{
			snprintf(attrcontent,sizeof(attrcontent),"android:%s = \"%s\"",strname,buffer);
		}
		cout<<attrcontent<<endl;
	/*	cout<<"name space URI:"<<hex<<attribute.nameSpaceUri<<endl;
		cout<<"Name:"<<hex<<attribute.name<<endl;
		cout<<"Value String:"<<hex<<attribute.stringValue<<endl;
		cout<<"Type:"<<hex<<attribute.type<<endl;
		cout<<"Data:"<<hex<<attribute.data<<endl;
		cout<<"-----------end-------------"<<endl;
	 */
		//cout<<"-----------end-------------"<<endl;
	}

}
/*void getTheAttrvalue(u4 type,char *buff,u4 data,struct AXMLFile *file){
	static float RadixTable[] ={0.00390625f, 3.051758E-005f, 1.192093E-007f, 4.656613E-010f};
	static char *DimemsionTable [] = {"px", "dip", "sp", "pt", "in", "mm", "", ""};
	static char *FractionTable[]= {"%", "%p", "", "", "", "", "", ""};
	char temBuff[32];
	switch(type){




	}


}*/

void printEndTagChunk(struct EndTagChunk * chunk,struct AXMLFile *file){

}

AXMLEvent doNext(u1 **base, u4 *len, struct AXMLFile * file) {
	AXMLEvent event;
	u1 *pBase = *base;
	u4 type = GetInt32(base);
	switch (type) {
	case CHUNK_STARTNS: {
		struct StartNamespaceChunk *chunk = (struct StartNamespaceChunk *) pBase;
		*len = chunk->chunkSize;
		event = AE_STARTDOC;
		printNamespaceChunk(chunk, file);
	}
		break;
	case CHUNK_STARTTAG: {
		struct StartTagChunk *chunk = (struct StartTagChunk *) pBase;
		*len = chunk->chunkSize;
		event = AE_STARTTAG;
		printStartChunk(chunk,file);

	}
		break;
	case CHUNK_ENDTAG:{
		struct EndTagChunk *chunk = (struct EndTagChunk*)pBase;
		*len = sizeof(struct EndTagChunk);
		event = AE_ENDTAG;
		printEndTagChunk(chunk,file);
	}
		break;
	case CHUNK_ENDNS:
		event = AE_ENDDOC;
		break;
	default:
		*len = 1;
		event = AE_ERROR;
		break;

	}

	return event;
}

void startParse(void *base, struct AXMLFile * file) {
	u1 *p1 = (u1 *) base;
	u4 len = -1;
	int count = 0;
	while (doNext(&p1, &len, file) != AE_ENDDOC) {
		p1 += len;
		/*if (count++ > 4) {
			break;
		} else {
			continue;
		}*/
	}
}

int main() {
	const char* path = "C:\\Users\\mixi\\Desktop\\AXML\\AndroidManifest.xml";
	int fd = open(path, 0);
	if (fd < 0) {
		cout << "open the" << path << " errno=" << errno << endl;
		return -1;
	}

	//recorder the memory mapping address
	struct MemMapping map;
	int memp = sysMapFileInShmemWritableReadOnly(fd, &map);
	if (memp < 0) {
		cout << "map the memory error" << endl;
		return -2;
	}
	struct AXMLFile *axmlFile = (struct AXMLFile*) malloc(
			sizeof(struct AXMLFile *));
	//is AXML? and size is right
	baseAXML(map.baseAddr, axmlFile);

	if (axmlFile->header->magic != AXML_FLAG) {
		cout << "the file is not binary xml file" << endl;
		return -1;
	}
	printfHeade(axmlFile->header);
	printfStringContent(axmlFile);
	u4 size = axmlFile->header->styleSize;
	if (size > 0) {
		cout << "not relize style function" << endl;
		return -3;
	}
	//read the ResouceIdList
	u1 * pXMLContent = getXMLContentPoint(axmlFile);
	struct ResouceIdList list;
	struct ResouceChunk *chunk = getResouceChunk(pXMLContent, axmlFile);
	list.resourceIdSize = chunk->resourceChunkSize;
	printf("resouceId size = %d\n", chunk->resourceChunkSize);
	list.resourId = getResourceIdList(pXMLContent, axmlFile);
	printResouceList(&list);
	//printf("%x",pXMLContent-axmlFile->base);
	//read the namespace tag

	u1 *namespaceTag = pXMLContent + list.resourceIdSize;
	printf("the namespaceTag = %x\n", namespaceTag - axmlFile->base);
	u4 chunkType = GetInt32(&namespaceTag);
	printf("chunkType = %x\n", chunkType);
	if (chunkType == 0x100100) {
		//start the parse axml
		cout << "------start parse axml-----------------" << endl;
		cout << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << endl;
		startParse(namespaceTag, axmlFile);

	} else {
		cout << "parse error in the namespaceTag" << endl;
	}
	//release the AXMLFile
	free(axmlFile);
	//release memory mapping
	sysReleaseShmem(&map);
	close(fd);
	return 0;
}

