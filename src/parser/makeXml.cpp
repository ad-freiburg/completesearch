// Copyright 2010 Hannah Bast.

#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>

using std::cout;
using std::endl;

// USAGE INFO
void printUsage()
{
  cout << endl
       << "Usage: makeXml <list of files or directories> [options]"
       << endl << endl
       << "Options: [-f file name] [-r root tag] [-d document tag] "
       <<          "[-s source tag] [-e encoding]"
       << endl << endl
       << "produces a single xml from the given files. "
       << "The default file name, xml root tag name,"
       << endl
       << "document tag name, and source tag name are collection.xml, "
       << "collection, document, and"
       << endl
       << "sourse, respectively. The default encoding is iso-8859-1, "
       << "alternatively utf-8 can be"
       << endl
       << "chosen. Directories are explored recursively."
       << endl << endl
       << "NEW 25Sep07: each control character with ASCII code below 32, with "
       << "the execption of tab,"
       << endl
       << "newline, and carriage return, is replaced by a space (to ensure "
       << "that the xml is well-formed)."
       << endl << endl
       << "Example: makeXml *.txt -f mail.xml -x mails -d mail -s src -e utf-8"
       << endl << endl;
}

const char XML_FILE_NAME[]  = "collection.xml";
const char ROOT_TAG[]     = "collection";
const char DOCUMENT_TAG[] = "document";
const char SOURCE_TAG[]   = "source";
const char ENCODING[]     = "iso-8859-1";
std::vector<std::string> documentFileNames;

// ADD FILE OR FILES IN DIRECTORY (recursively)
void addFiles(std::string fileName, std::string path,
              std::vector<std::string> *fileNames);

// MAIN
int main(int argc, char** argv)
{
  std::string xmlFileName, rootTag, documentTag, sourceTag, encoding;

  xmlFileName = XML_FILE_NAME;
  rootTag = ROOT_TAG;
  documentTag = DOCUMENT_TAG;
  sourceTag = SOURCE_TAG;
  encoding = ENCODING;

  // parse options
  while (true)
  {
    int c = getopt(argc, argv, "f:r:d:s:e:");
    if (c == -1) break;
    switch (c)
    {
      case 'f': xmlFileName = optarg;
                break;
      case 'r': rootTag = optarg;
                break;
      case 'd': documentTag = optarg;
                break;
      case 's': sourceTag = optarg;
                break;
      case 'e': encoding = optarg;
                break;
      default: printUsage();
               exit(1);
    }
  }
  // take remaining arguments as file names; if a file name
  // if a directory, add the names of all files in it
  off_t timer = time(NULL);
  while (optind < argc)
  {
    const char* file_name = argv[optind++];
    addFiles(file_name, "", &documentFileNames);
  }
  if (documentFileNames.size() == 0)
  {
    printUsage();
    exit(1);
  }
  std::cerr << "adding " << documentFileNames.size() << " files ... "
            << std::flush;


  // read documents and write xml
  std::ofstream xmlFile(xmlFileName.c_str());
  std::string line;
  xmlFile << "<?xml version=\"1.0\" encoding=\""
          << encoding << "\" standalone=\"yes\"?>" << endl;
  xmlFile << "<" << rootTag << ">" << endl;

  // cout << "forall document filenames( " << std::flush;

  for (unsigned int i = 0; i < documentFileNames.size(); ++i)
  {
    std::string& documentFileName = documentFileNames[i];
    std::ifstream documentFile(documentFileName.c_str());
    if (not documentFile.peek()) continue;
    documentFile >> std::noskipws;
    xmlFile << "<" << documentTag << ">" << endl
            << "<" << sourceTag << "><![CDATA[" << documentFileName
            << "]]></" << sourceTag << ">" << endl
            << "<text><![CDATA[" << endl;
    while (getline(documentFile, line))
    {
      // cout << "Check for control characters(..." << std::flush;
      // check for control characters
      for (unsigned int i = 0; i < line.size(); ++i)
      {
        if (line[i] >= 0 && line[i] < 32
              && line[i] != '\n' && line[i] != '\t' && line[i] != '\r')
          line[i] = ' ';
      }
      // check for ]]>
      // TODO(hannah): This is a bit inefficient, the searches after the first
      // search start from the beginning of the string again.
      while (true)
      {
        std::string::size_type pos = line.find("]]>");
        if (pos == std::string::npos) break;
        line[pos] = ')';
        std::cerr << "WARNING: document \"" << documentFileName
                  << "\" contains \"]]>\""
                  << ", replaced it by \")]>\" (temporary hack)" << endl;
      }
      xmlFile << line << endl;
      // cout << ")";
    }
    documentFile.close();
    xmlFile << "]]></text>" << endl
            << "</" << documentTag << ">" << endl;
  }
  xmlFile << "</" << rootTag << ">" << endl;
  xmlFile.close();
  timer = time(NULL) - timer;
  // std::cout << ") ";
  std::cerr << "done in " << timer << " seconds" << endl;
}


// ADD FILE OR FILES IN DIRECTORY (recursively)
void addFiles(std::string fileName, std::string path,
              std::vector<std::string> *fileNames)
{
  if (path.size() > 0 && path[path.size() - 1] != '/') path += '/';
  fileName = path + fileName;
  DIR* dir = opendir(fileName.c_str());
  struct dirent entry;
  struct dirent *result;
  int return_code;

  // Case: ordinary file
  if (dir == NULL)
  {
    fileNames->push_back(fileName);
  }

  // Case: is directory
  else
  {
    for (return_code = readdir_r(dir, &entry, &result);
         result != NULL && return_code == 0;
         return_code = readdir_r(dir, &entry, &result))
    {
      if (return_code != 0)
        perror("readdir_r() error");
      if (entry.d_name[0] == '.') continue;
      printf("  %s\n", entry.d_name);
      addFiles(entry.d_name, fileName, fileNames);
    }
  }
}
