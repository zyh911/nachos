// directory.cc 
//	Routines to manage a directory of file names.
//
//	The directory is a table of fixed length entries; each
//	entry represents a single file, and contains the file name,
//	and the location of the file header on disk.  The fixed size
//	of each directory entry means that we have the restriction
//	of a fixed maximum size for file names.
//
//	The constructor initializes an empty directory of a certain size;
//	we use ReadFrom/WriteBack to fetch the contents of the directory
//	from disk, and to write back any modifications back to disk.
//
//	Also, this implementation has the restriction that the size
//	of the directory cannot expand.  In other words, once all the
//	entries in the directory are used, no more files can be created.
//	Fixing this is one of the parts to the assignment.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "utility.h"
#include "filehdr.h"
#include "directory.h"

#include "system.h"
extern void GetCurrentDate(char str[], int strlength);
extern void GetCurrentTime(char str[], int strlength);
//----------------------------------------------------------------------
// Directory::Directory
// 	Initialize a directory; initially, the directory is completely
//	empty.  If the disk is being formatted, an empty directory
//	is all we need, but otherwise, we need to call FetchFrom in order
//	to initialize it from disk.
//
//	"size" is the number of entries in the directory
//----------------------------------------------------------------------


Directory::Directory(int size)
{
    currentSectorNum = 0;
    table = new DirectoryEntry[size];
    tableSize = size;

    table[0].inUse = table[1].inUse = TRUE;
    table[0].fileSize = table[1].fileSize = 4096;
    strcpy(table[0].name , ".");
    strcpy(table[0].fName, "root");
    strcpy(table[1].name , "..");
    strcpy(table[1].fName, "root");
    table[0].fileType = table[1].fileType = 'd';
    totalFID = 1;

    for (int i = 2; i < tableSize; i++)
	table[i].inUse = FALSE;
}

//----------------------------------------------------------------------
// Directory::~Directory
// 	De-allocate directory data structure.
//----------------------------------------------------------------------

Directory::~Directory()
{ 
    delete [] table;
} 

//----------------------------------------------------------------------
// Directory::FetchFrom
// 	Read the contents of the directory from disk.
//
//	"file" -- file containing the directory contents
//----------------------------------------------------------------------

void
Directory::FetchFrom(OpenFile *file)
{
    (void) file->ReadAt((char *)table, tableSize * sizeof(DirectoryEntry), 0);
    //(void) file->ReadAt((char *)(&totalFID), 4, tableSize * sizeof(DirectoryEntry));
}

//----------------------------------------------------------------------
// Directory::WriteBack
// 	Write any modifications to the directory back to disk
//
//	"file" -- file to contain the new directory contents
//----------------------------------------------------------------------

void
Directory::WriteBack(OpenFile *file)
{
    (void) file->WriteAt((char *)table, tableSize * sizeof(DirectoryEntry), 0);
    //(void) file->WriteAt((char *)(&totalFID), 4, tableSize * sizeof(DirectoryEntry));
}

//----------------------------------------------------------------------
// Directory::FindIndex
// 	Look up file name in directory, and return its location in the table of
//	directory entries.  Return -1 if the name isn't in the directory.
//
//	"name" -- the file name to look up
//----------------------------------------------------------------------

int
Directory::FindIndex(char *name)
{
    for (int i = 2; i < tableSize; i++)		//edit by YePeng
    {
        if (table[i].inUse && !strncmp(table[i].name, name, FileNameMaxLen))
	    return i;
    }
    return -1;		// name not in directory
}

//----------------------------------------------------------------------
// Directory::Find
// 	Look up file name in directory, and return the disk sector number
//	where the file's header is stored. Return -1 if the name isn't 
//	in the directory.
//
//	"name" -- the file name to look up
//----------------------------------------------------------------------

int
Directory::Find(char *name)
{
    int i = FindIndex(name);

    if (i != -1)
	return table[i].sector;
    return -1;
}

void
Directory::IncreaseFileSize(char *name, int Bytes)
{
	int i = FindIndex(name);
	if(i!=-1)
		table[i].fileSize += Bytes;
}
//----------------------------------------------------------------------
// Directory::Add
// 	Add a file into the directory.  Return TRUE if successful;
//	return FALSE if the file name is already in the directory, or if
//	the directory is completely full, and has no more space for
//	additional file names.
//
//	"name" -- the name of the file being added
//	"newSector" -- the disk sector containing the added file's header
//----------------------------------------------------------------------

bool
Directory::Add(char *name, int newSector, char filetype)	//edit by YePeng
{ 
    if (FindIndex(name) != -1)
	return FALSE;

    for (int i = 0; i < tableSize; i++)
        if (!table[i].inUse) {
	    table[i].FID = totalFID;
	    totalFID ++;
            table[i].inUse = TRUE;
            strncpy(table[i].name, name, FileNameMaxLen); 
            table[i].sector = newSector;
	    table[i].fileType = filetype;	//edit by YePeng
	    strcpy(table[i].fName, table[0].fName);
	    strncpy(table[i].createDate, DirGetCurrentDate(), StringMaxLen);
	    strncpy(table[i].createTime, DirGetCurrentTime(), StringMaxLen);
	    strncpy(table[i].lastAccessDate, DirGetCurrentDate(), StringMaxLen);
	    strncpy(table[i].lastAccessTime, DirGetCurrentTime(), StringMaxLen);
	    strncpy(table[i].lastUpdateDate, DirGetCurrentDate(), StringMaxLen);
	    strncpy(table[i].lastUpdateTime, DirGetCurrentTime(), StringMaxLen);
	    //GetPath(table[i].currentPath);
        return TRUE;
	}
    return FALSE;	// no space.  Fix when we have extensible files.
}

char* Directory::FindName(int sector)
{
	for (int i = 0; i < tableSize; i++)
        	if (table[i].sector == sector)
			return table[i].name;
}
//----------------------------------------------------------------------
// Directory::Remove
// 	Remove a file name from the directory.  Return TRUE if successful;
//	return FALSE if the file isn't in the directory. 
//
//	"name" -- the file name to be removed
//----------------------------------------------------------------------

bool
Directory::Remove(char *name)
{ 
    int i = FindIndex(name);

    if (i == -1)
	return FALSE; 		// name not in directory
    table[i].inUse = FALSE;
    return TRUE;	
}

void
Directory::RemoveAll()
{
	int i;
	for(i=0;i<tableSize;i++)
		table[i].inUse = FALSE;
}
//----------------------------------------------------------------------
// Directory::List
// 	List all the file names in the directory. 
//----------------------------------------------------------------------

void
Directory::List()
{
   char str[16];
   for (int i = 0; i < tableSize; i++)
	if ((table[i].inUse) && (strcmp(table[0].fName, table[i].fName)==0))
	{
	    printf("FID:%d\t type:%c\t createdTime:%s\t lastAccessTime:%s\t lastUpdateTime:%s\t path:%s\t size:%d\t\t name:%s\n", table[i].FID, table[i].fileType, table[i].createTime, table[i].lastAccessTime, table[i].lastUpdateTime, table[i].fName, table[i].fileSize, table[i].name);
	}
}


//----------
// Directory::ListFile
//	List all the informations of the certain file
//------------
void
Directory::ListFile(char filename[])
{
	int i;
	if((i=FindIndex(filename))!=-1)
	{
	    printf("%c\t %s\t", table[i].fileType, table[i].createTime);
	    printf("%s\t", table[i].lastAccessTime);
	    printf("%s\t%s\n", table[i].lastUpdateTime, table[i].name);
	}
}
//------------
// Directory::UpdateLastAccessTimestamp
//	Update the last access timestamp onto the directory  entry 
//-----------------------
bool
Directory::UpdateLastAccessTimestamp(char filename[], char lastTime[], char lastDate[])
{
   for (int i = 0; i < tableSize; i++)
	if (table[i].inUse)
		if (strcmp(table[i].name, filename)==0)
		{
			strncpy(table[i].lastAccessTime, lastTime, StringMaxLen);
            		strncpy(table[i].lastAccessDate, lastDate, StringMaxLen);
	    		//printf("%c\t%s %s\t%s %s\t%s %s\t%s\n", table[i].filetype, table[i].createDate, table[i].createTime, table[i].lastAccessDate, table[i].lastAccessTime, table[i].lastUpdateDate, table[i].lastUpdateTime, table[i].name);
			return true;
		}
   return false;
}

//-------------------
// Directory::UpdateLastUpdateTimestamp
//	Update the last written timestamp onto the directory entry
//------------------------
bool
Directory::UpdateLastUpdateTimestamp(char filename[], char lastTime[], char lastDate[])
{
   for (int i = 0; i < tableSize; i++)
	if (table[i].inUse)
		if (strcmp(table[i].name, filename)==0)
		{
			//table[i].lastUpdateTime = lastTime;
			strncpy(table[i].lastUpdateTime, lastTime, StringMaxLen);
            		strncpy(table[i].lastUpdateDate, lastDate, StringMaxLen);
			return true;
		}
   return false;
}
//----------------------------------------------------------------------
// Directory::Print
// 	List all the file names in the directory, their FileHeader locations,
//	and the contents of each file.  For debugging.
//----------------------------------------------------------------------

void
Directory::Print()
{ 
    FileHeader *hdr = new FileHeader;

    printf("Directory contents:\n");
    for (int i = 0; i < tableSize; i++)
	if (table[i].inUse) {
	    printf("Name: %s, Sector: %d\n", table[i].name, table[i].sector);
	    hdr->FetchFrom(table[i].sector);
	    hdr->Print();
	}
    printf("\n");
    delete hdr;
}

//-----------------------------------
// Directory::DirGetCurrentDate
//	Get the current date data
//	return:
//		the date strings address
//------------------------------------
char
*Directory::DirGetCurrentDate()
{
	char str[StringMaxLen];
	GetCurrentDate(str,StringMaxLen);
	return str;
       	
}

//-------------------------------
// Directory::DirGetCurrentTime
//	Get the current time data
//	return:
//		the time strings address
//----------------------------------
char
*Directory::DirGetCurrentTime()
{
	char str[StringMaxLen];
	GetCurrentTime(str,StringMaxLen);
	return str;
}

//-----------------------
// Directory::UpdateFileSize
//	The operation for updating fileSize in directory entry
//----------------------
void
Directory::UpdateFileSize(char str[], int initialSize)
{
	int i;
	if((i = FindIndex(str)) != -1)
		table[i].fileSize = initialSize;
}

int 
Directory::GetCurrentSectorNum()
{
	return currentSectorNum;
}


char
*Directory::PrintName()
{
	return table[0].fName;
}

//------------------------------------
// Directory::RecordName
//	Change the current folder name to the given name
//	table[0]	-- "." file
//	table[1]	-- ".." file
//	.fName		-- parent folder name
//--------------------------------------
void
Directory::RecordName(char name[])
{
	int i;
	if( strcmp(name,"..") == 0 )
	{
		if((i = FindIndex(table[0].fName)) != -1)
		{
			printf("i=%d\n",i);
			strncpy(table[0].fName, table[i].fName, FileNameMaxLen + 1);
			strncpy(table[1].fName, table[i].fName, FileNameMaxLen + 1);
		}
			
	}
	else
	{
		strncpy(table[0].fName, name, FileNameMaxLen + 1);
		strncpy(table[1].fName, name, FileNameMaxLen + 1);
	}
}

//--------------------------------------
// Directory::GetPath
//	Collect the whole strings of the current path onto the string array
//	path[], the length of this array must be defined on FileNameMaxLen+1
//---------------------------------------
void
Directory::GetPath(char path[])
{
	char name[10][FileNameMaxLen + 1];
	int i;
	int index = 0;
	for(i=0;i<10;i++)
	{
		if(strcmp(table[index].fName, "root") == 0)
			break;
		strcpy(name[i], table[index].fName);
		index = FindIndex(table[index].fName);
	}
	if(i!=0)
		i--;
	for(i;i>=0;i--)
	{
		strcat(path, "/");
		strcat(path, name[i]);
	}
}

