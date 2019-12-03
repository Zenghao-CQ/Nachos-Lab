// filehdr.h 
//	Data structures for managing a disk file header.  
//
//	A file header describes where on disk to find the data in a file,
//	along with other information about the file (for instance, its
//	length, owner, etc.)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#ifndef FILEHDR_H
#define FILEHDR_H

#include "disk.h"
#include "bitmap.h"
#include "time.h"

//Modify for lab5 exe2 exetend filehdr info
#ifdef EXTEND

#define TimeInfoSize 26 //25+'/0'
#define TypeInfoSize 5 //4+'/0'
#define NumDirect 	((SectorSize - 2 * sizeof(int) - (3 * TimeInfoSize + TypeInfoSize ) * sizeof(char)) / sizeof(int))

#else
#define NumDirect 	((SectorSize - 2 * sizeof(int)) / sizeof(int))
#endif //EXTEND
#define MaxFileSize 	(NumDirect * SectorSize)

// The following class defines the Nachos "file header" (in UNIX terms,  
// the "i-node"), describing where on disk to find all of the data in the file.
// The file header is organized as a simple table of pointers to
// data blocks. 
//
// The file header data structure can be stored in memory or on disk.
// When it is on disk, it is stored in a single sector -- this means
// that we assume the size of this data structure to be the same
// as one disk sector.  Without indirect addressing, this
// limits the maximum file length to just under 4K bytes.
//
// There is no constructor; rather the file header can be initialized
// by allocating blocks for the file (if it is a new file), or by
// reading it from disk.

class FileHeader {
  public:
    bool Allocate(BitMap *bitMap, int fileSize);// Initialize a file header, 
						//  including allocating space 
						//  on disk for the file data
    void Deallocate(BitMap *bitMap);  		// De-allocate this file's 
						//  data blocks

    void FetchFrom(int sectorNumber); 	// Initialize file header from disk
    void WriteBack(int sectorNumber); 	// Write modifications to file header
					//  back to disk

    int ByteToSector(int offset);	// Convert a byte offset into the file
					// to the disk sector containing
					// the byte

    int FileLength();			// Return the length of the file 
					// in bytes

    void Print();			// Print the contents of the file.
    //modify lab5 exe2 modify extend info
#ifdef EXTEND
    void setType(char* in){strcpy(type,in);}
    void setCreateTime();
    void setAccessTime();
    void setModifyTime();
    void initFileHdr(char* in);
    void setHdrPos(int pos){hdrPos = pos;}
    int getHdrPos(){return hdrPos;}
#endif
#ifdef EXE_LEN
    bool expandFileSize(BitMap* freeMap,int addBytes);
#endif
  private:
    int numBytes;			// Number of bytes in the file
    int numSectors;			// Number of data sectors in the file
    int dataSectors[NumDirect];		// Disk sector numbers for each data 
					// block in the file
    //modify lab5 exe2
#ifdef EXTEND
    char type[TypeInfoSize];
    char createTime[TimeInfoSize];
    char modifyTime[TimeInfoSize];
    char accessTime[TimeInfoSize];
    int hdrPos;
#endif
};
//modify lab5
#ifdef EXTEND
char * calType(char * filename);
#endif //EXTEND
#endif // FILEHDR_H
