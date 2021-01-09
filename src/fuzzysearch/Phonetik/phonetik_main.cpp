#include <fstream.h>
#include <iostream.h>
#include <time.h>

#include "phonetik.h"


main()  {

char buffer[128];
char buffer1[128];
char codestring[32];
char codestring1[32];
char answer[5];
int wordlength;
int P1_OK;



// ifstream in("eigs.txt");

cout << "Bitte Testwort eingeben: ";
cin >> buffer1;


wordlength = strlen(buffer1);
lowerstring(buffer1);
strcpy(codestring1, phonGerman_with((unsigned char*)buffer1));
// strcpy(codestring, phonGerman((unsigned char*)buffer1));

cout << "\nDEUTSCHE PHONETIK:" << codestring1 << "\n";


strcpy(codestring1, phonFrench_with((unsigned char*)buffer1));

cout << "\nFRANZ. PHONETIK:" << codestring1 << "\n";


cout << "\nENGLISCHE PHONETIK:" << codestring1 << "\n";

}

void lowerstring(char *str) 
{
  char flipcase[] = "ÄäÖöÜü";
  char *ptr;

  if (str == NULL)
    return;

  for (int i = 0; str[i]; i++) {
    if ((ptr = strchr(flipcase, (int) str[i])) != NULL ) {
      if (! ((ptr - flipcase) % 2)) 
        str[i] = *++ptr;
      else str[i] = *ptr;
    }
    else {
      str[i] = tolower(str[i]);
    }
  }
}





