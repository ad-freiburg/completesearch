#!/usr/bin/python
import os, sys, cgi, time, string, pdb, subprocess, getopt
import xml.etree.ElementTree as ET
from xml.dom import minidom
from operator import attrgetter

class Result:
	def __init__(self, date, match, query, id, time, unit, status, memory):
		self.date = date	
		self.match = match
		self.query = query
		self.id = id
		self.time = time
		self.unit = unit
		self.status = status
		self.memory = memory
		
	def __repr__(self):
		return repr((self.match, self.query, self.id, self.time, self.unit, self.status, self.memory))

class ServerPerformanceStressTest:
	time_threshold = 15
	memory_threshold = 3000
	queryParameters = "&c=10&h=20&"
	xmlTestData = []
	xmlOriginalData = []
	serverRequest = ''
	csv = False
	output_filename_details = "perf-results.xml-all-queries"
	output_filename = "perf-results.xml-summary"
	output_filename_badqueries = "perf-results.xml-badqueries"
	resultList = []

	# Type: 0 -> output + perf, 1 -> perf
	def __init__(self, collection_name, queries_filename, test_filename, pidfile, type):
		try:
			self.test_date = time.strftime("%d%b%y", time.localtime())
			self.queries_filename = queries_filename
			if (str.find(queries_filename, 'csv') != -1):
				self.output_filename_details = "perf-results.csv-all-queries"
				self.output_filename = "perf-results.csv-summary"
				self.output_filename_badqueries = "perf-results.csv-badqueries"
			self.test_filename = test_filename
			self.collection_name = collection_name
			# Split pidfilepath to "/home/user/.completesearch", "host_port.pid"
			cut = pidfile.partition('_')
			# Split second part of cut to "host" "port.pid"
			cut = cut[2].partition('_')
			self.host = cut[0]
			# Split second part of cut to "port" "pid"
			cut = cut[2].partition('.')
			self.port = int(cut[0])
			self.serverRequest = self.host + ":" + str(self.port) + "/?q="
			pid_file = open(pidfile, "r")
			self.pid = pid_file.readline()
			self.type = type;
                except IOError:
			print ("Error: Exception in class initialization of ServerPerformanceStressTest: Could not open %s." % error)
		except Exception as error:
			print ("Error: Exception in class initialization of ServerPerformanceStressTest: %s" % error)
			sys.exit(2)
	
	def analyseProcess(self):
		try:
			args = ['ps', 'u', '-p', str(self.pid)]
			ps = subprocess.Popen(args, shell=False, stdout=subprocess.PIPE)
			ps.wait()
			if (ps.returncode < 0):
				sys.stderr.write("Couldn't read out pid by using ps.\n")
				sys.stderr.flush()
				sys.exit(2)
			args = ['awk', '{print $5}']
			memp = subprocess.Popen(args, shell=False, stdin=ps.stdout, stdout=subprocess.PIPE)
			memp.wait()
			if (memp.returncode < 0):
				sys.stderr.write("Couldn't read out memory by using awk.\n")
				sys.stderr.flush()
				sys.exit(2)
			mem = memp.stdout.readlines()[1]
			return mem
		except Exception as error:
	     		print ("Error: Exception in class initialization of ServerPerformanceStressTest: %s" % error)
			sys.exit(2)

		
	def analyseServerRequest(self, xmlTest, xmlOriginal):
		try:
			date = time.strftime("%H:%M:%S %d.%m.%Y", time.localtime())
		  	mem = self.analyseProcess()
			root = ET.fromstring(xmlOriginal.encode('utf-8'))
			query = root.find('query').text
			id = root.find('query').get('id')
			status = root.find('status').text
			time_needed = root.find('time').text
			unit = root.find('time').get('unit')
			identic = ""
			if (self.type == 0):
				oHits = ""
				if (status == "OK"):
					oHits = root.find('hits').text
				rootTest = ET.fromstring(xmlTest);
				testHits = rootTest.find('hits').text
				if (oHits == testHits):
					identic = "EQUAL"
				else:
					identic = "DIFFER"
			return Result(date, identic, query, id, float(time_needed), unit, status, float(mem) / 1024)
		except Exception as error:
			print ("Error: Exception in analyseServerRequest(): %s\n%s" % (error, xmlOriginal))
			sys.exit(2)
	
	def getStatistics(self):
		try:
			if (self.type == 0):
				self.getTestData()
			self.resultList = []
			badqueries = ''
			self.getOriginalXmlData()
			statistics = ''
			average = 0.0
			errors = 0
			differ = 0
			minMemory = 9999999;
			maxMemory = 0;
			for position in range(len(self.resultList)):
				resultString = ''
				average = average + self.resultList[position].time
				if (self.type == 0):
					resultString = ("%s\t%s\t%s\t%5.2f %-5s\t%3.2f MB\t%s\n") \
						% (self.resultList[position].id \
						, self.resultList[position].query \
						, self.resultList[position].match \
						, self.resultList[position].time \
						, self.resultList[position].unit \
						, self.resultList[position].memory \
						, self.resultList[position].date)
				else:
					resultString = ("%s\t%s\t%s\t%5.2f %-5s\t%3.2f MB\t%s\n") \
						% (self.resultList[position].id \
						, self.resultList[position].query \
						, self.resultList[position].status \
						, self.resultList[position].time \
						, self.resultList[position].unit \
						, self.resultList[position].memory \
						, self.resultList[position].date)

				statistics += resultString
				if (self.resultList[position].memory > maxMemory):
					maxMemory = self.resultList[position].memory
				if (self.resultList[position].memory < minMemory):
					minMemory = self.resultList[position].memory
				if (self.resultList[position].match == "DIFFER"):
					badqueries += resultString
					differ = differ + 1
				if (self.resultList[position].status == "error"):
					badqueries += resultString
					errors = errors + 1
				elif (self.resultList[position].time >= self.time_threshold):
					badqueries += resultString
			self.resultList = sorted(self.resultList, key = attrgetter('time')) # sort by time
			if (len(self.resultList) % 2) == 0:
				percentile90 = (self.resultList[int(len(self.resultList)*0.9 - 1)].time \
						+ self.resultList[int(len(self.resultList)*0.9)].time)/2.0 
				percentile99 = (self.resultList[int(len(self.resultList)*0.99 - 1)].time \
						+ self.resultList[int(len(self.resultList)*0.99)].time)/2.0 
				median = (self.resultList[int(len(self.resultList)*0.5 - 1)].time + \
						self.resultList[int(len(self.resultList)*0.5)].time)/2.0
			else:
				median = self.resultList[int(len(self.resultList)*0.5)].time
				percentile90 = self.resultList[int(len(self.resultList)*0.9)].time
				percentile99 = self.resultList[int(len(self.resultList)*0.99)].time
			wholestatistics = statistics
			date = time.localtime();
			date = time.strftime("%H:%M:%S %d.%m.%Y", (date))
			statistics = ("\n\n\033[1mSummary of testing server %s on port %d (%d queries sent) - %s\033[0m\n\n") \
							% (self.host, self.port, len(self.resultList), date)
			statistics += ("%d requests of %d queries sent were erroneous.\n") % (errors, len(self.resultList))
			if (self.type == 0):
				statistics += ("%d requests of %d queries sent were different to their xml data.\n") % (differ, len(self.resultList))
			statistics += ("%-18s %8.2f msecs\n") % ("Average:", average / float(len(self.resultList)))
			statistics += ("%-18s %8.2f msecs\n") % ("Median:", median)
			statistics += ("%-18s %8.2f msecs\n") % ("90th Percentile:", percentile90)
			statistics += ("%-18s %8.2f msecs\n") % ("99th Percentile:", percentile99)
			statistics += ("%-18s %8.2f msecs\n") % ("Minimal time:", self.resultList[0].time)
			statistics += ("%-18s %8.2f msecs\n") % ("Maximal time:", self.resultList[len(self.resultList) - 1].time)
			statistics += ("%-18s %5d MB\n") % ("Minimal memory usage:", minMemory)
			statistics += ("%-18s %5d MB\n") % ("Maximal memory usage:", maxMemory)
		
			prefix = self.collection_name + "." + self.test_date + "."
			self.saveStatistics(wholestatistics, prefix +	self.output_filename_details, "w")
			self.saveStatistics(statistics, prefix + self.output_filename, "w")
			self.saveStatistics(badqueries, prefix + self.output_filename_badqueries, "w")
			# No bad error.
			if (self.resultList[len(self.resultList) - 1].time >= self.time_threshold):
			  sys.stderr.write(statistics)
			  sys.stderr.flush()
                          sys.stderr.write("\nMaximal time exceeds time threshold: %0.2f > %0.2f." \
                                % (self.resultList[len(self.resultList) - 1].time, \
                                self.time_threshold))
			  sys.exit(1)
			# Bad error.
			if (errors > 0) or (differ > 0) or (maxMemory > self.memory_threshold):
			  sys.stderr.write(statistics)
			  sys.stderr.flush()
                          if (errors > 0):
                            sys.stderr.write("\n%d error(s) occured." % errors)
                          elif (differ > 0):
                            sys.stderr.write("\n%d differ(s) occured." % differ)
                          else:
                            sys.stderr.write("\nMaximal memory exceeds memory \
                                threshold: %d > %d." % (maxMemory, \
                                self.memory_threshold))
			  sys.exit(2)
			return 0
		except Exception as error:
			print ("Error: Exception in getStatistics(): %s" % error)
			sys.exit(2)

	def saveStatistics(self, statistics, file, append_or_write):
		try:
			stat = open(file, append_or_write)
			stat.write(statistics)
			stat.close()
		except Exception as error:
			print ("Error: Exception in saveStatistics(): %s" % error)
			sys.exit(2)

	def sendQueryAndGetXml(self, query):
		try:
			url = self.serverRequest + query + self.queryParameters;
			args = ['curl', '-s', url]
                        # print args # DEBUG
			xml = subprocess.Popen(args, shell=False, stdout=subprocess.PIPE).stdout.read().decode("utf-8", "replace")
			return xml
		except Exception as error:
			print ("Error: Exception in sendQueryAndGetXml(): %s" % error)
			sys.exit(2)

	def getOriginalXmlData(self):
		try:
			self.xmlOriginalData = []
			queryfile = open(self.queries_filename, "r")
			i=0
			for line in queryfile:
                                line = line.rstrip()
			  	if len(line) == 0:
				  	print "Found empty line."
				  	continue;
				test = ""
				if (self.type == 0):
					test = self.xmlTestData[i]
				xml = self.sendQueryAndGetXml(line)
				if len(xml) == 0:
				  	print "Empty result for query " + line
				  	continue;
				result = self.analyseServerRequest(test, xml)
				self.resultList.append(result)

		except Exception as error:
			print ("Exception in getOriginalXmlData(): %s" % error)
			sys.exit(2)

	def getTestData(self):
		try:
			testfile = open(self.test_filename, "r")
			xml = ''
			for line in testfile:
				if str.find(line, "version=\"1.0\"") != -1:
					list = [xml, '', 0, 0]
					if (xml != ''):
						self.xmlTestData.append(xml)
					xml = ''
				else:
					xml += line
		except Exception as error:
			print ("Error: Exception in getTestData(): %s", error)
			sys.exit(2)

	def storeXml(self):
		try:
			xml = ''
			queryfile = open(self.queries_filename, "r")
			for line in queryfile:
				xml += (self.sendQueryAndGetXml(line.rstrip()))
			queryfile.close()
			testfile = open(self.test_filename ,"w")
			testfile.write(xml)
			testfile.close()
		except Exception as error:
			print ("Error: Exception in storeXml(): %s" % error)
			sys.exit(2)

def main():
		try:
			opts, args = getopt.getopt(sys.argv[1:], "gpdh", ["generate-xml-data", "perf-test", "diff-test", "help"])
		except getopt.GetoptError as err:
			# print help information and exit:
			print(err) 
			usage()
			sys.exit(2)
		if (len(args) != 4):
			usage()
			sys.exit(2)
		if (len(opts) == 0):
			opts = ['-d']
		for o, a in opts:
			if o in ("-g", "--generate-xml-data"):
				#print ("Generate XmlData ...")
				testInstance = ServerPerformanceStressTest(args[0], args[1], args[2],	args[3], 0)
				return testInstance.storeXml()
			elif o in ("-p",	"--perf-test"):
				testInstance = ServerPerformanceStressTest(args[0], args[1], args[2],	args[3], 1)
				#print ("Test Performance ...")
				return testInstance.getStatistics()
			elif o in	("-d", "--diff-test"):
				testInstance = ServerPerformanceStressTest(args[0], args[1], args[2], args[3], 0)
				#print ("Test Performance + Diff ...")
				return testInstance.getStatistics()
			elif o in ("-h", "--help"):
				usage()
				sys.exit(0)
			else:
				testInstance = ServerPerformanceStressTest(args[0], args[1], args[2],	args[3], 1)
				#print ("Test Performance + Diff ...")
				return testInstance.getStatistics()
		return 0 # exit errorlessly

def usage():
	sys.stderr.write("\nUsage info:\n")
	sys.stderr.write("\t./perf.evaluate-queries.py <collection name> ")
	sys.stderr.write("<input queries file> <testdata file> <pid file>\n")
	sys.stderr.write("Options:\n")
	sys.stderr.write("\t --diff-test, -d:\t <default> Testing performance + diff test\n")
	sys.stderr.write("\t --generate-xml-data, -g:\t Generate XmlData for diff. XmlData should be checked for errors afterwards manually.\n")
	sys.stderr.write("\t --perf-test, -p:\t Testing performance without comparing XmlOutput\n\n")
	sys.stderr.write("Description:\n")
	sys.stderr.write("This script can be used to test performance and output of a \
completesearch instance on a specific port and host.\nIt's important to execute \
this script on the same host on which the instance is running. Else the \
pid cannot be readout.\nThis script tests: memory, speed and output.\nTo test \
the output it is necessary to generate and check the xml data first.\nSince the xml data \
depends on the xml file given to the instance, the output test should be \
performed on a instance with static xml and changing code.\n")
	sys.stderr.flush()
if __name__ == '__main__':
	sys.exit(main())
