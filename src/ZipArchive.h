#ifndef ZIPARCHIVE_H
#define ZIPARCHIVE_H

#include "dojostring.h"
#include <string>
#include <vector>
#include <zzip/zzip.h>

namespace Dojo{

	class ZipArchive;
	class ZipFile;

	class ZipFile
	{       
	public:

        friend class ZipArchive;

                ~ZipFile();
                //close file
                void close();
                //read from file
                size_t read(void * ptr, size_t size, size_t count);
                //seek from file
                int seek (long int offset, int origin );
                //tell from file
                long int tell ();
                //get file size
                long int size();
                //return a uchar cast in int
                int getc();
                //rewind from file
                void rewind ();
                //write to file
                size_t write ( const void * ptr, size_t size, size_t count);
       
	private:
        
                ZipFile(ZZIP_FILE* file);
                ZZIP_FILE* file;
	};

	class ZipArchive
	{
	public:
	
                ZipArchive();
                ZipArchive(const String& path);
                ~ZipArchive();
                //open zip file
                bool open(const String& path);
                //close zip file
                void close();
                //open file
                ZipFile* openFile(const String& path,const String& mode);
                //paths and files in zip
                void getList(String path,std::vector<String>& out);
                void getListFiles(String path,std::vector<String>& out);
                void getListSubDirectories(String path,std::vector<String>& out);

                void getListAll(String path,std::vector<String>& out);
                void getListAllFiles(String path,std::vector<String>& out);
                void getListAllSubDirectories(String path,std::vector<String>& out);

	private:

                void madeValidPath(String& path);
                ZZIP_DIR* zip_file;

	};


};


#endif
