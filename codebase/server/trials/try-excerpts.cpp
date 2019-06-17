#include "Globals.h"
#include "ExcerptsDB_NEW.h"

int main(int argc, char** argv)
{
  ExcerptsDB_NEW excerptsDB;
  
  if (argc < 2) { cerr << "Usage: test-excerpts [<docs file>] [<doc offsets file>]" << endl; exit(0); }
  string docsFileName = argv[1];
  string docOffsetsFileName = argc > 2 ? argv[2] : docsFileName + ".offsets";

  excerptsDB.open(docsFileName, docOffsetsFileName);

  unsigned int docId = 0;

  char* input = (char*)malloc(100);
  while (true)
  {
    cout << "DocId: " << flush;
    cin >> input;
    docId = atoi(input);
    if (docId == 0) break;
    ExcerptData excerptData = excerptsDB.get(docId);

    cout << "DocId: " << docId << endl
         << "Title: \"" << excerptData.title() << "\"" << endl
	 << "URL: \"" << excerptData.url() << "\"" << endl
	 << "Text: \"" << excerptData.excerpt() << "\"" << endl;
}

  // excerptsDB.buildOffsets(1000000);
  // cout << "Done Building offsets." << endl;

  free(input);
  excerptsDB.close();
}
