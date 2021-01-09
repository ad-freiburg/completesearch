
/*
 * ----------
 * server.cpp
 * ----------
 */


#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sstream>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#include <signal.h>
#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>


#include "server.h"


std::ofstream *logfile = NULL;


#ifdef TRACE_LEVEL_2
#define TRACE_LEVEL_1
#endif


std::ostream& operator<<(std::ostream& o, const Null& n) { return o; }
std::istream& operator>>(std::istream& i, const Null& n) { return i; }

/*
   class Msg {
   public:
   static const char ServerShutdown[]      = "Server has been shut down.";
   static const char ServerRestarted[]     = "The Server has been restarted";
   static const char ReadingFromSocket[]   = "reading from socket";
   static const char WritingToSocket[]     = "writing to socket";
   static const char BufferOverflow[]      = "buffer overflow";
   static const char BindingSocket[]       = "binding socket"; 
   static const char GettingName[]         = "getting socket name";
   static const char ListeningSocket[]     = "listening to socket";
   static const char CreatingSocket[]      = "creating socket";
   static const char AcceptingConnection[] = "accepting connection";
   static const char CannotRead[]          = "cannot read from socket (client has gone)";
   static const char CannotWrite[]         = "cannot write to socket";
   static const char MalformedProtocol[]   = "malformed protocol";
   static const char ClosingFileHandle[]   = "closing file handle";
   static const char SettingSigHandler[]   = "setting signal handler";
   static const char Forking[]             = "forking";
   };
   */
class Msg {
  public:
    static const char ServerShutdown[];
    static const char ServerRestarted[];
    static const char ReadingFromSocket[];
    static const char WritingToSocket[];
    static const char BufferOverflow[];
    static const char BindingSocket[];
    static const char GettingName[];
    static const char ListeningSocket[];
    static const char CreatingSocket[];
    static const char AcceptingConnection[];
    static const char CannotRead[];
    static const char CannotWrite[];
    static const char MalformedProtocol[];
    static const char ClosingFileHandle[];
    static const char SettingSigHandler[];
    static const char Forking[];
};

const char Msg::ServerShutdown[]                  = "Server has been shut down.";
const char Msg::ServerRestarted[]                 = "The Server has been restarted";
const char Msg::ReadingFromSocket[]               = "reading from socket";
const char Msg::WritingToSocket[]                 = "writing to socket";
const char Msg::BufferOverflow[]                  = "buffer overflow";
const char Msg::BindingSocket[]                   = "binding socket"; 
const char Msg::GettingName[]                     = "getting socket name";
const char Msg::ListeningSocket[]                 = "listening to socket";
const char Msg::CreatingSocket[]                  = "creating socket";
const char Msg::AcceptingConnection[]             = "accepting connection";
const char Msg::CannotRead[]                      = "cannot read from socket (client has gone)";
const char Msg::CannotWrite[]                     = "cannot write to socket";
const char Msg::MalformedProtocol[]               = "malformed protocol";
const char Msg::ClosingFileHandle[]               = "closing file handle";
const char Msg::SettingSigHandler[]               = "setting signal handler";
const char Msg::Forking[]                         = "forking";

void fail_1(const char *msg)
{
  if (logfile) *logfile<<"Cannot continue normal operation:\n"<<msg;
}

  void fail_2(int resume) {
    if(errno!=0)
      if (logfile) *logfile<<": "<<strerror(errno);
    if (logfile) *logfile<<std::endl;
    if(!resume)
      exit(1);
  }

void Connector::fail(const char *msg, const char *info, int resume) {
  fail_1(msg);
  if(info!=NULL && info[0]!='\0' && logfile) 
    *logfile<<" ("<<info<<")";
  fail_2(resume);
}

void Connector::fail(const char *msg, const int info, int resume) {
  fail_1(msg);
  if (logfile) *logfile<<" ("<<info<<")";
  fail_2(resume);
}

void Connector::fail(const char *msg) { Connector::fail(msg, "", 0); }


  void shutdown() {
    if(logfile!=NULL)
      *logfile<<Msg::ServerShutdown<<std::endl;
    exit(0);
  }

static void sig_int(int signo) {
  // *logfile<<"SIG_INT caught.\n";
  shutdown(); 
}

static void sig_term(int signo) {
  // *logfile<<"SIG_TERM caught.\n";
  shutdown(); 
}

static void sig_quit(int signo) {
  // *logfile<<"SIG_QUIT caught.\n";
  shutdown(); 
}

void (*set_signal_handler (int signo, void (*disp)(int)))(int) {
  void (*volatile result) (int) = signal(signo,disp);
  if(result==SIG_ERR)
    Connector::fail(Msg::SettingSigHandler, signo);
  return result;
}

void Connector::init(const char *logfile_name) {

  // HOLGER 30Aug05: why fork?
  /*
  pid_t pid;
  if((pid=fork())<0)
    Connector::fail(Msg::Forking);
  else if(pid!=0)
    exit(0);
  setsid();
  */

  // HOLGER 30Aug05: commented out all three of these, but still no error messages :-(
  /*
  if(fclose(stdin))
    Connector::fail(Msg::ClosingFileHandle, "stdin");
  if(fclose(stdout))
    Connector::fail(Msg::ClosingFileHandle, "stdout");
  if(fclose(stderr))
    Connector::fail(Msg::ClosingFileHandle, "stderr");
  */

  logfile = new std::ofstream(logfile_name, std::ios::app);
  // HOLGER 30Aug05: no message here
  // if (logfile) *logfile<<std::endl<<Msg::ServerRestarted<<std::endl;

  set_signal_handler(SIGCHLD, SIG_IGN);
  set_signal_handler(SIGHUP,  SIG_IGN);
  set_signal_handler(SIGTERM, sig_term);
  set_signal_handler(SIGQUIT, sig_quit);
  set_signal_handler(SIGINT,  sig_int);
}


void dump(char *buf, int count, std::ostream& o) {
  // dumps a number of chars that buf points to onto o
  for(int i = 0; i<count; i++)
    if(isprint(buf[i]))
      o<<buf[i];
    else  
      o<<"#("<<((unsigned int)buf[i])<<')';
  o<<std::flush;
}


u_short Connector::port() {
  return ntohs(sock_struct.sin_port);
}


Connector::Connector(int newport, int buf_size) { 
  assert(buf_size>0);
  buf = new char[buffer_size = buf_size];
  accepted_fd = -1;

  // create a new socket of type SOCK_STREAM in the INET domain
  // and do automatic protocol selection
  if((fd = socket(AF_INET, SOCK_STREAM, 0))<0) {
    perror("Connector::Connector()");
    Connector::fail(Msg::CreatingSocket);
  }

  // bind socket
  sock_struct.sin_family = AF_INET;
  sock_struct.sin_addr.s_addr = INADDR_ANY;
  sock_struct.sin_port = htons(newport);
  int sizeof_sock_struct = sizeof(sock_struct);
  if(bind(fd, (sockaddr*)(&sock_struct), sizeof_sock_struct)) {
    perror("Connector::Connector()");
    Connector::fail(Msg::BindingSocket);
  }
  if(getsockname(fd, (sockaddr*)(&sock_struct), (socklen_t*)(&sizeof_sock_struct))) {
    perror("Connector::Connector()");
    Connector::fail(Msg::GettingName);
  }
  if(listen(fd, 5)) {
    perror("Connector::Connector()");
    Connector::fail(Msg::ListeningSocket);
  }

#ifdef TRACE_LEVEL_1
  if (logfile) *logfile<<"A welcoming connector is waiting at port "<<port()<<std::endl<<std::flush;
#endif

} 

  Connector::~Connector() {
    if(accepted_fd>=0)
      AcceptedGoodbye();

#ifdef TRACE_LEVEL_1
    if (logfile) *logfile<<"Destructed the welcoming connector at port "<<port()<<std::endl<<std::flush;
#endif

    delete buf;
    if(close(fd)) {
      Connector::fail(Msg::ClosingFileHandle, fd); 
    }
  }

void Connector::Accept() {
  assert(accepted_fd<0);

  int sizeof_accepted_sock_struct = sizeof(accepted_sock_struct);
  if((accepted_fd = accept(fd, (sockaddr*) &accepted_sock_struct, (socklen_t*)(&sizeof_accepted_sock_struct)))<0)
    Connector::fail(Msg::AcceptingConnection, "", 1);

#ifdef TRACE_LEVEL_1
  if (logfile) *logfile<<"Accepted a connection at port "
    <<ntohs(accepted_sock_struct.sin_port)<<std::endl<<std::flush;
#endif
}

void Connector::AcceptedGoodbye() {
  assert(accepted_fd>=0);

  if(close(accepted_fd)) {
    Connector::fail(Msg::ClosingFileHandle, accepted_fd, 1);
  }
  accepted_fd = -1;

#ifdef TRACE_LEVEL_1
  if (logfile) *logfile<<"Preparing for next accept at welcome port "<<port()<<std::endl<<std::flush;
#endif
}

void Connector::GetBytes(int count) {
  assert(count>=0 && accepted_fd>=0); 
  if(count>buffer_size)
    Connector::fail(Msg::BufferOverflow, "", 1);
  int result;
  for(int current_buf_count = 0; current_buf_count<count; current_buf_count += result) {
    result = read(accepted_fd, buf+current_buf_count, count-current_buf_count);
    if(result<=0)
#ifdef TRACE_LEVEL_1
      Connector::fail(Msg::CannotRead);
    // this is usually not an error - the client
    // simply has stopped operation, so we only dump
    // this message if some trace information is wanted
#else
    exit(0);
#endif
  }

#ifdef TRACE_LEVEL_2
  if (logfile) {
    *logfile<<"Received the following data into port "<<port()<<": ";
    dump(buf, count, logfile);
  }
#endif

}

void Connector::Write(const int i) {
  // has to be compatible with Java counterpart
  //    java.io.DataOutputStream.writeInt(int)
  //    java.io.DataInputStream.readInt()
  char b[4];
  unsigned int u = (unsigned int) i; // brute force since C++ has no >>> operator
  b[0] = (u>>24) & 0xFF;
  b[1] = (u>>16) & 0xFF;
  b[2] = (u>> 8) & 0xFF;
  b[3] = (u>> 0) & 0xFF;
  Write(b, 4);
}

void Connector::Read(int& i) {
  // has to be compatible with Java counterpart
  //    java.io.DataOutputStream.writeInt(int)
  //    java.io.DataInputStream.readInt()
  GetBytes(4);

  // careful! the following
  // casts seem to be superfluous, but AREN'T
  // especially the unsigned-char-to-int-assignment is needed
  int d0 = ((unsigned char) buf[0]), 
      d1 = ((unsigned char) buf[1]), 
      d2 = ((unsigned char) buf[2]), 
      d3 = ((unsigned char) buf[3]);
  i = ((d0 << 24) +
      (d1 << 16) +
      (d2 <<  8) +
      (d3 <<  0));
}


void float_4_to_8(unsigned char *sing, unsigned char *doub) {
  /* converts the 4 byte IEEE float starting at *sing
     into a 8 byte IEEE float starting at *doub
     by explicit bit manipulation 

     here is the bit representation I used

     12345678 12345678 12345678 12345678 12345678 12345678 12345678 12345678
     sing = seeeeeee emmmmmmm mmmmmmmm mmmmmmmm
     doub = seeeeeee eeeemmmm mmmmmmmm mmmmmmmm mmmmmmmm mmmmmmmm mmmmmmmm mmmmmmmm
     */

  int e; /* "exponent" */

  /* calculate new exponent from old one 
     - be careful of NaN and +inf and -inf
     */

  e = (( *(sing+3) & 0x7f) << 1) 
    | (( *(sing+2) & 0x80) >> 7);
  if(e!=0) {
    if(e==255) {
      e = 2047;
    } else {
      e += 1023-127;
    }
  }

  /* now do the ugly bit work ... */

  *(doub+7) =  (*(sing+3) & 0x80)
    | ( (e>>4)    & 0xff);
  *(doub+6) = ((e         & 0x0f) << 4)
    | ((*(sing+2) & 0x7f) >> 3);
  *(doub+5) = ((*(sing+2)         << 5) & 0xff)
    |  (*(sing+1)         >> 3);
  *(doub+4) = ((*(sing+1)         << 5) & 0xff) 
    |  ( *sing            >> 3);
  *(doub+3) =  ( *sing            << 5) & 0xff;
  *(doub+2) = 0;
  *(doub+1) = 0;
  *(doub+0) = 0;
}


void float_8_to_4(unsigned char *doub, unsigned char *sing) {
  /* converts the 8 byte IEEE float starting at *doub
     into a 4 byte IEEE float starting at *sing
     by explicit bit manipulation 

     here is the bit representation I used

     12345678 12345678 12345678 12345678 12345678 12345678 12345678 12345678
     sing = seeeeeee emmmmmmm mmmmmmmm mmmmmmmm
     doub = seeeeeee eeeemmmm mmmmmmmm mmmmmmmm mmmmmmmm mmmmmmmm mmmmmmmm mmmmmmmm
     */

  int e; /* "exponent" */

  /* calculate new exponent from old one 
     - be careful of NaN and +inf and -inf
     */

  e = ((*(doub+7) & 0x7f) << 4) 
    | (*(doub+6) >> 4);
  if(e!=0) {
    if(e==2947) {
      e = 255;
    } else {
      e -= (1023-127);
      if(e>255) { /* exponent overflow */
	e = 255;
      }
    }
  }

  /* now do the ugly bit work ... */

  *(sing+3) = *(doub+7) & 0x80;
  *(sing+2) = (((*(doub+6) & 0x0f) << 3) & 0xff) 
    |  ( *(doub+5)         >> 5);
  *(sing+1) = (( *(doub+5)         << 3) & 0xff) 
    |  ( *(doub+4)         >> 5);
  *(sing+0) = (( *(doub+4)         << 3) & 0xff) 
    |  ( *(doub+3)         >> 5);

  /* do rounding of mantissa! */

  if((*(doub+3) & 0x10)!=0) {
    if(*sing==0xff){
      *sing = 0;
      if(*(sing+1)==0xff) {
	*(sing+1) = 0;
	(*(sing+2))++;
	if((*(sing+2) & 0x80)!=0) {
	  *(sing+2) >>= 1;
	  if(e<255) {
	    e++;
	  }
	}
      } else {
	(*(sing+1))++;
      }
    } else {
      (*sing)++;
    }
  }

  /* merge exponent into bit representation */

  *(sing+3) |= (e >> 1);
  *(sing+2) |= (((e & 1) << 7) & 0xff);
}


void Connector::Write(const float f) {
  unsigned char float_of_4_bytes[4];
  if(sizeof(float)==4) {
    memcpy(&float_of_4_bytes,&f,4);
  } else {
    if(sizeof(float)==8) {
      float_8_to_4((unsigned char*) &f, (unsigned char*) &float_of_4_bytes);
      assert(*((float*) &float_of_4_bytes)==f);
    } else {
      // can't handle floats of sizes different than 8 or 4:
      assert(0);
    } 
  }
  Write(*((int*)&float_of_4_bytes));
}

void Connector::Read(float& f) {
  unsigned char float_of_4_bytes[4];
  Read(*((int*)&float_of_4_bytes));
  if(sizeof(float)==4) {
    f = *((float*)&float_of_4_bytes);       
  } else {
    if(sizeof(float)==8) {
      float_4_to_8((unsigned char*) &float_of_4_bytes,(unsigned char*) &f);
      assert(*((float*) &float_of_4_bytes) == f);
    } else {
      // can't handle floats of sizes different than 8 or 4:
      assert(0);
    }
  }
}


void Connector::Write(char *msg) {
  if (logfile && logfile->is_open()) *logfile << " [trying to send " << strlen(msg) << " characters ...";
  Write(msg, strlen(msg));
  if (logfile && logfile->is_open()) *logfile << " sent]";
}


void Connector::Write(char *msg, int count) {
  assert(accepted_fd>=0);
  int c;
  if((c=write(accepted_fd, msg, count))!=count)
  {
    if (logfile && logfile->is_open()) *logfile << "    Connector: sent " << c << " characters" << std::endl;
    Connector::fail(Msg::CannotWrite, "", 1);
  }

#ifdef TRACE_LEVEL_2
  if (logfile) {
    *logfile<<"Sent the following data to port "<<port()<<": ";
    dump(msg, count, logfile);
  }
#endif
}

void Connector::Read(char& c) {
  GetBytes(1);
  c = *buf;
}

int Connector::Read(char** c, int count) {
  assert(accepted_fd>=0);
  if (count>buffer_size) Connector::fail(Msg::BufferOverflow, "", 1);
  int result = read(accepted_fd, buf, count);
  *c = buf;
  return result;
}

void Connector::ReadLine(char** c) {
  assert(accepted_fd>=0);
  int current_buf_count = 0;
  do {       
    int result = read(accepted_fd, buf+current_buf_count, 1);
    if(result<=0)
      Connector::fail(Msg::CannotRead, "", 1);
    current_buf_count += result;  
    if((current_buf_count==buffer_size) && (buf[current_buf_count-1]!=0)) 
      Connector::fail(Msg::BufferOverflow, "", 1);
  } while((current_buf_count<=0) ? 1 : (buf[current_buf_count-1]!=0));

#ifdef TRACE_LEVEL_2
  if (logfile) {
    *logfile<<"Received the following data into port "<<port()<<": ";
    dump(buf, strlen(buf), logfile); 
  }
#endif

  *c = buf;
}

void Connector::Skip(const char c) {
  GetBytes(1);
  if(c!=*buf)
    Connector::fail(Msg::MalformedProtocol, "", 1);
}

void Connector::Skip(const char* s) {
  int l = strlen(s);
  GetBytes(l);
  if(strncmp(s,buf,l))
    Connector::fail(Msg::MalformedProtocol, "", 1);
}
