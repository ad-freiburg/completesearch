#include <map>
#include "CompletionServer.h"


static const string outputFilename = "serverTestCurlOutput";
static const string port = "8188";

class CompletionServerTest : public ::testing::TestWithParam<string>
{
  virtual void SetUp()
  {
    string additionalOptions = GetParam();
    isCorsEnabled = additionalOptions.find("--enable-cors") != string::npos;
    string startCommand = string("./startCompletionServer ")
       			+ "--port=" + port + " "
                        + "--maps-directory=TestFiles/ "
			+ additionalOptions
			+ " TestFiles/example-input.hybrid";
    execute(startCommand.c_str());
  }

  virtual void TearDown()
  {
    string killCommand  = string("./startCompletionServer --kill ") + port;
    execute(killCommand.c_str());
    removeFile(outputFilename.c_str());
  }

 public:
  bool isCorsEnabled;

  // Execute a command.
  void execute(const char* cmd)
  {
    FILE* fp = popen(cmd,  "w");
    if (fp == NULL)
    {
      cout << "Execution of \"" << cmd << "\" failed." << endl;
      FAIL();
    }
    char buff[512];
    while (fgets(buff, sizeof(buff), fp) != NULL)
    {
      printf("%s", buff);
    }
   
    int ret = pclose(fp);
    if (ret == -1)
    {
      cout << "Closing file stream of command \"" << cmd << "\" failed."
           << endl;
      FAIL();
    }
    else if (ret / 256 > 0)
    {
      cout << "ERROR on executing command \"" << cmd << "\"." << endl;
      cout << "If the command fails on killing the server or while curling, "
              "check if there is already a server running at this port."
           << endl;
      FAIL();
    }
  }

  void removeFile(const char* filename)
  {
    string command = string("rm -rf ") + filename;
    execute(command.c_str());
  }

  void touchFile(string filename)
  {
    string command = string("touch ") + filename;
    execute(command.c_str());
  }

  map<string, string> parseHeader(string header)
  {
    map<string, string> parsedHeader;
    size_t start = 0;
    size_t mid   = 0;
    for (size_t i = 0; i < header.size(); i++)
    {
      if (header[i] == ':') mid = i;
      if (header[i] == '\n' || header[i] == '\r')
      {
	if (header.substr(start, 4) == "HTTP")
	{
	  parsedHeader["Status"]
	    = header.substr(start, i - start);
	}
	else if (start < mid)
	{
	  string attribute
	    = header.substr(start, mid - start);
	  string value
	    = header.substr(mid + 2, i - mid - 2);
	  parsedHeader[attribute] = value;
	}
	start = i+1;
      }
    }
    //for (auto& t : parsedHeader) cout << t.first << " " << t.second << "\n";
    //cout << endl;
    return parsedHeader;
  }

  // Return a files content.
  std::string fileToString(const char* filename)
  {
    std::string retval;
    int fileLength;
    ifstream is;
    char* buffer;

    is.open(filename);
    // Determine file size.
    is.seekg(0, std::ios::end);
    fileLength = is.tellg();
    is.seekg(0, std::ios::beg);
    // Read the files content.
    buffer = new char[fileLength + 1];
    is.read(buffer, fileLength);
    is.close();
    buffer[fileLength] = '\0';
    retval = std::string(buffer);
    delete[] buffer;
    return retval;
  }

  string generateHTTPRequest(string type, string data, string output, string additionalOptions = "")
  {
    string prefix = string("curl -s -o /dev/null -D ") + output + " " + additionalOptions + " 127.0.0.1:" + port;
    
    if (type == "POST")
      return prefix + " --data \"" + data + "\" ";
    if (type == "GET")
      return prefix + "/?" + data;
    return "";
  }
};

string testOptions[] = { "--normalize", "--enable-cors" };
INSTANTIATE_TEST_CASE_P(OPTIONS,
                        CompletionServerTest,
                        ::testing::ValuesIn(testOptions));;


TEST_P(CompletionServerTest, processRequest_GET)
{
  // run requests
  for (unsigned int i = 0; i < 10; i++)
  {
    // Generate a word with different lengths. Content is not relevant.
    unsigned int requestSize = rand() % 30;
    string word (requestSize, 'a');
    // GET request with random length words.
    string request = generateHTTPRequest("GET", string("q=") + word, outputFilename);
    execute(request.c_str());
    map<string, string> header = parseHeader(fileToString(outputFilename.c_str()));
    ASSERT_NO_THROW(header.at("Status"));
    ASSERT_NO_THROW(header.at("Content-Length"));
    ASSERT_NO_THROW(header.at("Connection"));
    ASSERT_NO_THROW(header.at("Content-Type"));
    EXPECT_EQ("HTTP/1.1 200 OK", header["Status"]);
    EXPECT_EQ("close", header["Connection"]);
    EXPECT_EQ("text/xml; charset=UTF-8", header["Content-Type"]);
    EXPECT_LE(0, atoi(header["Content-Length"].c_str()));
    if (isCorsEnabled) {
      ASSERT_NO_THROW(header.at("Access-Control-Allow-Origin"));
      EXPECT_EQ("*", header["Access-Control-Allow-Origin"]);
    } else {
      EXPECT_EQ(header.find("Access-Control-Allow-Origin"), header.end());
    }
    removeFile(outputFilename.c_str());
  }
  // Try a very long request > 2083 byte. Take care that the requestSize does
  // not include the header. Therefore the content is actually larger and it's
  // hard to test the critical values.
  unsigned int requestSize = 2084;
  string word (requestSize, 'b');
  string request = generateHTTPRequest("GET", string("q=") + word, outputFilename);
  execute(request.c_str());
  map<string, string> header = parseHeader(fileToString(outputFilename.c_str()));
  ASSERT_NO_THROW(header.at("Status"));
  ASSERT_NO_THROW(header.at("Connection"));
  ASSERT_NO_THROW(header.at("Content-Type"));
  EXPECT_EQ("HTTP/1.1 414 Request-URI Too Long", header["Status"]);
  EXPECT_EQ("close", header["Connection"]);
  EXPECT_EQ("text/xml; charset=UTF-8", header["Content-Type"]);
  if (isCorsEnabled) {
    ASSERT_NO_THROW(header.at("Access-Control-Allow-Origin"));
    EXPECT_EQ("*", header["Access-Control-Allow-Origin"]);
  } else {
    EXPECT_EQ(header.find("Access-Control-Allow-Origin"), header.end());
  }
  removeFile(outputFilename.c_str());
}

TEST_P(CompletionServerTest, processRequest_POST)
{
  // run requests
  for (unsigned int i = 0; i < 10; i++)
  {
    // Generate a word with different lengths. Content is not relevant.
    unsigned int requestSize = rand() % 30;
    string word (requestSize, 'a');
    // POST request with random length words.
    string request = generateHTTPRequest("POST", string("q=") + word, outputFilename);
    execute(request.c_str());
    map<string, string> header = parseHeader(fileToString(outputFilename.c_str()));
    ASSERT_NO_THROW(header.at("Status"));
    ASSERT_NO_THROW(header.at("Content-Length"));
    ASSERT_NO_THROW(header.at("Connection"));
    ASSERT_NO_THROW(header.at("Content-Type"));
    EXPECT_EQ("HTTP/1.1 200 OK", header["Status"]);
    EXPECT_EQ("close", header["Connection"]);
    EXPECT_EQ("text/xml; charset=UTF-8", header["Content-Type"]);
    EXPECT_LE(0, atoi(header["Content-Length"].c_str()));
    if (isCorsEnabled) {
      ASSERT_NO_THROW(header.at("Access-Control-Allow-Origin"));
      EXPECT_EQ("*", header["Access-Control-Allow-Origin"]);
    } else {
      EXPECT_EQ(header.find("Access-Control-Allow-Origin"), header.end());
    }
    removeFile(outputFilename.c_str());
  }
  // Try a very long request > 2083 byte. Take care that the requestSize does
  // not include the header. Therefore the content is actually larger and it's
  // hard to test the critical values.
  unsigned int requestSize = 2084;
  string word (requestSize, 'b');
  // TODO (baumgari):
  // The server cannot handle "Expect: 100-continue" right now. Therefore we
  // just erase ist. Nevertheless the server should return the proper error code
  // "417 Expectation Failed" in this case. This is not working right now and
  // needs to be implemented.
  // NEW (baumgari): Done. Test both cases.
  string request = generateHTTPRequest("POST", string("q=") + word, outputFilename);
  execute(request.c_str());
  map<string, string> header = parseHeader(fileToString(outputFilename.c_str()));
  ASSERT_NO_THROW(header.at("Status"));
  ASSERT_NO_THROW(header.at("Connection"));
  ASSERT_NO_THROW(header.at("Content-Type"));
  EXPECT_EQ("HTTP/1.1 417 Expectation Failed", header["Status"]);
  EXPECT_EQ("close", header["Connection"]);
  EXPECT_EQ("text/xml; charset=UTF-8", header["Content-Type"]);
  if (isCorsEnabled) {
    ASSERT_NO_THROW(header.at("Access-Control-Allow-Origin"));
    EXPECT_EQ("*", header["Access-Control-Allow-Origin"]);
  } else {
    EXPECT_EQ(header.find("Access-Control-Allow-Origin"), header.end());
  }

  request = generateHTTPRequest("POST", string("q=") + word, outputFilename, "--header \"Expect:\"");
  execute(request.c_str());
  header = parseHeader(fileToString(outputFilename.c_str()));
  ASSERT_NO_THROW(header.at("Status"));
  ASSERT_NO_THROW(header.at("Content-Length"));
  ASSERT_NO_THROW(header.at("Connection"));
  ASSERT_NO_THROW(header.at("Content-Type"));
  EXPECT_EQ("HTTP/1.1 200 OK", header["Status"]);
  EXPECT_EQ("close", header["Connection"]);
  EXPECT_EQ("text/xml; charset=UTF-8", header["Content-Type"]);
  EXPECT_LE(0, atoi(header["Content-Length"].c_str()));
  if (isCorsEnabled) {
    ASSERT_NO_THROW(header.at("Access-Control-Allow-Origin"));
    EXPECT_EQ("*", header["Access-Control-Allow-Origin"]);
  } else {
    EXPECT_EQ(header.find("Access-Control-Allow-Origin"), header.end());
  }

  removeFile(outputFilename.c_str());
}


TEST_P(CompletionServerTest, processRequest_FORMAT)
{
  string word = "test";
  // Xml
  string request = generateHTTPRequest("POST", string("q=") + word + string("&format=xml"), outputFilename);
  execute(request.c_str());
  map<string, string> header = parseHeader(fileToString(outputFilename.c_str()));
  ASSERT_NO_THROW(header.at("Status"));
  ASSERT_NO_THROW(header.at("Content-Length"));
  ASSERT_NO_THROW(header.at("Connection"));
  ASSERT_NO_THROW(header.at("Content-Type"));
  EXPECT_EQ("HTTP/1.1 200 OK", header["Status"]);
  EXPECT_EQ("close", header["Connection"]);
  EXPECT_EQ("text/xml; charset=UTF-8", header["Content-Type"]);
  EXPECT_LE(0, atoi(header["Content-Length"].c_str()));
  if (isCorsEnabled) {
    ASSERT_NO_THROW(header.at("Access-Control-Allow-Origin"));
    EXPECT_EQ("*", header["Access-Control-Allow-Origin"]);
  } else {
    EXPECT_EQ(header.find("Access-Control-Allow-Origin"), header.end());
  }

  // Json
  request = generateHTTPRequest("POST", string("q=") + word + string("&format=json"), outputFilename);
  execute(request.c_str());
  header = parseHeader(fileToString(outputFilename.c_str()));
  ASSERT_NO_THROW(header.at("Status"));
  ASSERT_NO_THROW(header.at("Content-Length"));
  ASSERT_NO_THROW(header.at("Connection"));
  ASSERT_NO_THROW(header.at("Content-Type"));
  EXPECT_EQ("HTTP/1.1 200 OK", header["Status"]);
  EXPECT_EQ("close", header["Connection"]);
  EXPECT_EQ("application/json; charset=UTF-8", header["Content-Type"]);
  EXPECT_LE(0, atoi(header["Content-Length"].c_str()));
  if (isCorsEnabled) {
    ASSERT_NO_THROW(header.at("Access-Control-Allow-Origin"));
    EXPECT_EQ("*", header["Access-Control-Allow-Origin"]);
  } else {
    EXPECT_EQ(header.find("Access-Control-Allow-Origin"), header.end());
  }

  // Jsonp
  request = generateHTTPRequest("POST", string("q=") + word + string("&format=jsonp"), outputFilename);
  execute(request.c_str());
  header = parseHeader(fileToString(outputFilename.c_str()));
  ASSERT_NO_THROW(header.at("Status"));
  ASSERT_NO_THROW(header.at("Content-Length"));
  ASSERT_NO_THROW(header.at("Connection"));
  ASSERT_NO_THROW(header.at("Content-Type"));
  EXPECT_EQ("HTTP/1.1 200 OK", header["Status"]);
  EXPECT_EQ("close", header["Connection"]);
  EXPECT_EQ("application/javascript; charset=UTF-8", header["Content-Type"]);
  EXPECT_LE(0, atoi(header["Content-Length"].c_str()));
  if (isCorsEnabled) {
    ASSERT_NO_THROW(header.at("Access-Control-Allow-Origin"));
    EXPECT_EQ("*", header["Access-Control-Allow-Origin"]);
  } else {
    EXPECT_EQ(header.find("Access-Control-Allow-Origin"), header.end());
  }

}
// Run all tests. TODO(bast): Remove and link all tests against -lgtest_main.
int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
