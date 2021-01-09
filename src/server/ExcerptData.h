#ifndef __EXCERPT_DATA_H__
#define __EXCERPT_DATA_H__

#include "Globals.h"

//! OLD DATA STRUCTURE (still from Thomas, simply cut it from ExcerptsDB)

//! Datastructure for the data-field in an ExcerptsDB.
/*!
 *  In an ExcerptsDB the keys are document ids, represented by numbers of type \c unsigned \c long, and
 *  the data stored under a key is of type ExcerptData. This comprises a url, a title, and an excerpt.
 */
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


    //! Construct an object from a not null-terminated char*.
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
      std::string data(_data);
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
      std::string data(_data, n);
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
    void set(const std::string& data)
    {
      size_t tab1 = data.find('\t');
      if (tab1 == std::string::npos)
      {
	std::cerr << "ExcerptData::set(const char* data): Format of data incorrect (no \'\\t\' found)" << std::endl;
	std::cerr << "String was: " << data << std::endl;
	exit(1);
      }
      _url = data.substr(0, tab1);

      size_t tab2 = data.find('\t', tab1+1);
      if (tab2 == std::string::npos)
      {
	std::cerr << "ExcerptData::set(const char* data): Format of data incorrect (only one \'\\t\' found)" << std::endl;
	std::cerr << "String was: " << data << std::endl;
	exit(1);
      }
      _title = data.substr(tab1+1, tab2-tab1-1);

      size_t tab3 = data.find('\t', tab2+1);
      if (tab3 == std::string::npos)
      {
	_excerpt = data.substr(tab2+1);
	_score = 0;
      }
      else
      {
	_excerpt = data.substr(tab2+1, tab3-tab2-1);
	if (data.length()-tab3-1 != sizeof(int))
	{
	  std::cerr << "ExcerptData::set(const char* data): Document score of size " << sizeof(int) << " bytes expexted" << std::endl
		    << "Got " << data.length()-tab3-1 << " bytes." << std::endl;
	  exit(1);
	}
	_score = *((unsigned int*)(data.c_str() + tab3+1));
      }

//      if (data.find('\t', tab2+1) != std::string::npos)
//      {
//	std::cerr << "ExcerptData::set(const char* data): Format of data incorrect (more than two \'\\t\' found)" << std::endl;
//	std::cerr << "String was: " << data << std::endl;
//	exit(1);
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
    void set(const std::string& url, const std::string& title, const std::string& excerpt, unsigned int score = 0)
    {
      _url = url;
      _title = title;
      _excerpt = excerpt;
      _score = score;
    }


    //! Sets the url.
    void setUrl(const std::string& url) { _url = url; }


    //! Sets the title.
    void setTitle(const std::string& title) { _title = title; }


    //! Sets the excerpt.
    void setExcerpt(const std::string& excerpt) { _excerpt = excerpt; }


    //! Sets the score.
    void setScore(unsigned int score) { _score = score; }


    //! Returns the url as string.
    const std::string& url() const { return _url; }


    //! Returns the title as string.
    const std::string& title() const { return _title; }


    //! Returns the excerpt as string.
    const std::string& excerpt() const { return _excerpt; }


    //! Returns the score.
    const unsigned int& score() const { return _score; }


    //! Conversion to a string.
    /*!
     *  Returns a string containing the url, the title and the excerpt, in that order,
     *  separated by a '\\t'.
     */
    const std::string str() const
    {
      std::string s = _url + '\t' + _title + '\t' + _excerpt + '\t';
      for (size_t i=0; i<sizeof(int); i++) s += ((char*)(&_score))[i];
      return s;
    }
}; 

#endif
