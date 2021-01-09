// Albert-Ludwigs-University Freiburg
// Chair of Algorithms and Data Structures
// Copyright CC 2010

 /*
  * --------
  * server.h
  * --------
  */


// #define TRACE_LEVEL_1 to have a generic trace dump in the logfile
// #define TRACE_LEVEL_2 to have a detailed trace dump in the logfile


#ifndef CODEBASE_SERVER_SERVER_H_
#define CODEBASE_SERVER_SERVER_H_


#include <assert.h>
#include <stdio.h>
#include <netinet/in.h>
#include <fstream>

// using namespace std;


 // extern ofstream* logfile;
 // uncomment the above line to be able to
 // dump log out to the logfile from this file, too


class Connector
{
 private:
  int                fd;                    // file descriptor
  int                buffer_size;           // size of the internal buffer
  int                accepted_fd;           // file descriptor of an accepte
                                            // connection
  struct sockaddr_in sock_struct;           // socket data for welcoming port
  struct sockaddr_in accepted_sock_struct;  // socket data for accepted port
  char               *buf;                  // pointer to internal buffer

  void GetBytes(int n);
  // reads in n chars from the connection into its buffer

 public:
  // class methods

  static void init(const char *logfile_name);
  static void fail(const char *msg, const char* info, int resume = 0);
  static void fail(const char *msg, const int info, int resume = 0);
  static void fail(const char *msg);

  // object methods

  // the default buffer size of a connection
  static const int DEFAULT_BUFFER_SIZE = 8*1024;

  Connector(int port, int buf_size = DEFAULT_BUFFER_SIZE);
  ~Connector();

  u_short port();
  // returns the port number that this connection uses

  void Accept();
  // accepts a new client on this port

  void AcceptedGoodbye();
  // closes the accepted socket (not the accepting one)

  void Write(char *msg, int byte_count);
  // void Write(char *msg, int byte_count,ofstream*);
  // writes a number of chars out to the port

  void Write(char *msg);
  // void Write(char *msg,ofstream*);
  // writes a null terminated string onto the port

  void Write(const int i);
  // writes the bytes that represent i onto the port
  // (does *not* write the ascii representation);

  void Write(const float f);

  void Read(int& i);
  // reads in an int from the port;
  // corresponds to Write(const int i);
  // the method to reconstruct the incoming byte stream back into an int

  void Read(float& f);

  void Read(char& c);
  // reads in a single char from the port

  int Read(char** c, int count);
  // reads up to count bytes from the port

  void ReadLine(char** c);
  // reads a null terminated string from the port
  // sets *c to point to the line; the line
  // *c is pointing to must not be modified from outside,
  // if you are in need of that, use strcpy first

  void Skip(const char *s);
  // reads in strlen(s) chars from the port and
  // checks if the read chars match the specified s

  void Skip(const char c);
  // same as Skip(const char *s) for a single char
};


class Null {};


std::ostream& operator<<(std::ostream& o, const Null& n);
std::istream& operator>>(std::istream& i, const Null& n);


class Algorithm
{
 protected:
   // This is an abstract class.
  Algorithm() {}
 public:
  virtual void Init(Connector& connector) {}
  virtual void Operate(Connector& connector) {}
};


#endif  // CODEBASE_SERVER_SERVER_H_

