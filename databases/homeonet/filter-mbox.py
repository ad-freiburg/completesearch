#!/usr/bin/python
# -*- coding: utf-8 -*-
import re
import sys
import shutil
import mailbox

# Usage info
def usage_info():
  print
  print "USAGE INFO:"
  print
  print "Use filter-mbox.py to filter specific messages out of a mailbox "
  print "by using the key or the message id."
  print "\t./filter-mbox.py <INPUT_MBOX> <OUTPUT_MBOX> <LIST_OF_MAILS>"
  print 

######################################
######################################
if len(sys.argv) != 4:
  usage_info()
  exit(1)
INPUT_FILE=sys.argv[1]
OUTPUT_FILE=sys.argv[2]
LIST_OF_MAILS=sys.argv[3]

num_mails = 0
key_list = []
message_id_list = []

## Parse list of mails
mails_file = open(LIST_OF_MAILS, "r")
for mail in mails_file:
  # Found a key.
  if re.match("\d+", mail):
    key_list.append(mail)
  # Comment or empty line.
  elif mail[0] == '#' or mail[0] == '\n':
    continue;
  # Found a message id.
  else:
    message_id_list.append(re.sub("\n", "", mail))
  num_mails += 1

print "Going to delete " + str(num_mails) + " messages (some might reference" \
      + " to the same mail)"
#input_file = open(INPUT_FILE, "r")
#output_file = open(OUTPUT_FILE, "w+")
# Copy input to output to act on that.
print "Copying input to output ..."
shutil.copy(INPUT_FILE, OUTPUT_FILE)
#input_file.close()
#output_file.close()

## Parse the mbox.
mbox = mailbox.mbox(OUTPUT_FILE)
mbox.lock()
print "Start filtering ..."
for key in mbox.keys():
  if (key % 10000 == 9999):
    print "Processed " + str(key + 1) + " mails."
  # Look if that mail should be deleted by using the key.
  for i in key_list:
    if key == int(i):
      mbox.remove(key)
      print "Removed mail with key " + str(key) + "."
      key += 1
      break;
  # Look if that mail should be deleted by using the message_id.
  message = mbox.get_message(key)
  if 'Message-ID' in message:
    for i in message_id_list:
      if message['Message-ID'] == i:
        print "Removed mail with message-id " + message['Message-ID'] + "."
        mbox.remove(key)
        break;
print "Apply changes to " + OUTPUT_FILE + " ..."
mbox.flush()
mbox.unlock()

 
