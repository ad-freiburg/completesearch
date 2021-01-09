#include "./Vector.h"

// _____________________________________________________________________________
template <class T>
Vector<T>::Vector()
{
  _isFullList = false;
#ifndef STL_VECTOR
  _size = 0;
  _capacity = 0;
  _data = NULL;
#endif
}

// _____________________________________________________________________________
template <class T>
Vector<T>::~Vector()
{
#ifndef STL_VECTOR
  if (_data)
  {
    free(_data);
    _data = NULL;
  }
#endif
}

// _____________________________________________________________________________
template<class T>
string Vector<T>::asString() const
{
  ostringstream os;
  os << "[";
  for (size_t i = 0; i < size(); i++)
  {
    if (i > 0) os << " ";
    os << operator[](i);
  }
  os << "]";
  return os.str();
}

// _____________________________________________________________________________
template<class T>
void Vector<T>::parseFromString(const string& vectorAsString)
{
  clear();
  size_t pos = 0;
  while (true)
  {
    pos = vectorAsString.find_first_not_of(", ", pos);
    if (pos == string::npos) break;
    Vector<T>::push_back(atoi(vectorAsString.c_str() + pos));
    pos = vectorAsString.find_first_of(", ", pos);
    if (pos == string::npos) break;
  }
}

// _____________________________________________________________________________
template <class T>
Vector<T>::Vector(const T& element, unsigned long nofRepetitions)
#ifdef STL_VECTOR
  : vector<T>(nofRepetitions, element)
#endif
{
  _isFullList = false;
#ifndef STL_VECTOR
  _size = 0;
  _capacity = 0;
  _data = NULL;
  reserve(nofRepetitions);
  //        if (vectorError) {return;}
  _size = nofRepetitions;
  for (register size_type i= 0; i < _size; ++i)
  {
    _data[i] = element;
  }
#endif
}

#ifndef STL_VECTOR

// _____________________________________________________________________________
//! Reserve space for the given number of elements (does nothing if enough
// space is already reserved)
/*  
 *  Note: The reserve does currently NOT do the initialization by memset
 */
template <class T> 
void Vector<T>::reserve(size_type n)
{
  //        assert(n>0);
  assert(n >= _size);
  assert(sizeof(T) > 0);
  // INCREASE CAPACITY
  //        if(n > _capacity)
  //          {
  /*
     T* newData;
     newData = (T*) malloc(sizeof(T) * n);
     assert(newData);
     if(_data)
     {
     assert(_size>0);
     memcpy(newData,_data, sizeof(T) * _size);
     assert(_data);
     free(_data);
     }
     _data = newData;          
     */
  if(n>0)
  {
    _data = (T*) realloc(_data, sizeof(T) * n);
    if (!_data)
    {
      ostringstream os;
      os << n << " items of size " << sizeof(T);
      THROW(Exception::REALLOC_FAILED, os.str());
    }
    assert(_data);
  }
  else
  {
    if(_data) {free(_data); _data = NULL;}
    assert(_data == NULL);
  }

  //          }
  // DECREASE CAPACITY
  //        if(n < _capacity)
  //          {
  //            T* newData = (T*) realloc(_data, sizeof(T) * n);
  //            assert(newData == _data);
  //            _data = newData; // just for the case that they are not equal
  //          }
  _capacity = n;
}

// _____________________________________________________________________________
//! Resize vector and set new elements to specified value (default = 0)
template <class T> 
void Vector<T>::resize(size_type n, T inEl)
{
  if(n > _capacity)
  {
    reserve(n+1);//one additional element (for artificial element at end)
    //            if (vectorError) {return;}
    assert(n<_capacity);
  }

  assert(n<=_capacity);

  if(n > _size)
  {
    assert(_data);
    if(inEl == 0)
    {
      assert(&(_data[_size]));
      assert(&(_data[n-_size]));
      memset((void*) &(_data[_size]), (int) 0, sizeof(T) * (n - _size) );
    }
    else
    {
      for(register size_type i=_size; i<n; ++i)
      {
        _data[i] = inEl;
      }
    }
  }
  _size = n;
}

// _____________________________________________________________________________
//! Copy Constructor
template <class T> 
Vector<T>::Vector(const Vector& orig)
{
  _isFullList = orig._isFullList;
  _size = orig._size;
  _capacity = orig._size; //yes! size! not capacity.
  assert((orig._data == NULL) || (_size > 0 ));
  if(_size > 0)
  {
    _data = (T*) realloc(NULL,sizeof(T) * _size);
    if (!_data) 
    {
      ostringstream os;
      os << _size << " items of size " << sizeof(T);
      THROW(Exception::REALLOC_FAILED, os.str());
    }
    assert(_data);
    memcpy(_data, orig._data, _size*sizeof(T));
  }
  else
  {
    _data = NULL;
  }
};

// _____________________________________________________________________________
//! Assignment operator
template <class T>
Vector<T>& Vector<T>::operator=(const Vector& orig)
{
  _isFullList = orig._isFullList;
  _size = orig._size;
  _capacity = orig._size; //yes! size! not capacity.
  assert((orig._data == NULL) || (_size > 0 ));
  if(_size > 0)
  {
    _data = (T*) realloc(_data,sizeof(T) * _size);
    if (!_data) 
    {
      ostringstream os;
      os << _size << " items of size " << sizeof(T);
      THROW(Exception::REALLOC_FAILED, os.str());
    }
    assert(_data);
    memcpy(_data, orig._data, _size*sizeof(T));
  }
  else
  {
    _data = NULL;
  }

  return *this;
}

#endif

// ____________________________________________________________________________
template <class T>
string Vector<T>::debugString()
{
  ostringstream os;
  os << "{";
  for (size_t i = 0; i < size(); ++i)
  {
    if (i > 0) os << ",";
    os << operator[](i);
  }
  os << "}";
  return os.str();
}

//! EXPLICIT INSTANTIATION (so that actual code gets generated)
template class Vector<int>;
template class Vector<unsigned char>;
template class Vector<unsigned int>;
template class Vector<unsigned long>;
template class Vector<long int>;
