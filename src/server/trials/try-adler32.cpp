#include "Globals.h"

using namespace std;

void printUsage()
{
	cout << "Usage: test-adler32 <number of bytes>\n\n"
		      "Generate a random sequence of the given number of bytes, \n"
			    "runs the adler32 checksum computation on that sequence \n"
			    "as well as a simple memcpy and a simple sum. Shows the \n"
					"running times for each.\n\n"
	        "adler32 code taken from http://www.gzip.org/zlib/rfc-zlib.html\n\n";
}

// adler32 stuff (defined at the end of this file)
#define BASE 65521 /* largest prime smaller than 65536 */
unsigned long update_adler32(unsigned long adler, unsigned char *buf, int len);
unsigned long adler32(unsigned char *buf, int len);

int main(int argc, char** argv)
{
  cout << endl << EMPH_ON << "ZLIB ADLER32 CHECKSUM EFFICIENCY TEST"
		   << " (" << VERSION << ")" << EMPH_OFF << endl << endl;
	Timer timer;
	if (argc <= 1) { printUsage(); exit(1); }
	unsigned int n = atoi(argv[1]);
	unsigned int R = argc > 2 ? atoi(argv[2]) : 3;

	// GENERATE RANDOM SEQUENCE
	cout << "generating random sequence of " << commaStr(n) << " bytes ... " << flush;
	timer.start();
	char* buf = (char*)(malloc(n));
	for (unsigned int i = 0; i < n; ++i)
		buf[i] = i % 100 == 0 ? (char)(256*drand48()) : (char)(i * 251);
	timer.stop();
	cout << "done in " << timer << "\n\n";

	// SIMPLE MEMCPY
	for (unsigned int r = 1; r <= R; ++r)
	{
		cout << "simple memcpy ... " << flush;
		char* buf2 = (char*)(calloc(n,1));
		timer.start();
		memcpy(buf2, buf, n);
		timer.stop();
		cout << "done in " << timer << " (not measuring allocation time)\n";
	}
  cout << endl;

	// SIMPLE SUM
	for (unsigned int r = 1; r <= R; ++r)
	{
		cout << "simple sum of bytes ... " << flush;
		timer.start();
		char sum = 0;
		for (unsigned int i = 0; i < n; ++i)
			sum += buf[i];
		timer.stop();
		if (sum > sum) cout << "!!! ";
		cout << "done in " << timer << endl;
	}
  cout << endl;

	// COMPUTE ADLER CHECKSUM
	for (unsigned int r = 1; r <= R; ++r)
	{
		cout << "adler32 checksum ... " << flush;
		timer.start();
    unsigned long adler = 1L;
		adler = update_adler32(adler, (unsigned char*)(buf), n);
		timer.stop();
		if (adler > adler) cout << "!!! ";
		cout << "done in " << timer << endl;
	}
  cout << endl;
}



/*
   Update a running Adler-32 checksum with the bytes buf[0..len-1]
 and return the updated checksum. The Adler-32 checksum should be
 initialized to 1.

 Usage example:

   unsigned long adler = 1L;

   while (read_buffer(buffer, length) != EOF) {
     adler = update_adler32(adler, buffer, length);
   }
   if (adler != original_adler) error();
*/
unsigned long update_adler32(unsigned long adler,
   unsigned char *buf, int len)
{
  unsigned long s1 = adler & 0xffff;
  unsigned long s2 = (adler >> 16) & 0xffff;
  int n;

  for (n = 0; n < len; n++) {
    s1 = (s1 + buf[n]) % BASE;
    s2 = (s2 + s1)     % BASE;
  }
  return (s2 << 16) + s1;
}

/* Return the adler32 of the bytes buf[0..len-1] */

unsigned long adler32(unsigned char *buf, int len)
{
  return update_adler32(1L, buf, len);
}
