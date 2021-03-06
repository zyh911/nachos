// directory.h 
//	Data structures to manage a UNIX-like directory of file names.
// 
//      A directory is a table of pairs: <file name, sector #>,
//	giving the name of each file in the directory, and 
//	where to find its file header (the data structure describing
//	where to find the file's data blocks) on disk.
//
//      We assume mutual exclusion is provided by the caller.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
#include "system.h"
#include "synch.h"


#include "copyright.h"

#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "openfile.h"

#define FileNameMaxLen 		256	// for simplicity, we assume 
					// file names are <= 9 characters long

// The following class defines a "directory entry", representing a file
// in the directory.  Each entry gives the name of the file, and where
// the file's header is to be found on disk.
//
// Internal data structures kept public so that Directory operations can
// access them directly.

class DirectoryEntry {
  public:
    DirectoryEntry(){
	    strncpy(createDate, "0000-00-00", StringMaxLen);	  
	    strncpy(lastAccessDate, "0000-00-00", StringMaxLen);	  
	    strncpy(lastUpdateDate, "0000-00-00", StringMaxLen);
	    //createTime = 0;
	    //lastAccessTime = 0;
	    //lastUpdateTime = 0;	  
	    strncpy(createTime, "00:00:00", StringMaxLen);
	    strncpy(lastAccessTime, "00:00:00", StringMaxLen);
	    strncpy(lastUpdateTime, "00:00:00", StringMaxLen);
	    fileSize = 0;
    };
    bool inUse;				// Is this directory entry in use?
    int sector;				// Location on disk to find the 
					//   FileHeader for this file 
    char name[FileNameMaxLen + 1];	// Text name for file, with +1 for 
					// the trailing '\0'
    char fileType;			// file '-' or folder 'd'
    char createDate[StringMaxLen];	
    char createTime[StringMaxLen];
    char lastAccessTime[StringMaxLen];
    char lastAccessDate[StringMaxLen];
    char lastUpdateTime[StringMaxLen];
    char lastUpdateDate[StringMaxLen];
    int	 fileSize;
    char fName[FileNameMaxLen + 1];	//father folder name
    int FID;
    
    //-------------------------------------------
};

// The following class defines a UNIX-like "directory".  Each entry in
// the directory describes a file, and where to find it on disk.
//
// The directory data structure can be stored in memory, or on disk.
// When it is on disk, it is stored as a regular Nachos file.
//
// The constructor initializes a directory structure in memory; the
// FetchFrom/WriteBack operations shuffle the directory information
// from/to disk. 

class Directory {
  public:
    Directory(int size); 		// Initialize an empty directory
					// with space for "size" files
    ~Directory();			// De-allocate the directory

    void FetchFrom(OpenFile *file);  	// Init directory contents from disk
    void WriteBack(OpenFile *file);	// Write modifications to 
					// directory contents back to disk

    int Find(char *name);		// Find the sector number of the 
					// FileHeader for file: "name"
    char* FindName(int sector);
    bool Add(char *name, int newSector, char filetype); 

    bool Remove(char *name);		// Remove a file from the directory
    void RemoveAll();

    void List();			// Print the names of all the files
					//  in the directory
    void ListFile(char filename[]);	// Print the certain file's all info
    					// including filetype, currenttime, updtetime,
    					// accesstime, filename, filesize	--edit by YePeng

    void Print();			// Verbose print of the contents
					//  of the directory -- all the file
					//  names and their contents.
    char *DirGetCurrentDate();		// Get the DirectoryEntry's current date --edit by YePeng
    char *DirGetCurrentTime();		// Get the DirectoryEntry's current time --edit by YePeng
    void UpdateFileSize(char str[], int initialSize);	//edit by YePeng
    bool UpdateLastAccessTimestamp(char filename[], char lastTime[], char lastDate[]);	//edit by YePeng
    bool UpdateLastUpdateTimestamp(char filename[], char lastTime[], char lastDate[]);	//edit by YePeng
    int  GetCurrentSectorNum();
    char *PrintName();
    void RecordName(char name[]);
    void GetPath(char path[]);

    void IncreaseFileSize(char *name, int Bytes);		// Increase the file size 
 // private:
    int tableSize;			// Number of directory entries
    DirectoryEntry *table;		// Table of pairs: 
					// <file name, file header location> 
    int totalFID;

    int FindIndex(char *name);		// Find the index into the directory 
					//  table corresponding to "name"
    int currentSectorNum;		// edit by YePeng
};

#endif // DIRECTORY_H

