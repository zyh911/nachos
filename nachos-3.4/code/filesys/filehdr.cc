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

void
FileHeader::AllocateToDataSectors(BitMap *freeMap, int numSectors, int dataSectors[])
{
	int i;
	for(i=0;i<=numSectors;i++)
		dataSectors[i] = freeMap->Find();
}

bool
FileHeader::Allocate(BitMap *freeMap, int fileSize)
{ 
    int lastIndex = NumDirect - 1;
    int subSectors;
    numBytes = fileSize;
    numSectors  = divRoundUp(fileSize, SectorSize);
    if (freeMap->NumClear() < numSectors)
	return FALSE;		// not enough space

    if(numSectors < lastIndex)	 
    {
	//    printf("numSectors = %d\n",numSectors);
	AllocateToDataSectors(freeMap, numSectors, dataSectors);
	dataSectors[lastIndex] = -1;
    }
   else
   {
	   AllocateToDataSectors(freeMap, lastIndex, dataSectors);
	   dataSectors[lastIndex] = freeMap->Find();

	   int dataSectors2[NumDirect];
	   subSectors = numSectors - lastIndex;

	   //Make sure that we have enough secend level index saving blocks
	   //If not, ASSERT right now
	   ASSERT(subSectors<=NumDirect);

	   AllocateToDataSectors(freeMap, subSectors, dataSectors2);
		   
	//   printf("write the last sector to subSectors = %d\n", subSectors-1);
	 //  printf("write dataSectors2 to dataSectors[%d]\n",lastIndex);
	   synchDisk->WriteSector(dataSectors[lastIndex],(char *)dataSectors2);
   }
    return TRUE;
}

//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

void 
FileHeader::Deallocate(BitMap *freeMap)
{
    int lastIndex = NumDirect - 1;
    int dataSectors2[lastIndex];
    int Sectors;
    if(numSectors >= lastIndex)
    {
    	int subSectors = numSectors - lastIndex;
	int dataSectors2[lastIndex];
	synchDisk->ReadSector(dataSectors[lastIndex],(char *)dataSectors2);
    	for(int i = 0; i < subSectors;i++)
	{
		ASSERT(freeMap->Test((int) dataSectors2[i]));  // ought to be marked!
		freeMap->Clear((int) dataSectors2[i]);
//		printf("dataSectors2[%d] was deallocated\n",i);
	}
	Sectors = NumDirect;
    }
    else
	Sectors = numSectors;

    for (int i = 0; i < Sectors; i++) {
	ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
	freeMap->Clear((int) dataSectors[i]);
//	printf("dataSectors[%d] was deallocated\n",i);
    	}
}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk. 
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void
FileHeader::FetchFrom(int sector)
{
//	printf("FetchFrom at sector=%d\n", sector);
    synchDisk->ReadSector(sector, (char *)this);
 //   printf("this->numBytes = %d\n",this->numBytes);
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

int
FileHeader::ByteToSector(int offset)
{
    int lastIndex = NumDirect - 1;
    if(offset / SectorSize >= lastIndex)	//edit by YePeng
    {
	int dataSectors2[lastIndex];
	//printf("ByteToSector:dataSector[%d]\t",lastIndex);
	synchDisk->ReadSector(dataSectors[lastIndex],(char *)dataSectors2);
	//printf("ByteToSector:offset = %d, SectorSize = %d, SectorNumber = %d, dataSector2[%d]=%d\n", offset, SectorSize, offset / SectorSize, offset / SectorSize - lastIndex, dataSectors2[offset / SectorSize - lastIndex]);
    	return(dataSectors2[offset / SectorSize - lastIndex]);
    }
    else
    	return(dataSectors[offset / SectorSize]);
}

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
FileHeader::IncreaseBytes(int Bytes)
{
	numBytes += Bytes;
}
void
FileHeader::UpdateNumSectors(int newNumSectors)
{
	numSectors = newNumSectors;
}
void
FileHeader::Print()
{
    int i, j, k;
    char *data = new char[SectorSize];
    int lastIndex = NumDirect - 1;
    int subSectors;

    printf("FileHeader contents.  File size: %d.  File blocks:\n", numBytes);
    if(dataSectors[lastIndex] == -1)
    {
	    printf("numSectors is %d\n", numSectors);
    for (i = 0; i < numSectors; i++)
	printf("%d ", dataSectors[i]);
    printf("\nLevel 1 File contents:\n");
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
    }
    else 
    {
    for (i = 0; i < lastIndex; i++)
	printf("%d ", dataSectors[i]);
    int dataSectors2[lastIndex];
    //printf("ReadSector from lastIndex:%d\n",dataSectors[lastIndex]);
    synchDisk->ReadSector(dataSectors[lastIndex],(char *)dataSectors2); 
    subSectors = numSectors - lastIndex;
    for (i = 0; i < subSectors; i++)
	printf("%d ", dataSectors2[i]);

    printf("\nLevel 1 & 2 File contents:\n");

    for (i = k = 0; i < lastIndex; i++) {
	synchDisk->ReadSector(dataSectors[i], data);
       	for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
    		if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
			printf("%c", data[j]);
        	else
			printf("\\%x", (unsigned char)data[j]);
		}
       	printf("\n"); 
    	}
    for (i = k = 0; i < subSectors; i++) {
	synchDisk->ReadSector(dataSectors2[i], data);
       	for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
    		if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
			printf("%c", data[j]);
        	else
			printf("\\%x", (unsigned char)data[j]);
		}
       	printf("\n"); 
    	}
    }
    delete [] data;
}

