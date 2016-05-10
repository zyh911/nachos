// openfile.cc 
//	Routines to manage an open Nachos file.  As in UNIX, a
//	file must be open before we can read or write to it.
//	Once we're all done, we can close it (in Nachos, by deleting
//	the OpenFile data structure).
//
//	Also as in UNIX, for convenience, we keep the file header in
//	memory while the file is open.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "filehdr.h"
#include "openfile.h"
#include "system.h"
#include "directory.h"
extern void GetCurrentDate(char str[], int strlength);
extern void GetCurrentTime(char str[], int strlength);
#ifdef HOST_SPARC
#include <strings.h>
#endif

//Lock OpenFile::ReaderLock = new Lock("ReaderLock");
//Lock OpenFile::WriterLock = new Lock("WriterLock");
//----------------------------------------------------------------------
// OpenFile::OpenFile
// 	Open a Nachos file for reading and writing.  Bring the file header
//	into memory while the file is open.
//
//	"sector" -- the location on disk of the file header for this file
//----------------------------------------------------------------------

OpenFile::OpenFile(int sector)
{ 
    hdr = new FileHeader;
    currentSector = sector;
    hdr->FetchFrom(sector);
    seekPosition = 0;
}

//----------------------------------------------------------------------
// OpenFile::~OpenFile
// 	Close a Nachos file, de-allocating any in-memory data structures.
//----------------------------------------------------------------------

OpenFile::~OpenFile()
{
    delete hdr;
}

//----------------------------------------------------------------------
// OpenFile::Seek
// 	Change the current location within the open file -- the point at
//	which the next Read or Write will start from.
//
//	"position" -- the location within the file for the next Read/Write
//----------------------------------------------------------------------

void
OpenFile::Seek(int position)
{
    seekPosition = position;
}	

//----------------------------------------------------------------------
// OpenFile::Read/Write
// 	Read/write a portion of a file, starting from seekPosition.
//	Return the number of bytes actually written or read, and as a
//	side effect, increment the current position within the file.
//
//	Implemented using the more primitive ReadAt/WriteAt.
//
//	"into" -- the buffer to contain the data to be read from disk 
//	"from" -- the buffer containing the data to be written to disk 
//	"numBytes" -- the number of bytes to transfer
//----------------------------------------------------------------------

int
OpenFile::Read(char *into, int numBytes)
{
    GetCurrentDate(readDate, StringMaxLen);
    GetCurrentTime(readTime, StringMaxLen);
    //readTime = stats->totalTicks;
	
DEBUG('f', "seekPosition: %d \n", seekPosition);
   int result = ReadAt(into, numBytes, seekPosition);
   seekPosition += result;
   return result;
}

int
OpenFile::Write(char *into, int numBytes)
{
	int result;
   	result = WriteAt(into, numBytes, seekPosition);
   seekPosition += result;
	DEBUG('f', "MMMMWrited %d bytes at \n",numBytes);
   GetCurrentDate(writeDate, StringMaxLen);
   GetCurrentTime(writeTime, StringMaxLen);
   //writeTime = stats->totalTicks;

    Directory *directory = new Directory(12);
    char *name;

    DEBUG('f', "Opening file %s\n", name);
    directory->FetchFrom(fileSystem->directoryFile);
    name = directory->FindName(currentSector);
    int index = directory->FindIndex(name);
    strncpy(directory->table[index].lastUpdateDate, directory->DirGetCurrentDate(), StringMaxLen);
    strncpy(directory->table[index].lastUpdateTime, directory->DirGetCurrentTime(), StringMaxLen);
    directory->WriteBack(fileSystem->directoryFile); 

   return result;
}

int 	//Make sure that one time opening file only for one revoking this function
OpenFile::SysCallWrite(char *into, int numBytes)
{
	DEBUG('f', "BBBBBBWrited from %s in %d bytes at %d\n",into, numBytes, seekPosition);
   int result;
   result = SysCallWriteAt(into, numBytes, hdr->FileLength());
   GetCurrentDate(writeDate, StringMaxLen);
   GetCurrentTime(writeTime, StringMaxLen);
   //writeTime = stats->totalTicks;
   return result;
}
//----------------------------------------------------------------------
// OpenFile::ReadAt/WriteAt
// 	Read/write a portion of a file, starting at "position".
//	Return the number of bytes actually written or read, but has
//	no side effects (except that Write modifies the file, of course).
//
//	There is no guarantee the request starts or ends on an even disk sector
//	boundary; however the disk only knows how to read/write a whole disk
//	sector at a time.  Thus:
//
//	For ReadAt:
//	   We read in all of the full or partial sectors that are part of the
//	   request, but we only copy the part we are interested in.
//	For WriteAt:
//	   We must first read in any sectors that will be partially written,
//	   so that we don't overwrite the unmodified portion.  We then copy
//	   in the data that will be modified, and write back all the full
//	   or partial sectors that are part of the request.
//
//	"into" -- the buffer to contain the data to be read from disk 
//	"from" -- the buffer containing the data to be written to disk 
//	"numBytes" -- the number of bytes to transfer
//	"position" -- the offset within the file of the first byte to be
//			read/written
//----------------------------------------------------------------------

int
OpenFile::ReadAt(char *into, int numBytes, int position)
{
    int fileLength = hdr->FileLength();
    int i, firstSector, lastSector, numSectors;
    char *buf;

    if ((numBytes <= 0) || (position >= fileLength))
    	return 0; 				// check request
    if ((position + numBytes) > fileLength)		
	numBytes = fileLength - position;
    DEBUG('f', "Reading %d bytes at %d, from file of length %d.\n", 	
			numBytes, position, fileLength);

    firstSector = divRoundDown(position, SectorSize);
    lastSector = divRoundDown(position + numBytes - 1, SectorSize);
    numSectors = 1 + lastSector - firstSector;

    // read in all the full and partial sectors that we need
    buf = new char[numSectors * SectorSize];
    //printf("Start to ReadAt from %d to %d\n",firstSector, lastSector);
    for (i = firstSector; i <= lastSector; i++)	
        synchDisk->ReadSector(hdr->ByteToSector(i * SectorSize), 
					&buf[(i - firstSector) * SectorSize]);

    // copy the part we want
    bcopy(&buf[position - (firstSector * SectorSize)], into, numBytes);
    delete [] buf;
    return numBytes;
}

int
OpenFile::SysCallReadAt(char *into, int numBytes, int position)
{
    int fileLength = hdr->FileLength();
    int i, firstSector, lastSector, numSectors;
    char *buf;

    if ((numBytes <= 0) || (position >= SectorSize))
    	return 0; 				// check request
/*    if ((position + numBytes) > fileLength)		
	numBytes = fileLength - position;	*///don't need this any more
    DEBUG('f', "Reading %d bytes at %d, from file of length %d.\n", 	
			numBytes, position, fileLength);

    firstSector = divRoundDown(position, SectorSize);
    lastSector = divRoundDown(position + numBytes - 1, SectorSize);
    numSectors = 1 + lastSector - firstSector;

    // read in all the full and partial sectors that we need
    buf = new char[numSectors * SectorSize];
    //printf("Start to ReadAt from %d to %d\n",firstSector, lastSector);
        synchDisk->ReadSector(hdr->ByteToSector(0), buf);

    // copy the part we want
    bcopy(buf, into, numBytes);
    delete [] buf;
    return numBytes;
}
int
OpenFile::WriteAt(char *from, int numBytes, int position)
{
    int fileLength = hdr->FileLength();
    int i, firstSector, lastSector, numSectors;
    bool firstAligned, lastAligned;
    char *buf;

    if ((numBytes <= 0) || (position >= fileLength))
    {
	return 0;				// check request
    }
    if ((position + numBytes) > fileLength)
	numBytes = fileLength - position;
    DEBUG('f', "Writing %d bytes at %d, from file of length %d.\n", 	
			numBytes, position, fileLength);

    firstSector = divRoundDown(position, SectorSize);
    lastSector = divRoundDown(position + numBytes - 1, SectorSize);
    numSectors = 1 + lastSector - firstSector;

    buf = new char[numSectors * SectorSize];

    firstAligned = (position == (firstSector * SectorSize));
    lastAligned = ((position + numBytes) == ((lastSector + 1) * SectorSize));

// read in first and last sector, if they are to be partially modified
    if (!firstAligned)
        ReadAt(buf, SectorSize, firstSector * SectorSize);	
    if (!lastAligned && ((firstSector != lastSector) || firstAligned))
        ReadAt(&buf[(lastSector - firstSector) * SectorSize], 
				SectorSize, lastSector * SectorSize);	

// copy in the bytes we want to change 
    bcopy(from, &buf[position - (firstSector * SectorSize)], numBytes);

// write modified sectors back
    for (i = firstSector; i <= lastSector; i++)	
        synchDisk->WriteSector(hdr->ByteToSector(i * SectorSize), 
					&buf[(i - firstSector) * SectorSize]);
    delete [] buf;
    return numBytes;
}

int
OpenFile::SysCallWriteAt(char *from, int numBytes, int position)
{
	DEBUG('f', "CCCCwrited %d bytes at \n",numBytes);
    int fileLength = hdr->FileLength();
    int i, firstSector, lastSector, numSectors;
    bool firstAligned, lastAligned;
    char *buf;

    if ((numBytes <= 0) || (position >= SectorSize))
    {
	DEBUG('f', "STOP! %d <=0 or %d >= %d",numBytes, position, SectorSize);
	printf("Arrived the Maxinum Size:128 Bytes!\n");
	return 0;				// check request
    }
DEBUG('f', "Going ON!!! \n");
DEBUG('f', "Writing %d bytes at %d, from file of length %d.\n",numBytes, position, fileLength); 	
    firstSector = divRoundDown(position, SectorSize);
    lastSector = divRoundDown(position + numBytes - 1, SectorSize);
    numSectors = 1 + lastSector - firstSector;
DEBUG('f', "TTTTT numSectors update to %d \n",numSectors); 	
    hdr->UpdateNumSectors(numSectors);
    hdr->WriteBack(this->currentSector);	//Only SysCall operations can modify file header data on the disk
DEBUG('f', "PPPPPP write back hdr to disk, sector: %d\n",currentSector); 	

DEBUG('f', "numSectors added to %d\n",numSectors); 	
    buf = new char[SectorSize];

    SysCallReadAt(buf, SectorSize, position);
DEBUG('f', "RRRRRRead '%s' from disk, start copy postion is %d, numSectors = %d, copy size is %d\n",buf, position, numSectors, numBytes); 	
    if(position + numBytes >= SectorSize)
	    numBytes = SectorSize - position;
    bcopy(from, &buf[position], numBytes);
    DEBUG('f', "EEEEEEEEEEEEE,buf is \n%s\n",buf);

// read in first and last sector, if they are to be partially modified

// copy in the bytes we want to change 
//    bcopy(from, buf, numByts);

// write modified sectors back
	DEBUG('f', "Sector is  %d bytes at \n",hdr->ByteToSector(0));
    DEBUG('f', "FFFFFFFFFFF\n");
    synchDisk->WriteSector(hdr->ByteToSector(0), buf);
    delete [] buf;
    return numBytes;
}
//----------------------------------------------------------------------
// OpenFile::Length
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int
OpenFile::Length() 
{ 
    return hdr->FileLength(); 
}

void
OpenFile::IncreaseLength(int Bytes)
{
	hdr->IncreaseBytes(Bytes);
}

