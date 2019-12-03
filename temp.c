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

    if ((position + numBytes) > fileLength)
#ifndef EXE_LEN
	numBytes = fileLength - position;
#else
    {
        OpenFile *freeMapFile = new OpenFile(0);
        BitMap *freeMap = new BitMap(NumSectors);
        freeMap->FetchFrom(freeMapFile);
        hdr->expandFileSize(freeMap,position + numBytes - fileLength);
        hdr->WriteBack(hdr->getHdrPos());
        freeMap->WriteBack(freeMapFile);
        delete freeMapFile;
        fileLength = hdr->FileLength();
    }
#endif
