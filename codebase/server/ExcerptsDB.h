#ifndef __EXCERPTSDB_H__
#define __EXCERPTSDB_H__

#include <iostream>
#include <fstream>
#include <string>
#include <db_cxx.h>
// NEW 07Aug07 (Holger): moved to separate header file
#include "ExcerptData.h"


//! A Dbt containing a key of an ExcerptsDB.
/*!
 *  A key in an ExcerptsDB is an \c unsigned \c long.
 *
 *  For more information about Dbt's see the
 *  <A HREF="http://www.sleepycat.com/docs/api_cxx/frame.html"> BerkeleyDB documentation </A>.
 */
class DbtKey : public Dbt
{
  private:

    //! The key-value.
    unsigned long _key;


  public:

    //! The default constructor.
    /*!
     *  Simply calls the Dbt-constructor.
     */
    DbtKey() : Dbt() {}


    //! Constructs a DbtKey from an \c unsigned \c long.
    DbtKey(const unsigned long& k)
    {
      _key = k;
      set_size(sizeof(unsigned long));
      set_data((void*)(&_key));
    }


    //! Initialize \c *this with an \c unsigned \c long.
    void set(const unsigned long& k)
    {
      _key = k;
      set_size(sizeof(unsigned long));
      set_data((void*)(&_key));
    }


    //! Get the key from \c *this.
    /*!
     *  \param 		k 			Is set to the key-value.
     */
    void get(unsigned long& k) const
    {
      if (get_size() != sizeof(unsigned long))
      {
	std::cerr << "DbtKey::get(unsigned long): Wrong size" << std::endl;
	exit(1);
      }
      k = *((unsigned long*)get_data());
    }


    //! Return the key-value.
    unsigned long value() const
    {
      if (get_size() != sizeof(unsigned long))
      {
	std::cerr << "DbtKey::get(unsigned long): Wrong size" << std::endl;
	exit(1);
      }
      return *((unsigned long*)get_data());
    }
}; // class DbtKey



//! A Dbt containing a data-field of an ExcerptsDB.
/*!
 *  A data-field in an ExcerptsDB is an object of type ExcerptData.
 *
 *  For more information about Dbt's see the
 *  <A HREF="http://www.sleepycat.com/docs/api_cxx/frame.html"> BerkeleyDB documentation </A>.
 */
class DbtData : public Dbt
{
  private:

    //! Contains the data as a string.
    std::string _string;


  public:

    //! The default-constructor.
    /*!
     *  Simply calls the Dbt-constructor.
     */
    DbtData() : Dbt() {}


    //! Constructs an object from an object of type ExcerptData.
    DbtData(const ExcerptData& _data)
    {
      _string = _data.str();
      set_size(_string.length());
      set_data((void*)(_string.c_str()));
    }


    //! Initializes \c *this from an object of type ExcerptData.
    void set(const ExcerptData& _data)
    {
      _string = _data.str();
      set_size(_string.length());
      set_data((void*)(_string.c_str()));
    }


    //! Get the data stored in \c *this.
    /*!
     *  \param 		_data 			Is set to the data.
     */
    void get(ExcerptData& _data) const { _data.set((char*)get_data(), get_size()); }


    //! Returns the data stored in \c *this as string.
    std::string contents() const { return std::string((char*)get_data(), get_size()); }
}; // class DbtData



//! A database-handle for an excerpts-database.
/*!
 *  This class uses the BerkeleyDB C++-API to handle a database containing excerpts of documents.
 *  For more information about BerkeleyDB see
 *  <A HREF="http://www.sleepycat.com/docs/api_cxx/frame.html"> this documentation </A>.
 *  - The keys in an ExcerptsDB are document-ids represented by numbers of type \c unsigned \c long.
 *  - The data-fields stored under the keys are objects of type ExcerptData.
 */
class ExcerptsDB
{
  private:

    //! The BerkeleyDB database handle.
    Db* _database;


    //! The name of the database file.
    std::string _filename;


    //! The access mode.
    /*!
     *  Can take one of the values READ, WRITE and RANDOM.
     */
    char _mode;


    //! The cache size in bytes.
    unsigned int _cacheSize;


    //! Variable for returning data as reference.
    mutable ExcerptData _returnData;


    //! Is true iff a database is open.
    bool _isOpen;


  public:

    //! The access mode READ (read-only).
    static const char READ;

    //! The access mode WRITE (write-only).
    static const char WRITE;

    //! The access mode RANDOM (read-write).
    static const char RANDOM;

   
    //! The default constructor.
    ExcerptsDB() { _database = NULL; _mode = READ; _isOpen = false; _cacheSize = 0; }
    

    //! Constructor taking a filename, a cache size and an access mode.
    /*!
     *  Calls the function open().
     */
    ExcerptsDB(const std::string& filename, const unsigned int& cache_size, const char& mode = READ)
    {
      _isOpen = false;
      open(filename, cache_size, mode);
    }


    //! Creates and opens a database.
    /*!
     *  \param 		filename 		The name of the database-file to be created.
     *  \param 		cache_size 		The cache-size in bytes.
     *  \param 		mode 			The access-mode.
     *  \return 	\c true, if the database could be successfully created, and \c false otherwise.
     *
     *  If a database with the given name already exists, an error is reported and the function returns \c false.
     *  This function refuses the mode READ since read-only access to a newly created database
     *  does not make much sense.
     */
    bool create(const std::string& filename, const unsigned int& cache_size, const char& mode = WRITE)
    {
      if (mode == READ)
      {
	std::cerr << "Cannot create a database with read-only access" << std::endl;
	exit(1);
      }

      if (_isOpen) close();

      _database = new Db(NULL, 0);
      _database->set_cachesize(0, cache_size, 1);

      int status;
      if ((status = _database->open(NULL, filename.c_str(), NULL, DB_BTREE, DB_CREATE | DB_EXCL, 0)))
      {
	std::cerr << "Error while creating database " << filename << ": " << DbEnv::strerror(status) << std::endl;
	return false;
      }

      _filename = filename;
      _mode = mode;
      _cacheSize = cache_size;
      _isOpen = true;
      return true;
    }


    //! Open an existing database.
    /*!
     *  \param 		filename 		The name of the database-file to be opened.
     *  \param 		cache_size 		The cache size in bytes.
     *  \param 		mode 			The access-mode.
     *  \return 	\c true, if the database could be successfully opened, and \c false otherwise.
     */
    bool open(const std::string& filename, const unsigned int& cache_size, const char& mode = READ)
    {
      if (_isOpen) close();

/*
      HOLGER 27Jul05: The following gave a fatal error on GEEK but not on DUDE ???
                      I just commented it out, since I have no idea, why Thomas opens the file here

      std::fstream file;
      std::_Ios_Openmode om = (mode & WRITE ? std::fstream::out : std::fstream::in);
      file.open(filename.c_str(), om);
      if (!file.is_open()) 
      {
        // HOLGER 27Jul05: made this a fatal error    Q: WHY is this file opened???
        cerr << "ERROR in ExcerptsDB::open -> could not open file \"" << filename << "\"" << endl;
        exit(1);
        // return false;
      }
*/
     
      unsigned int OpenFlags = 0;
      int status;

      if (mode == READ) OpenFlags = DB_RDONLY;

      _database = new Db(NULL, 0);
      _database->set_cachesize(0, cache_size, 1);

      if ((status = _database->open(NULL, filename.c_str(), NULL, DB_BTREE, OpenFlags, 0)))
      {
	std::cerr << "ERROR in ExcerptsDB::open -> Db::open on file \"" << filename << "\""
                  << " -> " << DbEnv::strerror(status) << std::endl;
	exit(1);
      }

      _filename = filename;
      _mode = mode;
      _cacheSize = cache_size;
      _isOpen = true;
      return true;
    }


    //! Closes the database.
    void close() { if (_isOpen) _database->close(0); }
    

    //! Write to the database.
    /*!
     *  \param 		docid 			The key under which the data is to be stored.
     *  \param 		data 			The data that is to be stored under the key.
     *
     *  Writes the given key/data pair to the database provided that the mode is not READ.
     *  If the key alerady exists this function does nothing.
     */
    void put(const unsigned long& docid, const ExcerptData& data)
    {
      if (!_isOpen)
      {
	std::cerr << "Error writing to database: No database opened" << std::endl;
	exit(1);
      }
      if (!(_mode & WRITE))
      {
	std::cerr << "Error writing to database " << _filename << ": " << "File has been opened read-only" << std::endl;
	exit(1);
      }

      DbtKey Key(docid);
      DbtData Data(data);

      int status = _database->put(NULL, &Key, &Data, DB_NOOVERWRITE);

      if (status && status != DB_KEYEXIST)
      {
	std::cerr << "Error writing to database " << _filename << ": " << DbEnv::strerror(status) << std::endl;
	exit(1);
      }
    }


    //! Read from the database.
    /*!
     *  \param 		docid 			The key of the data to be read.
     *  \param 		data 			Will be set to the data stored under key.
     *
     *  Reads the data corresponding to docid from the database provided that the mode is not WRITE.
     */
    void get(const unsigned long& docid, ExcerptData& data) const { data = get(docid); }

   
	//! Read from the database.
	/*!
	 *  \param 		docid 			The key of the data to be read.
	 *  \return 	The data corresponding to \a docid.
	 *
	 *  Reads the data corresponding to \a docid from the database provided that the mode is not WRITE.
	 */
	ExcerptData& get(const unsigned long& docid) const
	{
	  if (!_isOpen)
	  {
		std::cerr << "Error reading from database: No database opened" << std::endl;
		exit(1);
	  }
	  if (!(_mode & READ))
	  {
		std::cerr << "Error reading from database " << _filename << ": " << "File has been opened writeonly" << std::endl;
		exit(1);
	  }

	  DbtKey Key(docid);
	  DbtData Data;

	  int status = _database->get(NULL, &Key, &Data, 0);

	  if (status)
	  {
		std::cerr << "Error reading from database " << _filename << ": " << DbEnv::strerror(status) << std::endl;
		_returnData.setUrl("");
		_returnData.setTitle("DOCUMENT NOT FOUND");
		_returnData.setExcerpt("");
	  } else Data.get(_returnData);

	  return _returnData;
	}

   
    //! Returns the access mode.
    char mode() const { return _mode; }


    //! Tells whether the database is open.
    bool isOpen() const { return _isOpen; }


    //! Returns the filename of the database.
    const std::string& filename() const { return _filename; }


    //! Returns the pointer to the database-handle
    Db* database() { return _database; }


    //! Returns the cache-size in bytes.
    const unsigned int& cacheSize() const { return _cacheSize; }
}; // class ExcerptsDB

////} // namespace aucmpl

#endif
