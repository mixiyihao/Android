#ifndef __DUMP
#define __DUMP
#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<sys/types.h>
#include<unistd.h>

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef uint64_t u8;
typedef int8_t s1;
typedef int16_t s2;
typedef int32_t s4;
typedef int64_t s8;
/**
 * AXML 的头文件
 */
static u4 AXML_FLAG = 0x00080003;

typedef enum {
	AE_STARTDOC = 0, AE_ENDDOC, AE_STARTTAG, AE_ENDTAG, AE_TEXT, AE_ERROR

} AXMLEvent;
enum {
	CHUNK_HEAD = 0x00080003,
	CHUNK_STRING = 0x001c0001,
	CHUNK_RESOURCE = 0x00080180,
	CHUNK_STARTNS = 0x00100100,
	CHUNK_ENDNS = 0x00100101,
	CHUNK_STARTTAG = 0x00100102,
	CHUNK_ENDTAG = 0x00100103,
	CHUNK_TEXT = 0x00100104,
};
/* attributes' types */
enum {
	ATTR_NULL = 0,
	ATTR_REFERENCE = 1,
	ATTR_ATTRIBUTE = 2,
	ATTR_STRING = 3,
	ATTR_FLOAT = 4,
	ATTR_DIMENSION = 5,
	ATTR_FRACTION = 6,
	ATTR_FIRSTINT = 16,
	ATTR_DEC = 16,
	ATTR_HEX = 17,
	ATTR_BOOLEAN = 18,
	ATTR_FIRSTCOLOR = 28,
	ATTR_ARGB8 = 28,
	ATTR_RGB8 = 29,
	ATTR_ARGB4 = 30,
	ATTR_RGB4 = 31,
	ATTR_LASTCOLOR = 31,
	ATTR_LASTINT = 31,
};




struct Header {
	u4 magic; //标识AXML 文件
	u4 fileSize; //文件的大小
	u4 stringChunkTag; //stringChunk的标识
	u4 stringChunksize; //stringChunk 的大小  0x08stringchunkSize+0x08
	u4 stringSize; //字符串的大小
	u4 styleSize; //style大小 未知作用
	u4 unknown; //补齐作用吧
	u4 stringContextOffset; //字符串内容偏移量+0x08
	u4 styleOffset; //stype的偏移量
};
/**
 * memory mapping
 */
struct MemMapping {
	void *addr;
	/* start of data */
	size_t length;
	/* length of data */
	void *baseAddr;
	/* page-aligned base address */
	size_t baseLength; /* length of mapping */
};

struct AXMLFile {
	u1 *base; //base address
	struct Header *header; //axml header
	struct StringContent *stringContent;

};
struct StartNamespaceChunk {
	u4 startNamespaceTag; //flag = 0x00100100
	u4 chunkSize; //size
	u4 lineNumber; //line number
	u4 value; //0xffffffff
	u4 prefix;
	u4 uri;
};
struct Attribute {
	u4 nameSpaceUri;
	u4 name;
	u4 stringValue;
	u4 type;
	u4 data;
};
struct StartTagChunk {
	u4 startTagChunk; //flag = 0x00100102
	u4 chunkSize; //size
	u4 lineNumber;
	u4 value; //0xffffffff
	u4 nameSpaceUri; //0xffffffff -1
	u4 name;
	u4 value2; //0x00140014
	u4 attrValueNum; //attribute number
	u4 duiqi; //0x00 00 00 00
	struct Attribute attribute[1];
};
struct EndTagChunk{
	u4 endTagChunk;
	u4 endTagSize;
	u4 name;
	u4 unknown;
	u4 unknown1;
	u4 unknown2;
};
struct ResouceChunk {
	u4 resourceIdTag;
	u4 resourceChunkSize;
};

struct ResouceIdList {
	u4 resourceIdSize;
	struct ResourceId *resourId;
};

struct ResourceId {
	u4 resourcId;
};

struct StringContent {
	u4 offset;
};
/**
 * 获取那ResourceId个数
 */
static inline int resourceIdCount(struct ResouceChunk *chunk) {
	return (chunk->resourceChunkSize - 8) / 4;
}
static inline u4 GetInt32(u1 **base) {
	u1 *p = *base;
	return (*(p + 0) | *(p + 1) << 8 | *(p + 2) << 16 | *(p + 3) << 24);
}
/**
 *获取StringContent
 */
struct StringContent* getStringContentByIndex(u4 index,
		struct AXMLFile * axml) {
	return &axml->stringContent[index];
}

/**
 *get string but no length
 */
u2* getStringNoLenByIndex(u4 index, struct AXMLFile * address) {
	struct StringContent *content = getStringContentByIndex(index, address);
	return (u2*) (address->base + content->offset
			+ address->header->stringContextOffset + 8 + 2);
}
/**
 *get string but hava length
 *
 */
u2* getStringLenByIndex(u4 index, struct AXMLFile * address) {
	struct StringContent *content = getStringContentByIndex(index, address);
	return (u2*) (address->base + content->offset
			+ address->header->stringContextOffset + 8);
}
/**
 *getstring
 */
char* getStringByIndex(u4 index, struct AXMLFile *address) {
	u2 * str = getStringLenByIndex(index, address);
	u2 len = str[0];
	//去掉第一个 还有len -1 个
	char * buffer = (char *) malloc(len + 1);
	int i;
	for (i = 1; i <= len; i++) {
		buffer[i - 1] = (char) str[i];
	}
	buffer[len] = '\0';
	return buffer;
}

#endif
