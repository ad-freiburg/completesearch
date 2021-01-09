#ifndef __EXCERPTSDB_H__
#define __EXCERPTSDB_H__

#include "Globals.h"
#include <iostream>
#include <fstream>
#include <string>
#include <assert.h>
#include <iomanip>
      
#define MAX_LINE_LENGTH_EXCERPT          100000000

////namespace aucmpl {

//! Datastructure for the data-field in an ExcerptsDB.
/*!
 *  In an ExcerptsDB the keys are document ids, represented by numbers of type \c unsigned \c long, and
 *  the data stored under a key is of type ExcerptData. This comprises a url, a title, and an excerpt.
 */


using namespace std;

class ExcerptData
{
  private:

    //! Contains the url.
    string _url;

    //! Contains the title.
    string _title;

    //! Contains the excerpt.
    string _excerpt;

    //! Used for scoring search results.
    unsigned int _score;

  public:

    //! The default constructor
    /*!
     *  Returns an object with empty url, title and excerpt.
     */
    ExcerptData() : _url(""), _title(""), _excerpt(""), _score(0) {}

    
    //! Constructs an object from a null-terminated char*.
    /*!
     *  \param 		data 		The pointer to the null-terminated string.
     *
     *  The string must contain exactly two '\\t'-characters. Everything before the first '\\t' is taken
     *  as the url, everything between the first and the second '\\t' as the title, and everything
     *  after the second '\\t' as the excerpt.
     */
    ExcerptData(const char* data) { set(data); }


    //! Construct an object form a not null-terminated char*.
    /*!
     *  \param 		data 		The pointer to the string.
     *  \param 		n 		The length of the string.
     *
     *  The string must contain exactly two '\\t'-characters. Everything before the first '\\t' is taken
     *  as the url, everything between the first and the second '\\t' as the title, and everything
     *  after the second '\\t' as the excerpt.
     */
    ExcerptData(const char* data, const unsigned int& n) { set(data, n); }
    

    //! Initializes \c *this from a null-terminated char*.
    /*!
     *  \param 		_data 		The pointer to the null-terminated string.
     *
     *  The string must contain exactly two '\\t'-characters. Everything before the first '\\t' is taken
     *  as the url, everything between the first and the second '\\t' as the title, and everything
     *  after the second '\\t' as the excerpt.
     */
    void set(const char* _data)
    {
      string data(_data);
      set(data);
    }


    //! Initializes \c *this from a not null-terminated char*.
    /*!
     *  \param 		_data 		The pointer to the string.
     *  \param 		n 		The length of the string.
     *
     *  The string must contain exactly two '\\t'-characters. Everything before the first '\\t' is taken
     *  as the url, everything between the first and the second '\\t' as the title, and everything
     *  after the second '\\t' as the excerpt.
     */
    void set(const char* _data, const unsigned int& n)
    {
      string data(_data, n);
      set(data);
    }

    
    //! Initializes \c *this from a \c string.
    /*!
     *  \param 		data 		The string to initialize \c *this with.
     *
     *  The string must contain exactly two '\\t'-characters. Everything before the first '\\t' is taken
     *  as the url, everything between the first and the second '\\t' as the title, and everything
     *  after the second '\\t' as the excerpt.
     */
    void set(const string& data)
    {
      size_t tab1 = data.find('\t');
      if (tab1 == string::npos)
      {
	cerr << "ExcerptData::set(const char* data): Format of data incorrect (no \'\\t\' found)" << endl;
	cerr << "String was: " << data << endl;
	exit(0);
      }
      _url = data.substr(0, tab1);

      size_t tab2 = data.find('\t', tab1+1);
      if (tab2 == string::npos)
      {
	cerr << "ExcerptData::set(const char* data): Format of data incorrect (only one \'\\t\' found)" << endl;
	cerr << "String was: " << data << endl;
	exit(0);
      }
      _title = data.substr(tab1+1, tab2-tab1-1);

      size_t tab3 = data.find('\t', tab2+1);
      if (tab3 == string::npos)
      {
	_excerpt = data.substr(tab2+1);
	_score = 0;
      }
      else
      {
	_excerpt = data.substr(tab2+1, tab3-tab2-1);
	if (data.length()-tab3-1 != sizeof(int))
	{
	  cerr << "ExcerptData::set(const char* data): Document score of size " << sizeof(int) << " bytes expexted" << endl
		    << "Got " << data.length()-tab3-1 << " bytes." << endl;
	  exit(0);
	}
	_score = *((unsigned int*)(data.c_str() + tab3+1));
      }

//      if (data.find('\t', tab2+1) != string::npos)
//      {
//	cerr << "ExcerptData::set(const char* data): Format of data incorrect (more than two \'\\t\' found)" << endl;
//	cerr << "String was: " << data << endl;
//	exit(0);
//      }
//      _excerpt = data.substr(tab2+1);

//      _score = 0;
    }


    //! Initializes \c *this from three strings.
    /*!
     *  \param 		url 		The string containing the url.
     *  \param 		title 		The string containing the title.
     *  \param 		excerpt 	The string containing the excerpt.
     */
    void set(const string& url, const string& title, const string& excerpt, unsigned int score = 0)
    {
      _url = url;
      _title = title;
      _excerpt = excerpt;
      _score = score;
    }


    //! Sets the url.
    void setUrl(const string& url) { _url = url; }


    //! Sets the title.
    void setTitle(const string& title) { _title = title; }


    //! Sets the excerpt.
    void setExcerpt(const string& excerpt) { _excerpt = excerpt; }


    //! Sets the score.
    void setScore(unsigned int score) { _score = score; }


    //! Returns the url as string.
    const string& url() const { return _url; }


    //! Returns the title as string.
    const string& title() const { return _title; }


    //! Returns the excerpt as string.
    const string& excerpt() const { return _excerpt; }


    //! Returns the score.
    const unsigned int& score() const { return _score; }


    //! Conversion to a string.
    /*!
     *  Returns a string containing the url, the title and the excerpt, in that order,
     *  separated by a '\\t'.
     */
    const string str() const
    {
      string s = _url + '\t' + _title + '\t' + _excerpt + '\t';
      for (size_t i=0; i<sizeof(int); i++) s += ((char*)(&_score))[i];
      return s;
    }
}; // class ExcerptData


//! A database-handle for an excerpts-database.
class ExcerptsDB_NEW
{
  private:

    //! The filehandle.
    FILE* _file;

    FILE* _offsetsFile;


    //! The name of the database file.
    string _filename;


    //! The access mode.
    /*!
     *  Can take one of the values READ, WRITE and RANDOM.
     */
    // Oct 2006 (Deb) Mode is always READ, one may at most need to create
    // .docoffsets file if it does not exist
    // char _mode;


    bool _isEof;


    unsigned int _nofOffsets;


    off_t* _offsets;


    //! Variable for returning data as reference.
    mutable ExcerptData _returnData;



  public:

    //! The default constructor.
    ExcerptsDB_NEW() { _file = NULL; _offsetsFile = NULL; }
    

    //! Constructor taking a filename, a cache size and an access mode.
    /*!
     *  Calls the function open().
     */
    ExcerptsDB_NEW(const string& docsFileName, const string& docOffsetsFileName)
    {
      open(docsFileName, docOffsetsFileName);
    }

    void open(const string& docsFileName, const string& docOffsetsFileName = "")
    {
       if (isOpen()) close();
      _filename = docsFileName;
      _file = fopen(docsFileName.c_str(), "r");
      if (_file == NULL) { cerr << "fopen \"" << docsFileName << "\" (" << strerror(errno) << ")" << endl << endl; exit(1); }
      _isEof = false;
      openOffsetsFile(docOffsetsFileName != "" ? docOffsetsFileName : docsFileName + ".offsets");
    }

    void openOffsetsFile(const string& docOffsetsFileName)
    {
      _offsetsFile = fopen(docOffsetsFileName.c_str(), "r");
      if (_offsetsFile == NULL) { cerr << "fopen \"" << docOffsetsFileName << "\" (" << strerror(errno) << ")" << endl << endl; exit(1); }
      // if (_offsetsFile == NULL) 
      // {
      //   char ans;
      //   cout << "The offsets file to support the excerpts does not exist. Do you want to create it? [Y/N]: " << flush;
      //   cin >> ans;
      //   if (ans == 'Y' || ans == 'y') buildOffsets(_filename, docOffsetsFileName);
      //   else { cout << "OK, Thank you. But then I must quit, as without that file excerpts cannot be supported." << endl; }
      //   // And now open offsetsFile again
      //   _offsetsFile = fopen(docOffsetsFileName.c_str(),"r"); 
      // }

      // Load offsets
      //   1. first get filesize and determine total number of offsets 
      if (fseeko(_offsetsFile,(off_t)0,SEEK_END) != 0) { cerr << "Could not go to end of file " << (_filename+".offsets") << endl; std::exit(1); }
      off_t fileSize = ftello(_offsetsFile);
      if (fileSize % sizeof(off_t) != 0) { cerr << "ERROR! Size of docOffsets file (" << fileSize << ") must be a multiple of " << sizeof(off_t) << endl; std::exit(1); }

      _nofOffsets = fileSize/sizeof(off_t);

      // go to beginning of file again
      if (fseeko(_offsetsFile,(off_t)0,SEEK_SET) != 0) { cerr << "Could not go to beginning of file " << (_filename+".offsets") << endl; std::exit(1); }

      // read the whole file
      _offsets = (off_t*)malloc(fileSize);

      if (fread((void*)_offsets,(size_t)1,fileSize,_offsetsFile)!=fileSize) { cerr << "ERROR! Could not read offsets." << endl; std::exit(1); }

      // Test
      // for (unsigned int i = 1; i <= _nofOffsets; i+=1000)
      // {
      //   cout << "Offset of docId " << i << ": " << _offsets[i-1] << endl;
      // }

      assert(isOffsetsOpen());
    }


    bool isOffsetsOpen() { return (_offsetsFile != NULL); }

    bool isOpen() { return ((_file != NULL) || (_offsetsFile != NULL)); }

    bool isEof() { return _isEof; }


    //! build a file which stores the offset of each document beginning
    //! (each document consists of one line)
    void buildOffsets(const string& docsFileName, const string& docOffsetsFileName, size_t MAX_DOCID_IN_DOCUMENT = 26000000) 
    {
      // const size_t MAX_LINE_LENGTH_EXCERPT = 100000000;

      FILE* docs_file = fopen(docsFileName.c_str(), "r");
      FILE* offsets_file = fopen(docOffsetsFileName.c_str(), "w");
      off_t* offsets = (off_t*)malloc(MAX_DOCID_IN_DOCUMENT*sizeof(off_t));
      char* line = (char*)malloc(MAX_LINE_LENGTH_EXCERPT+1);

      unsigned int lineNumber = 0;

      DocId docId=0;
      DocId maxDocId=0;

      off_t beginOffset;
      // go to beginning of file
      if (fseeko(docs_file ,(off_t)0,SEEK_SET) != 0) { cerr << "Could not go to beginning of file " << docsFileName << endl; std::exit(1); }
      while (true)
      {
	if (isEof()) break;
	beginOffset = ftello(docs_file);

        // GET NEXT LINE
        _isEof = ( fgets(line, MAX_LINE_LENGTH_EXCERPT+1, docs_file) == NULL );
        lineNumber++;
        
        // GET DOCID (rest of the line is ignored for this step)
        if (sscanf(line, "%lu", &docId) < 1) 
        { cerr << "Wrong line format in line: " << lineNumber << endl; std::exit(1); }

        // Update maxDocId if necessary 
        // (required only if docids are not consecutive)
        if (docId > maxDocId) maxDocId = docId;
	assert(docId <= MAX_DOCID_IN_DOCUMENT);

        offsets[docId-1] = beginOffset;

	// set end offset
	if (docId == maxDocId && docId % 1000 == 0) { cerr << "\rProcessed " << beginOffset/(1024.0*1024.0) << "MB" << flush; }
      }
      cerr << "done." << endl;
      cerr << "Writing offsets to file..." << flush;
      if (fwrite((void*)offsets,(size_t)1,maxDocId*sizeof(off_t),offsets_file)!=maxDocId*sizeof(off_t)) 
      { cerr << "!Error: could not write offset for document " << docId << endl; std::exit(1); }
      else { cerr << setprecision(3) << "Written " << maxDocId << " offsets (" << maxDocId*sizeof(off_t)/(1024.0*1024.0) << "MB) in " << docOffsetsFileName << "..." << flush; }

      fclose(docs_file);
      fclose(offsets_file);
      free(offsets);
      free(line);
      cerr << "done." << endl;
    }
	

    //! Closes the database.
    void close() 
    { 
      if (_file != NULL) fclose(_file); 
      if (_offsetsFile != NULL) fclose(_offsetsFile);
    }
    

    // void get(const unsigned long& docid, ExcerptData& data) const { data = get(docid); }

   

    //! Returns the excerptData corresponding to a given DocId
    ExcerptData& get(const DocId& docId) const
    {
      cout << endl << "get(" << docId << ")" << endl;
      assert(_offsetsFile != NULL);
        //if (!isOffsetsOpen()) openOffsetsFile();
      if (_nofOffsets < docId) { cerr << "DocId (" << docId << ") out of range [0," << _nofOffsets << "]." << endl; std::exit(1); }
      off_t offset = _offsets[docId - 1];

      cout << "got offset." << endl;

      // Go to offset in file
      if (fseeko(_file,(off_t)offset,SEEK_SET) != 0) 
      { cerr << "Could not go to offset " << offset << " of file " << _filename << endl; std::exit(1); }

      cout << "At offset " << offset << endl;

      // read a line
      char* line = (char*)malloc(MAX_LINE_LENGTH_EXCERPT+1);
      if ( fgets(line, MAX_LINE_LENGTH_EXCERPT+1, _file) == NULL ) { cerr << "END OF FILE" << endl; std::exit(1); }

      cout << "Read line." << endl;

      // parse line (ideally done by ExcerptData.set(line))
      DocId id;

      // get docId
      if (sscanf(line, "%lu", &id)!=1) { cerr << "Wrong format: DocId expected" << endl; std::exit(1); }
      if (id != docId) { cerr << "Wrong line. Expected DocId is " << docId << " but found " << id << ". Is your offsets file corrupted? " << endl; exit(1); }

      cout << "Read docId and it matches input docid." << endl;

      // skip until next TAB
      unsigned int i = 0;
      bool endOfLine = false;
      char tag;
      while (line[i] != 0 && line[i] != '\t') i++;
      if (line[i] == 0) endOfLine = true;
      while (!endOfLine)
      {
        if (line[i] !='\t' || line[i+1] == 0 || line[i+2] != ':') { cerr << "Wrong format." << endl; exit(1); }
        tag = line[i+1]; 
        i+=3;

        // what follows next (until next tab / end of line) is the content
        char* content = (char*)malloc(MAX_LINE_LENGTH_EXCERPT+1);
        unsigned int j = 0;
        while (line[i] != 0 && line[i] != '\t') 
        {
          content[j] = line[i];
          i++; j++;
        }
        if (line[i] == 0) endOfLine = true;
        if (j > 0) content[j] = 0;

        switch (tag)
        {
          case 'u':
            // url = string(content);
            _returnData.setUrl(string(content));
            break;
          case 't':
            // title = string(content);
            _returnData.setTitle(string(content)+" [USING NEW EXCERPT]");;
            break;
          case 'H':
            // text = string(content);
            _returnData.setExcerpt("[NEW] " + string(content));
            break;
          default:
            cerr << "Wrong tag " << tag << "." << endl; exit(1); 
            break;
        }
        free(content);
        
      }
      free(line);
      _returnData.setScore(0);

      cout << endl << "finished get(" << docId << ")" << endl;
      return _returnData;
    }




    /*
    //! Returns the access mode.
    char mode() const { return _mode; }


    //! Tells whether the database is open.
    bool isOpen() const { return _isOpen; }


    //! Returns the filename of the database.
    const string& filename() const { return _filename; }
    */
}; // class ExcerptsDB


#endif
