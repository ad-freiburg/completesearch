#!/usr/bin/python
# -*- coding: utf-8 -*-
import re
import sys
import mailbox
import unicodedata
from email.Header import decode_header
from htmlentitydefs import codepoint2name
from xml.sax.saxutils import escape
import html5lib
from bs4 import BeautifulSoup


# method for extracting html entities out of an also implemented map in
# htmlentitydefs.
unichr2entity = dict((unichr(code), u'&%s;' % name)
for code,name in codepoint2name.iteritems()
if code!=38) # exclude "&"

def htmlescape(text, d=unichr2entity):
  if u"&" in text:
    text = text.replace(u"&", u"&amp;")
  for key, value in d.iteritems():
    if key in text:
      text = text.replace(key, value)
  return text

# Usage info
def usage_info():
  print
  print "USAGE INFO:"
  print
  print "Use convertmbox.py to convert a mbox to either XML or CSV."
  print
  print "\t./convertmbox.py <FORMAT> <DB>"
  print 
  print "DB should be the prefix of the output file and the input file:"
  print "  for example (DB = data/test): data/test.mbox -> data/test.xml"
  print
  print "Formats:"
  print "  XML:\n    <homeonet>"
  print "      <mail>"
  print "        <author>...</author>"
  print "        <title>...</title>"
  print "        <url>...</url>"
  print "        <text>\"<CDATA stuff>\"</text>"
  print "      </mail>\n      ..."
  print "    </homeonet>"
  print
  print "  CSV:"
  print "    author  title  url excerpts"
  print

# Just a simple method for a standard version of printing an exception (or
# problems anyway).
def my_warning(text, detail=""):
  print "WARNING:", text,
  if detail != "":
    print "(", detail, ")",
  print "!"
  return

def createXmlValidHeader(text):
  # We can either use htmlescape (and get &ouml; and so on) or just escape. Both
  # works. But they are also both a bit tricky. htmlescape creates entities
  # which needs to be mapped within the homeonet.dtd, else it won't lint.
  # escape() doesn't escape special characters like ö,ä, etc., so they need to be
  # specified first.
  if (FORMAT == "XML"):
    try:
      text = htmlescape(text)
    except Exception as detail:
      my_warning("could not replace chars by html entities: " + text, detail)
      exit(1)
  #try:
  #  text = escape(text)
  #except:
  #  my_warning("could not escape header: " + text, detail)

  # Now encode to utf-8.
  try:
    assert(text.__class__.__name__ == "unicode")
    text = text.encode('utf-8', "replace")
  except Exception as detail:
    my_warning("could not convert header to utf-8: " + text, detail)
    exit(1)
  if not text or text == "":
    my_warning("createXmlValidHeader(): empty header, don't know why")
    exit(1)

  return text

def decodedPartOfHeader(header):
  # In some cases it happenes that authors have a structue like 
  # =?Utf-8?Q?Ilse H=C3=Bcbler-Bier?= or
  # =?Iso-8859-1?Q?B=E4Rbel Leucht?=. Just decode it.
  # Moreover take care of umlauts, etc., which should be represented as
  # htmlentity.

  # HACK: x-user-defined is not a valid charset, therefore we need to set one.
  # I decided to use utf-8, since it offers kind of all letters and didn't throw
  # an error doing it.
  # header = header.replace("x-user-defined", "unicode")
  # doesn't work
  
  # Sometimes the format isn't encoding "=?<encoding>...", but "<text>
  # =?<encoding>. Handle this by splitting the string and decoding just the
  # second part of it.
  firstPart = -1
  if header.find("?="):
    firstPart = header.find("=?") 
  if firstPart > 1:
    secondPart = header[firstPart:]
    firstPart = header[0:firstPart]
    header = secondPart
  else:
     firstPart = ""

  # Start decoding.
  try:
    # =?Utf-8?Q?Ilse H=C3=Bcbler-Bier?= 
    # to
    # Ilse Hübler Bier
    decoded = decode_header(header)
  except Exception as detail:
    my_warning("could not decode header: " + header, detail)
    exit(1)
  # decoded[0][1] stores the encoding of the header, which can be used for
  # decoding the message.
  if decoded[0][1]:
    try:
      header = (firstPart + decoded[0][0]).decode(decoded[0][1])
    except Exception as detail:
      my_warning("could not decode header to unicode: " + header, detail)
      exit(1)
  if not header or header == "":
    my_warning("decodedPartOfHeader(): empty header, don't know why")
    exit(1)
  return header

def decodeText(text, charset):
  # Convert text to unicode. Note that BeautifulSoup already returns
  # unicode so we don't have to do anything in that case (and the unicode
  # constructor below would even yield an error, namely "decoding Unicode is not
  # supported".
  if text.__class__.__name__ != "unicode":
    if charset == "iso-8859-1":
      try:
        text = unicode(text, "latin-1", "replace")
      except Exception as detail :
        my_warning("could not convert message from iso-8859-1 to unicode", detail)
        exit(1)
    elif charset == "default":
      try:
        text = unicode(text, "ascii", "replace")
      except Exception as detail :
        my_warning("could not convert message from us-ascii to unicode", detail)
        exit(1)
    elif charset == "cp-1252":
      try:
        text = unicode(text, "cp1252", "replace")
      except Exception as detail :
        my_warning("could not convert message from cp1252 to unicode", detail)
        exit(1)
    elif charset == "macintosh":
      try:
        text = unicode(text, "mac_roman", "replace")
      except Exception as detail :
        my_warning("could not convert message from us-ascii to unicode", detail)
        exit(1)
    elif charset == "" or charset == " ":
      my_warning("no charset given, cannot convert to unicode")
      exit(1)
    # HACK: x-user-defined is not a valid charset, therefore we need to set one.
    # I decided to use utf-8, since it offers kind of all letters and didn't throw
    # an error doing it.
    elif charset == "x-user-defined":
      try:
        text = unicode(text, "utf-8", "replace")
      except:
        my_warning("don't now what to do with \"charset\" x-user-defined, tried " 
                 + "to decode as utf-8, but didn't work out")
        exit(1)
    else:
      try:
        text = unicode(text, charset, "replace")
      except Exception as detail:
        my_warning("could not convert " + charset + " to unicode", detail)
        exit(1)
  if not text or text == "":
    my_warning("decodeText(): empty text, don't know why")
    exit(1)

  # Erase characters which are invalid in either xml and/or utf-8.
  # This is done here, since we need a unicode string to do it properly.
  text = re.sub(invalidXmlRegEx, "", text)
  return text

# Within the date the months are written in their short form. Since we need the
# long form for creating the url of the message, use a map.
def get_month(short):
  months = {
    "Jan": "January",
    "Feb": "February",
    "Mar": "March",
    "Apr": "April",
    "May": "May",
    "Jun": "June",
    "Jul": "July",
    "Aug": "August",
    "Sep": "September",
    "Oct": "October",
    "Nov": "November",
    "Dec": "December"
    }
  try:
    return months[short]
  except Exception as detail:
    my_warning("could not get to full name of the month: " + month, detail)
    exit(1)

####################################################
####################################################
####################################################
####################################################
invalidXmlRegEx=u'[\x00-\x1f\x7f-\x84\x86-\x9f\ud800-\udfff\ufdd0-\ufddf\ufffe-\uffff]'
if len(sys.argv) != 3:
  usage_info()
  exit(1)
DB=sys.argv[2]
FORMAT=sys.argv[1]
file_name = DB + '.mbox'
if (FORMAT == "XML"):
  output_file_name = DB + '.xml'
elif (FORMAT == "CSV"):
  output_file_name = DB + '.csv'
else:
  print("Wrong parameters.\n")
  usage_info()
  exit(1)
numberOfContinuedMails = 0
oldkey = -1

## Parse the mbox.
file = open(file_name, "r")
output = open(output_file_name, "w")
if (FORMAT == "XML"):
  output.write("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n")
  output.write("<!DOCTYPE homeonet SYSTEM \"homeonet.dtd\">\n")
  output.write("<homeonet>\n")
else:
  output.write("author\ttitle\turl\texcerpts\n")

mbox = mailbox.mbox(file_name)

for key in mbox.keys():
  message = mbox.get_message(key)
  if (key != oldkey + 1):
    my_warning("could not get " + str(key - oldkey - 1) + " messages from mbox")
    numberOfContinuedMails += key - oldkey - 1
  oldkey = key
 
  # Get headers before walking down, since they get lost elsewise.
  title = message.get('Subject')
  author = message.get('From')
  date = message.get('Date')

  # There really seems to be neither subject nor author:
  # reasons are: mbox module read them out wrongly (message has no header,
  # starts within the text); wrong formatted header, like missing : after
  # subject or from; non standard header (?), which means that the header isn't
  # at the total top of the message
  if not (title and author):
    numberOfContinuedMails += 1
    my_warning("could not find neither subject nor author")
    continue

  # Get only first part of mult-part message (if the first part is a multi-part
  # message again, we consider the first part of that, and so on, until nesting
  # level 10, if there are more levels, something is probably wrong).
  nesting_level = 0;
  while message.is_multipart() is True:
    message = message.get_payload(0);
    nesting_level += 1
    if (nesting_level >= 10):
      my_warning("multi-part message with nesting level > 10", "")
      numberOfContinuedMails += 1
      continue
  if nesting_level > 0:
    pass
    #print "[multi-part message, taking just first part]"

  # Message should be single-part now. The argument "True" in the call to
  # get_payload makes sure that the message is decoded accorded to the
  # Content-Transfer-Encoding header (if it is "quoted-printable" or "base64").
  assert message.is_multipart() is False
  message_text = message.get_payload(decode=True)
  content_type = message.get_content_type()
  charset = message.get_content_charset("default").split(";")[0]
  #print "[Content-type:", content_type ,"; charset:", charset, "]"
  #print 

  # Sometimes we get empty messages, don't parse them. The reason for that is
  # unknown by now.
  if not message_text or message == "":
    my_warning("empty message")
    numberOfContinuedMails += 1
    continue

  # Content type: for now only consider text/plain and text/html. Use
  # BeautifulSoup to extract the text from text/html.
  # BEWARE: BeautifulSoup returns a unicode object, not a string!!!
  if content_type == "text/html" or content_type == "text/enriched" or content_type == "multipart/alternative":
    try:
      message_text = BeautifulSoup(message_text)
      message_text = message_text.get_text()
    except Exception as detail:
      my_warning("BeautifulSoup parse error", detail)
      numberOfContinuedMails += 1
      continue
  elif content_type != "text/plain":
    my_warning("content type neither text/plain nor text/html", content_type)
 
  # Try to decode the message to unicode, if it's not already.
  # This method already includes removing non-utf-8 or xml invalid byte
  # sequences.
  try:
    message_text = decodeText(message_text, charset) 
  except:
    numberOfContinuedMails += 1
    continue

  # Now it really should be in unicode.
  if not message_text.__class__.__name__ == "unicode":
    my_warning("message is still not unicode", message_text.__class__.__name__)
    numberOfContinuedMails += 1
    continue

  # Try to make it xml valid, by escaping entities, removing ]] to avoid
  # problems with CDATA and encoding it to utf-8.
  try:
     message_text = re.sub("]]>", "", message_text)
     message_text = escape(message_text)
     message_text = message_text.encode("utf-8", "replace")
  except Exception as detail:
    my_warning("could not convert message from unicode to utf-8", detail)
    numberOfContinuedMails += 1
    continue
  
  text = message_text

  # base url of the messages (needs to be combined with the date and the key of the message)
  #url = "http://lists.informatik.uni-freiburg.de/pipermail/homeo/"
  url = "http://www.homeonet.org/homeonet/"
  
  # Use date for creating the url of the message.
  if date:
    date_parts = date.split(" ")
    # Delete empty elements, which can occur because of two whitespaces.
    for d in date_parts:
      if d == "":
        date_parts.remove(d)
    try:
    #  if re.match("\d+", date_parts[2]):
    #    month = get_month(date_parts[1])
    #  else:
    #    month = get_month(date_parts[2])
      year = date_parts[3]
    #  url += year + "-" + month + "/" + str(key).zfill(6) + ".html"
      url += year + "/" + str(key).zfill(6) + ".html"
    except:
      pass

  # Process author.
  if author:
    try:
      author = decodedPartOfHeader(author)
      author = decodeText(author, charset)
    except:
      numberOfContinuedMails += 1
      continue;

    # Capitalize all words in author
    author = author.title()
    # Erase mail from author tag. If there is just the mail and no name, so take
    # everything before @ as author.
    author = re.sub(' <.*?>', "", author)
    author = author.replace('<',"")
    author = author.replace('>',"")
    justmail = re.match('(.*?)@.*?', author)
    if justmail:
      author = justmail.group(1)
    # Erase " from author.
    author = author.replace('\"',"")
    try:
      author = createXmlValidHeader(author)
    except:
      numberOfContinuedMails += 1
      continue;
  
  # Process title.
  if title:
    try:
      title = decodedPartOfHeader(title)
      title = decodeText(title, charset)
      title = createXmlValidHeader(title)
    except:
      numberOfContinuedMails += 1
      continue;
    if date and author:
      title += " (" + author + ", " + date_parts[1] + " " + date_parts[2] + " " + date_parts[3] + ")"
  
  # Write to xml.
  if (author or title) and (FORMAT == "XML"):
    output.write("<mail>\n")
    if author:
      output.write("<author>" + author + "</author>\n")
    else:
      output.write("<author>Unknown</author>\n")
    if title:
      output.write("<title>" + title + "</title>\n")
    else:
      output.write("<title>Kein Betreff</title>\n")
    if url:
      output.write("<url>" + url + "</url>\n")
    output.write("<text><![CDATA[" + text + "]]></text>\n") 
    output.write("</mail>\n")
  elif author and title and url and text and FORMAT == "CSV":
    # Since we use csv we need to make sure, that there are absolutely no tabs
    # and line breaks within the variables.
    author = re.sub("\t", " ", author)
    title = re.sub("\t", " ", title)
    text = re.sub("[\t\r\n]+", " ", text)
    output.write(author + "\t" + title + "\t" + url + "\t" + text + "\n")

print "Jumped over " + str(numberOfContinuedMails) + " mails."
if (FORMAT == "XML"):
  output.write("</homeonet>")
