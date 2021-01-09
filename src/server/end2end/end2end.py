#!/usr/bin/python
# Copyright 2011, University of Freiburg
# Chair of Algorithms and Data Structures.
# Author: Jens Hoffmann <hoffmaje>.

# File: end2end.py
# An end to end test for completesearch.

import sys, os, time, re, httplib, linecache, random
from optparse import OptionParser
from subprocess import call, Popen, PIPE
from datetime import datetime, timedelta


MODULE_DIR = os.path.dirname(__file__)
SVN        = "/usr/bin/svn"
MAKE       = "/usr/bin/make"
SCS        = "../startCompletionServer"
DATA_DIR   = os.path.abspath(os.path.join(MODULE_DIR, "data"))


class bcolors:
  "console print colors and styles"
  BOLD    = '\033[1m'
  HEADER  = '\033[95m'+BOLD
  OKBLUE  = '\033[94m'
  OKGREEN = '\033[92m'
  WARNING = '\033[93m'
  FAIL    = '\033[91m'
  ENDC    = '\033[0m'


def smaps_dict():
  s = 'Size Rss Pss Shared_Clean Shared_Dirty Private_Clean Private_Dirty Referenced Swap KernelPageSize MMUPageSize'
  return dict([(x, 0) for x in s.split()])


def smaps_dict_as_tupel(sd):
  return (sd['Size'], sd['Rss'], sd['Pss'], sd['Shared_Clean'], sd['Shared_Dirty'],
          sd['Private_Clean'], sd['Private_Dirty'], sd['Referenced'], sd['Swap'], sd['KernelPageSize'],
          sd['MMUPageSize'])


def smaps_dict_as_string_tupel(sd):
  t = smaps_dict_as_tupel(sd)
  return '{0:>7.1f} |{1:>7.1f} |{2:>7.1f} |{3:13.1f} |{4:13.1f} |{5:14.1f} |{6:14.1f} |{7:11.1f} |{8:7.1f} |{9:15.1f} |{10:12.1f} |'.format(
    t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7], t[8], t[9], t[10])


def get_memory_map(pid):
  """
  Return a smaps dictionary for given process id pid.
  """

  fd = open('/proc/%s/smaps' % pid)
  smaps_content = fd.readlines()
  fd.close()
  nof_pages = len(smaps_content) / 12
  result = {}
  for i in range(nof_pages):
    page = smaps_content[12*i:12*i+12]
    head = page[0].split()
    if len(head) == 6:
      name = head[5]
    else:
      name = 'anonymous'
    if not result.get(name): result[name] = smaps_dict()
    for l in page[1:]:
      tup = l.split()
      result[name][tup[0].strip(':')] += int(float(tup[1]))
      if tup[2] != 'kB':
        print 'Fatal: kB expected'
        sys.exit(1)
  return result


def mprint(msg, bctype, sleep=0, newline=True):
  "print message with a bcolor value ``bctype`` and sleep for ``sleep`` seconds [default 0]"
  bc = getattr(bcolors, bctype)
  msg = bc + msg + bcolors.ENDC
  if newline:
    print msg
  else:
    print msg,
    sys.stdout.flush()
  if sleep: time.sleep(sleep)


def get_head_rev_num(url):
  "return heads revision number"
  import pysvn
  return pysvn.Client().info2(url, pysvn.Revision(pysvn.opt_revision_kind.head))[0][1].get('rev').number


def mexit(retcode):
  sys.exit(retcode)


class QueryGeneratorException(Exception): pass
class QueryGenerator(object):

  _query_templ = []
  _vocabulary = None
  _nof_vocabulary = 0

  def __init__(self, query_templates, vocabulary):
    """
    Constructor.
    """
    # Parse query templates.
    fd = open(query_templates)
    lines = fd.readlines()
    for l in lines:
      if re.match('^\s*$', l): continue
      l = l.strip('\n')
      tup = (l, self._parse_query_template(l))
      self._query_templ.append(tup)
      # print tup[1]
    fd.close()
    # Determine number of lines in vocabulary.
    self._vocabulary = vocabulary
    self._nof_vocabulary = sum(1 for line in open(vocabulary))


  def _parse_query_template(self, templ):
    """
    Make template query ``templ`` evaluable. Each call to eval upon the output of this
    function (within this class) delivers a new query with randomly choosen words and prefixes.
    """
    if re.match('^.*[^ ]+ [^ ]+$', templ): return '+" "+'.join(map(self._parse_query_template, templ.split(' ')))
    if re.match('^.*[^\|]+\|[^\|]+$', templ): return '+"|"+'.join(map(self._parse_query_template, templ.split('|')))
    if re.match('^.+\*$', templ): return '+"*"'.join(map(self._parse_query_template, templ.split('*')))
    if re.match('^.+\~$', templ): return '+"~"'.join(map(self._parse_query_template, templ.split('~')))
    if re.match('^WORD$', templ): return 'self._get_word()'
    if re.match('^PREFIX$', templ): return 'self._get_prefix()'
    return ''


  def _get_word(self):
    """
    Get random word from vocabulary.
    """
    line_no = random.randint(1, self._nof_vocabulary)
    result = linecache.getline(self._vocabulary, line_no).strip('\n')
    # print 'word: %s' % result
    return result


  def _get_prefix(self):
    """
    Get random prefix from vocabulary.
    """
    word = self._get_word()
    tup = word.split(':')
    if len(tup) > 1:
      result = ':'.join(tup[0:len(tup)-1])+':'
    else:
      result = word[0:len(word)/2]
    # print 'prefix (from word %s): %s' % (word, result)
    return result


  def get_queries(self, repetitions):
    """
    Get list of generated queries.
    """
    result = []
    for qt in self._query_templ:
      entry = (qt[0], [])
      for i in range(repetitions):
        entry[1].append(eval(qt[1]))
      result.append(entry)
    return result


class CompleteSearchInstanceException(Exception): pass
class CompleteSearchInstance(object):

  _svn_url = "http://ad-svn.informatik.uni-freiburg.de/completesearch/codebase/"
  _verbose_name = None
  _path = None
  _rev = None
  _db_prefix = None
  _queries = []
  _results = []


  @classmethod
  def from_revision(cls, verbose_name, rev, rev_dir, db_prefix, db_path):
    """
    Constructor for instances to get from a revision.
    """
    obj = cls()
    obj._db_prefix = db_prefix
    obj._db_path = db_path
    if rev == 'HEAD':
      obj._rev = get_head_rev_num(obj._svn_url)
    else:
      try:
        obj._rev = int(float(rev))
      except Exception, e:
        raise CompleteSearchInstanceException('truth-base-rev must be number|HEAD')
    obj._verbose_name = '%s instance (from revision %d)' % (verbose_name, obj._rev)
    obj._path = os.path.join(rev_dir, 'completesearch-%s.r%d' % (verbose_name, obj._rev))
    return obj


  @classmethod
  def from_path(cls, verbose_name, path, db_prefix, db_path):
    """
    Constructor for instances from a path.
    """
    obj = cls()
    obj._db_prefix = db_prefix
    obj._db_path = db_path
    obj._path = path
    obj._verbose_name = '%s instance (from path %s)' % (verbose_name, obj._path)
    return obj


  def checkout(self):
    """
    Checkout instance if instance is not be used from path.
    """
    if not self._rev: return
    mprint('Checkout %s:' % self._verbose_name, 'HEADER', 1)
    retval = call([SVN, 'checkout', self._svn_url, '-r', '%s' % self._rev, self._path])
    if retval != 0: raise CompleteSearchInstanceException('cant checkout revision %d' % self._rev)


  def _adjust_makefile(self, makefile):
    """
    Set CS_CODE_DIR in makefile.
    """
    fd = open(makefile)
    newlines = []
    found_CS_CODE_DIR = False
    for line in fd.readlines():
      m = re.match('^CS_CODE_DIR\s*=', line)
      if m:
        found_CS_CODE_DIR = True
        cs_code_dir = os.path.dirname(makefile)
        newlines.append('CS_CODE_DIR\t= %s\n' % cs_code_dir)
      else:
        newlines.append(line)
    fd.close()
    if not found_CS_CODE_DIR:
      raise CompleteSearchInstanceException('Makefile %s has no identifier ``CS_CODE_DIR``.' % makefile, 'FAIL')
    # Write back newlines to Makefile.
    fd = open(makefile, 'w')
    fd.writelines(newlines)
    fd.close()


  def build(self):
    """
    Run the build command.
    """
    makefile = os.path.join(self._path, 'Makefile')
    self._adjust_makefile(makefile)
    mprint('Build %s:' % self._verbose_name, 'HEADER', 1)
    retval = call([MAKE, '-C', self._path, 'build-all'])
    if retval != 0: raise CompleteSearchInstanceException('cant use %s due to build errors' % self._verbose_name, 'FAIL')


  def run_server(self):
    """
    Start completesearch server.
    """
    mprint('Start server for %s:' % self._verbose_name, 'HEADER', 1)
    hyb_index_file = os.path.join(self._db_path, self._db_prefix + '.hybrid')
    pid_file = self._db_prefix + '.pid'
    if os.path.exists(pid_file): os.remove(pid_file)
    self._server_process = Popen([SCS, '-F', '-P', pid_file, '-t', 'HYB', hyb_index_file])

    # Wait for end of server init phase.
    while True:
      try:
        self.send_query('wait')
        break
      except Exception, e:
        print e
        time.sleep(1)


  def stop_server(self):
    """
    Stop completesearch server.
    """
    self._server_process.kill()


  def send_query(self, query):
    """
    Send single query to running server.
    """
    conn = httplib.HTTPConnection('127.0.0.1', 8888)
    conn.request('GET', '/?q=%s' % query)
    r = conn.getresponse()
    conn.close()
    return r.read()


  def send_queries(self, queries):
    """
    Send a list of queries to running server and do some evaluation
    for each query send.
    """
    avg_mem_heap = smaps_dict()
    avg_mem_stack = smaps_dict()
    avg_mem_other = smaps_dict()
    avg_mem_total = smaps_dict()
    page_cnt = heap_cnt = stack_cnt = other_cnt = 0

    # Determine length of longest query template entry.
    max = 0
    for q in queries:
      length = len(q[0])
      if length > max: max = length

    # Send queries.
    cnt2 = 0
    query_time = timedelta()
    print '--- START REQUESTS ---'
    for q in queries:
      cnt = 1
      fill = ''.join([' ' for x in range(max - len(q[0]))])
      entry = (q[0], [])
      for r in q[1]:
        print '\rSend queries of type %s%s : %d/%d' % (q[0], fill, cnt, len(q[1])),
        sys.stdout.flush()
        t0 = datetime.now()
        response = self.send_query(r)
        t1 = datetime.now()
        query_time += t1 - t0
        memory = get_memory_map(self._server_process.pid)
        entry[1].append((r, response, memory),)
        for page in memory:
          page_cnt += 1
          for key in memory[page]: avg_mem_total[key] += memory[page][key]
          if page == '[heap]':
            for key in memory[page]: avg_mem_heap[key] += memory[page][key]
            heap_cnt += 1
          elif page == '[stack]':
            for key in memory[page]: avg_mem_stack[key] += memory[page][key]
            stack_cnt += 1
          else:
            for key in memory[page]: avg_mem_other[key] += memory[page][key]
            other_cnt += 1

        self._results.append(entry)
        cnt += 1
        cnt2 += 1
      print ''
    print '--- END ---'
    nof_queries_per_second = cnt2*1.0 / (query_time.seconds + query_time.microseconds / 1000000.0)
    fill = ''.join([' ' for x in range(max)])
    print 'Queries per second %s : %f (high precission)' % (fill, nof_queries_per_second)
    for key in avg_mem_heap:  avg_mem_heap[key] = avg_mem_heap[key]   * 1.0 / heap_cnt
    for key in avg_mem_stack: avg_mem_stack[key] = avg_mem_stack[key] * 1.0 / stack_cnt
    for key in avg_mem_other: avg_mem_other[key] = avg_mem_other[key] * 1.0 / other_cnt
    for key in avg_mem_total: avg_mem_total[key] = avg_mem_total[key] * 1.0 / page_cnt
    mem_stat = """
Average memory consumption (in kB), considering %d memory lookups (in /proc/<pid>/smaps):

        |  Size  |  Rss   |  Pss   | Shared_Clean | Shared_Dirty | Private_Clean | Private_Dirty | Referenced |  Swap  | KernelPageSize | MMUPageSize |
  ------+--------+--------+--------+--------------+--------------+---------------+---------------+------------+--------+----------------+-------------+
  heap  |%s
  stack |%s
  other |%s
  ------+--------+--------+--------+--------------+--------------+---------------+---------------+------------+--------+----------------+-------------+
  total |%s
  """ % (page_cnt, smaps_dict_as_string_tupel(avg_mem_heap), smaps_dict_as_string_tupel(avg_mem_stack),
         smaps_dict_as_string_tupel(avg_mem_other), smaps_dict_as_string_tupel(avg_mem_total))
    print mem_stat
    return self._results


def main():
  # Load globals.
  svn = SVN
  make = MAKE
  data_dir = DATA_DIR

  # Set up and parse options.
  usage = 'Usage: %prog [options] truth-base-rev database-prefix database-path'
  parser = OptionParser(usage, version='%prog 0.1')
  parser.add_option('--dodgy-rev', dest='dodgy_rev',
                    help='revision to compare with truth-base-rev',
                    metavar='NUM|HEAD')
  parser.add_option('--dodgy-path', dest='dodgy_path',
                    help='path to completesearch codebase',
                    metavar='PATH')
  parser.add_option('--rev-cache', dest='rev_cache',
                    help='store and use revisions in PATH',
                    metavar='PATH')
  parser.add_option('--repeat', dest='repeat', default='100',
                    help='send NUM (different) queries for each query template [default: %default]',
                    metavar='NUM')
  (options, args) = parser.parse_args()

  # Get and check trailing arguments.
  if len(args) != 3:
    parser.error('incorrect number of arguments')

  try:
    repeat_queries = int(float(options.repeat))
  except:
    parser.error('option repeat expects a number')

  truth_base_rev = args[0]
  db_prefix = args[1]
  db_path = args[2]

  if not os.path.exists(db_path):
    parser.error('no such path: %s' % db_path)

  rev_dir = options.rev_cache if options.rev_cache else data_dir

  # The truth-base instance.
  truth_instance = CompleteSearchInstance.from_revision('truth-base', truth_base_rev, rev_dir, db_prefix, db_path)

  # The dodgy instance.
  if options.dodgy_rev and options.dodgy_path:
    parser.error('options dodgy-rev and dodgy-path must not appear together')
  if options.dodgy_rev:
    dodgy_instance = CompleteSearchInstance.from_revision('dodgy', options.dodgy_rev, rev_dir, db_prefix, db_path)
  else:
    path = options.dodgy_path if options.dodgy_path else os.path.abspath(os.path.join(MODULE_DIR, '..', '..'))
    dodgy_instance = CompleteSearchInstance.from_path('dodgy', path, db_prefix, db_path)


  # Checkout and build instances.
  truth_instance.checkout()
  dodgy_instance.checkout()
  truth_instance.build()
  dodgy_instance.build()

  # Generate random queries from query-templates.
  qg = QueryGenerator('query-templates', os.path.join(db_path, db_prefix+'.vocabulary'))
  queries = qg.get_queries(repeat_queries)

  # Send queries to truth-base instance.
  truth_instance.run_server()
  result_truth = truth_instance.send_queries(queries)
  truth_instance.stop_server()

  # Send queries to dodgy instance.
  dodgy_instance.run_server()
  result_dodgy = dodgy_instance.send_queries(queries)
  dodgy_instance.stop_server()

  # Compare results.
  nof_differences = 0
  for i in range(0, len(result_truth)):
    for j in range(0, len(result_truth[i][1])):
      if result_truth[i][1][j] != result_dodgy[i][1][j]:
        nof_differences += 1
        print 'Query results different for query: %s' % result_truth[i][0]
  print 'Number of different query results: %d' % nof_differences


if __name__ == '__main__': main()
