// filehdr.cc 
//	Routines for managing the disk file header (in UNIX, this
//	would be called the i-node).
//
//	The file header is used to locate where on disk the 
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the 
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect 
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector, 
//
//      Unlike in a real system, we do not keep track of file permissions, 
//	ownership, last modification date, etc., in the file header. 
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "filehdr.h"

//----------------------------------------------------------------------
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return FALSE if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the bit map of free disk sectors
//----------------------------------------------------------------------
#ifndef INDIRERCT
bool
FileHeader::Allocate(BitMap *freeMap, int fileSize)
{ 
    numBytes = fileSize;
    numSectors  = divRoundUp(fileSize, SectorSize);
    if (freeMap->NumClear() < numSectors)
	return FALSE;		// not enough space

    for (int i = 0; i < numSectors; i++)
    {
    	dataSectors[i] = freeMap->Find();
        printf("Allocate direct sector: %d\n",dataSectors[i]);
    }
    return TRUE;
}
#else
bool
FileHeader::Allocate(BitMap *freeMap, int fileSize)
{ 
    numBytes = fileSize;
    numSectors  = divRoundUp(fileSize, SectorSize);
    if (freeMap->NumClear() < numSectors)
	return FALSE;		// not enough space
    if(numSectors < NumDirect)
    {
        for (int i = 0; i < numSectors; i++)
        {
	        dataSectors[i] = freeMap->Find();
            printf("Allocate direct sector: %d\n",dataSectors[i]);
        }
        printf("Allocate file space success, size: %d,sector size: %d\n",fileSize,numSectors);
        return TRUE;
    }
    else if(numSectors < (NumDirect - 1) + SectorSize/sizeof(int) )
    {
        for(int i = 0; i < NumDirect; ++i)//include last page
        {
	        dataSectors[i] = freeMap->Find();
            printf("Allocate direct sector: %d\n",dataSectors[i]);
        }
        int indirect[SectorSize/sizeof(int)];
        for(int i = 0; i < numSectors - NumDirect + 1; ++i)
        {
            indirect[i] = freeMap->Find();
            printf("Allocate indirect sector: %d\n",indirect[i]);
        }        
        synchDisk->WriteSector(dataSectors[NumDirect-1], (char*) indirect);
        printf("Allocate file space success, size: %d,sector size: %d\n",fileSize,numSectors);
        return TRUE;
    }
    else
    {
        DEBUG('f',"FileSize out of range\n");
        printf("file size:%d,max size:%d\n",numSectors,(NumDirect - 1) + SectorSize/sizeof(int));
        ASSERT(FALSE);
    }
}
#endif //INDIRERCT
//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------
#ifndef INDIRERCT
void 
FileHeader::Deallocate(BitMap *freeMap)
{
    for (int i = 0; i < numSectors; i++) {
	ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
	freeMap->Clear((int) dataSectors[i]);
    }
}
#else
void 
FileHeader::Deallocate(BitMap *freeMap)
{
    if(numSectors < NumDirect)
    {
        for (int i = 0; i < numSectors; i++) 
        {            
            printf("DeAllocate direct sector: %d\n",dataSectors[i]);
	        ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
	        freeMap->Clear((int) dataSectors[i]);
        }
    }
    else
    {
        int indirect[SectorSize/sizeof(int)];
        synchDisk->ReadSector(dataSectors[NumDirect-1],(char*)indirect);
        for(int i = 0; i < numSectors - NumDirect + 1; ++i)
        {
            printf("DeAllocate indirect sector: %d\n",indirect[i]);
            ASSERT(freeMap->Test((int) indirect[i]));  // ought to be marked!
	        freeMap->Clear((int) indirect[i]);
        }
    }
}
#endif INDIRERCT
//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk. 
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void
FileHeader::FetchFrom(int sector)
{
    synchDisk->ReadSector(sector, (char *)this);
}

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk. 
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void
FileHeader::WriteBack(int sector)
{
    synchDisk->WriteSector(sector, (char *)this); 
}

//----------------------------------------------------------------------
// FileHeader::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------
#ifndef INDIRERCT
int
FileHeader::ByteToSector(int offset)
{
    return(dataSectors[offset / SectorSize]);
}
#else
int
FileHeader::ByteToSector(int offset)
{
    if(offset < (NumDirect - 1) * SectorSize)
        return(dataSectors[offset / SectorSize]);
    else
    {
        int indirect[SectorSize/sizeof(int)];
        synchDisk->ReadSector(dataSectors[NumDirect-1],(char*)indirect);
        int pos = (offset - (NumDirect - 1) * SectorSize ) / SectorSize;
        return indirect[pos];
    }
}
#endif //INDIRERCT
//----------------------------------------------------------------------
// FileHeader::FileLength
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int
FileHeader::FileLength()
{
    return numBytes;
}

//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

void
FileHeader::Print()
{
    int i, j, k;
    char *data = new char[SectorSize];

    printf("FileHeader contents.  File size: %d.  File blocks:\n", numBytes);
#ifndef INDIRERCT
    for (i = 0; i < numSectors; i++)
	printf("%d ", dataSectors[i]);
#else
    if(numSectors < NumDirect)
    {
        printf("direct:");
        for (i = 0; i < numSectors; i++)
	        printf("%d ", dataSectors[i]);
    }
    else
    {
        printf("direct:");
        for (i = 0; i < NumDirect - 1; i++)
	        printf("%d ", dataSectors[i]);
        int indirect[SectorSize/sizeof(int)];
        synchDisk->ReadSector(dataSectors[NumDirect-1],(char*)indirect);
        printf("\nindirect:");
        for(int i = 0; i < numSectors - NumDirect + 1; ++i)
            printf("%d ", indirect[i]);
    }
#endif //INDIRERCT

#ifdef EXTEND
    //modify lab5 exe2
    printf("\ntype:%s",type);
    printf("createTime:%s",createTime);
    printf("accessTime:%s",accessTime);
    printf("last modifyTime:%s",modifyTime);
#endif //EXTEND
    printf("\nFile contents:\n");
#ifndef INDIRERCT
    for (i = k = 0; i < numSectors; i++) {
	synchDisk->ReadSector(dataSectors[i], data);
        for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
	    if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
		printf("%c", data[j]);
            else
		printf("\\%x", (unsigned char)data[j]);
	}
        printf("\n"); 
    }
#else
    if(numSectors < NumDirect-1)
    {
        for (i = k = 0; i < numSectors; i++) 
        {
    	    synchDisk->ReadSector(dataSectors[i], data);
            for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) 
            {
	            if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
		            printf("%c", data[j]);
                else
		            printf("\\%x", (unsigned char)data[j]);
	        }
            printf("\n"); 
        }
    }
    else
    {
        for (i = k = 0; i < NumDirect-1; i++) 
        {
    	    synchDisk->ReadSector(dataSectors[i], data);
            for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) 
            {
	            if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
		            printf("%c", data[j]);
                else
		            printf("\\%x", (unsigned char)data[j]);
	        }
            printf("\n"); 
        }
        int indirect[SectorSize/sizeof(int)];
        synchDisk->ReadSector(dataSectors[NumDirect-1],(char*)indirect);
        for(i = 0; i<numSectors - NumDirect +1; ++i)
        {
            synchDisk->ReadSector(indirect[i], data);
            for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) 
            {
	            if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
		            printf("%c", data[j]);
                else
		            printf("\\%x", (unsigned char)data[j]);
	        }
            printf("\n"); 
        }
    }

#endif //INDIRERCT
    delete [] data;
}

//modify lab exe2
#ifdef EXTEND
void
FileHeader::setCreateTime()
{
    time_t time_tmp;
    time(&time_tmp);
    struct tm* currentTime = localtime(&time_tmp);
    char* time = asctime(currentTime);
    
}
void
FileHeader::setModifyTime()
{
    time_t time_tmp;
    time(&time_tmp);
    struct tm* currentTime = localtime(&time_tmp);
    char* time = asctime(currentTime);
    strcpy(modifyTime,time);
}
void
FileHeader::setAccessTime()
{
    time_t time_tmp;
    time(&time_tmp);
    struct tm* currentTime = localtime(&time_tmp);
    char* time = asctime(currentTime);
    strcpy(accessTime,time);
}
void
FileHeader::initFileHdr(char *in)
{
    setType(in);
    time_t time_tmp;
    time(&time_tmp);
    struct tm* currentTime = localtime(&time_tmp);
    char* time = asctime(currentTime);
    strcpy(createTime,time);
    strcpy(modifyTime,time);
    strcpy(accessTime,time);
}
char * calType(char * filename)
{
  char* pos = strrchr(filename,'.');
  if(!pos) 
    return "None";
  else 
    return ++pos;
}
#endif //EXTEND

//modify lab5 exe5
#ifdef EXE_LEN
bool
FileHeader::expandFileSize(BitMap* freeMap,int addBytes)
{
    ASSERT(addBytes>0);
    numBytes += addBytes;
    int oldnumSectors = numSectors;
    numSectors = divRoundUp(SectorSize,numBytes);
    printf("Expands %d bytes,%d sectors,",addBytes,numSectors-oldnumSectors);
    if(numSectors == oldnumSectors)
        return TRUE;
    if(numSectors-oldnumSectors > freeMap->NumClear())
    {
        printf("faild Disk full!\n");
        return FALSE;
    }
    for(int i = oldnumSectors; i < numSectors; ++i)
        dataSectors[i] = freeMap->Find();
    return TRUE;
}
#endif //EXE_LEN