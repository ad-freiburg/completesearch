# -*- coding: utf-8 -*-

# Copyright 2021, University of Freiburg
# Chair of Algorithms and Data Structures
# Author: Hannah Bast <bast@cs.uni-freiburg.de>

"""
Goal of this code: make a TSV file from Satzungen (Prüfungsordnungen, Gesetze,
etc.), where each line corresponds to one unit from information and each line
knows its associated parent headers. For Satzungen, this unit is typically an
"Absatz" and the associated "Paragraph" to which it belongs.

TODO: Some PDFs (for example, the Landeshochschulgesetz - LHG) have a table of
contents in the beginning, which lists all the paragraphs. It would be desirable
that this table of contents is ignored, and in particular not every header is
output as a proper unit of information. That is not so easy, however, since
locally a paragraph header in the TOC looks just like a paragraph header in the
actual content. One possible solution: analyze which lines (the text, excluding
the numeration) occurs multiple times in the document and if there is a section
at the beginning of the document which contains mostly lines repeated later,
consider that section the table of contents.
"""

import argparse
import sys
import subprocess
import re
import os.path
import logging


""" Global log. """
log = logging.getLogger()
log.setLevel(logging.DEBUG)
handler = logging.StreamHandler(sys.stdout)
# handler.setFormatter(logging.Formatter(
#     "%(asctime)s.%(msecs)03d %(levelname)-5s %(message)s", "%Y-%m-%d %H:%M:%S"))
log.addHandler(handler)

""" Class with various methods for extracting text from a PDF. """
class PdfToText:
    """
    Each of these methods take a filename as argument and returns the complete
    text as simple text, with the following additions:

    1. The end of the page is signalled by a \x0c =  (form feed) at the
    beginning of the line. The end of the file is signalled by \x0c\x0c as the
    last two characters. All tool provide this information reliably because PDF
    is a page-based format.

    2. The beginning of a line in bold face is signalled by a \x01 =  (start
    of heading). This is an important signal to identify headers. All tools
    provide this information with high reliability because font information is
    explicit in a PDF (pdtotext only with Claudius' extensions).

    3. The beginning a new paragraph is signalled by a completely blank line.
    Only the extended pdftotext and pdfact provide this with reasonable (but not
    perfect) reliability. The original pdftotext and pdftohtml both have a lot
    of false positives (blank lines in the middle of a paragraph) as well as
    false negatives (missing blank lines betwenn two paragraphs).
    """

    @staticmethod
    def via_pdftotext(filename):
        """
        Poppler's pdftotext with extensions by Claudius.
        """ 
        # cmd = ["/usr/local/bin/pdftotext", filename, "-"]
        cmd = ["/usr/local/bin/pdftotext", "-semantic-layout", filename, "-"]
        # cmd = ["/usr/bin/pdftotext", "-layout", filename, "-"]
        result = subprocess.run(cmd, stdout=subprocess.PIPE)
        text = result.stdout.decode("utf-8")

        # HACK: Currently a lot of things are marked [TITLE]. Also, most TOPs
        # from the Studienkommission are wrongly marked [PAGE HEADER].
        text = re.sub("\[(TITLE|PAGE-HEADER)\] ", "", text)

        # Make sure there is an empty line before each page break and reduce
        # multiple empty lines to one.
        text = re.sub("\x0c", "\n\x0c", text)
        text = re.sub("\n *(\n *)+", "\n\n", text)
        return text + "\n\x0c\x0c\n"

    @staticmethod
    def via_pdfact(filename):
        """
        pdfact is the most accurate, but 50 times slower.
        """
        cmd = ["/local/data/pdfact/bin/pdfact",
                # "--exclude-roles", "page-header,footer",
                "--with-control-characters", filename]
        result = subprocess.run(cmd, stdout=subprocess.PIPE)
        text = result.stdout.decode("utf-8")
        # HACK to get "End of document correct"
        text = re.sub("\x0c", "\x0c ", text)
        return text + "\n\x0c\x0c\n"

    @staticmethod
    def via_pdftohtml(filename):
        """
        pdftohtml provides  and  and paragraph info as fast as
        pdftotext, and with resonable quality, though not as good as pdfact. The
        information is containen in HTML tags, which we transform as indicated
        in the code below.
        """
        cmd = ["/usr/bin/pdftohtml", "-i", "-stdout", filename]
        result = subprocess.run(cmd, stdout=subprocess.PIPE)
        text = result.stdout.decode("utf-8")
        # We are only interested in the part inside <body>...</body> and without
        # the first <a name=1></a> (we want end of page, not beginning of page).
        text = re.sub("^.*<body[^>]*>.?<a name=1></a>(.*)</body>.*$",
                      "\\1", text, flags=re.S)
        # Make sure that a <br/> is followed by a newline.
        text = re.sub("<br/>\n?", "<br/>\n", text)
        # <a name=...></a> signals a new page with the given number.
        text = re.sub("^<a name=\d+></a>", "\x0c", text, flags=re.M)
        # <b>...</b> signals a line in bold face.
        text = re.sub("^(\x0c?)<b>", "\\1\x01", text, flags=re.M)
        # <br/> signals an empty line.
        text = re.sub("^\s*<br/>\s*$", "", text, flags=re.M)
        # Remove all remaining HTML tags.
        text = re.sub("</?[^>]+/?>", "", text)
        # Replace all HTML entites by space.
        text = re.sub("&#\d+;", " ", text)

        return text + "\n\x0c\x0c\n"


""" Config for parsing a particular PDF file. """
class PdfParserConfig:

    def __init__(self, filename):
        """
        Pick the config depending on whether the filename matches a particular
        regular expression.
        
        TODO: Eventually, there should be config file with a set of rules for
        each filename pattern and a set of default rules if no pattern matches.
        """

        # DEFAULT CONFIG (the more pdftotext progresses, the more should be in
        # the default config. Ideall, no special configuration for a particular
        # collection is needed at all).

        # Regular expressions to extract from the filename the title (the
        # document name displayed in the facet box for "TITLE) and the date. If
        # the match fails, the entries witll be NO_TITLE and NO_DATE,
        # respectively.
        self.title_regex = re.compile('\d\d\d\d-\d\d-\d\d\s(.*).pdf$')
        self.date_regex = re.compile('(\d\d\d\d-\d\d-\d\d)')

        # Which parser to use. The goal is to use integrate as many features
        # from pdfact into pdftotext as possible and use pdftotext for
        # everything because it is so much faster than pdfact.
        self.pdftotext = PdfToText.via_pdftotext

        # Require that H1 is in bold.
        self.h1_must_be_emphasized = True
        # Require that H1 must be new paragraph.
        self.h1_must_be_new_par = False #(self.pdftotext == PdfToText.via_pdftotext)
        # Extend H1 beyond h1_regex match if bold or until new paragraph?
        self.h1_single_line_only = False
        # Output sections without content?
        self.output_sections_without_content = False
        # Output sections without header?
        self.output_sections_without_header = False

        # Join words that were hyphenated by line breaks.
        self.join_hyphenated_words = True

        # Determine the configuration from the path to the PDF file.
        #
        # TODO: Eventually, this should be in a config file and not part of this
        # file, which contains the generic PDF to TSV conversion code.
        if re.search("/(satzungen|gesetze)/", filename):

            # H0 = Teil, Abschnitt, Unterabschnitt ... NOTE: not yet used
            self.h0_regex = re.compile('^(teil|abschnitt|unterabschnitt)',
                                       re.IGNORECASE)
            # H1 = Paragraphen, e.g. § 14 
            self.h1_regex = re.compile('^(\s*(§|Artikel)\s*(\d+).*)$')
            # H2 = Absätze, e.g. (3)
            self.h2_regex = re.compile('^\s*(\((\d+[a-z]?)\))')
            # P = H2 (no content outside Absätze).
            self.p_regex = self.h2_regex

        elif re.search("/(stuko|promo|dekanat|senat)/", filename):
            log.debug("CONFIG: \x1b[1mProtokolle\x1b[0m")
            self.pdftotext = PdfToText.via_pdftotext
            self.h1_regex = re.compile('^(\s*T[Oo][Pp]\s*:?\s*([0-9.]+).*)$')
            self.h2_regex = re.compile('^XYZ$') # Something that never matches.
            self.p_regex = self.h2_regex
            if re.search("/senat/", filename):
                self.title_regex = re.compile('/([^/]+)\.pdf$')
                self.date_regex = re.compile('(\d\d.\d\d.\d\d\d\d)$')

        elif re.search("/reading/", filename):
            self.title_regex = re.compile('/([^/]+)\.pdf$')
            self.date_regex = re.compile('^$')
            self.h1_regex = re.compile('^(\s*(\d+).*)$')
            self.h2_regex = re.compile('^(().*)') # Something that always matches.
            self.pdftotext = PdfToText.via_pdftotext

        else:
            log.error("No config found for \"" + filename + "\", you can" +
                      " add it to class PdfParserConfig in pdf_to_tsv.py")
            sys.exit(1)


""" Class for parsing a single PDF file. """
class PdfParser:

  def __init__(self, filename):
      """
      Initialize parsing of file with given name.
      """

      self.omit_text_before_first_heading = True

      # Command for text extraction (should write to STDOUT).
      self.path = "data/" + os.path.basename(filename)

      # Get config via filename (see class PdfParserConfig above).
      self.config = PdfParserConfig(filename)

      # Extract title and date from file name.
      self.title = (self.config.title_regex.search(filename)
                     or ("", "NO_TITLE"))[1]
      self.date = (self.config.date_regex.search(filename)
                    or ("", "NO_DATE"))[1]


  def numeral_to_number(self, numeral):
      """
      Normalize a numeral, as used in a section or subsection header, to the
      corresponding number. For example, "3" or "c" or "III" or "iii" should all
      become 3.
      
      TODO: For now, just assume that the numeral is already an integer.
      """
      return int(numeral)


  def output_tsv_line(self, h1_text, h2_text, content, page_number):
      """
      Output TSV line with given headers, content, and page number. If content
      consists only of whitespace, don't output anything unless
      self.output_sections_without_content
      
      NOTE: If not self.output_sections_without_content, a header that is
      followed by no content, will never appear in the TSV file. It depdends on
      the document whether this is desirable, hence the option.
      """

      # Check if section without content.
      if re.search("^\s*$", content) \
              and self.config.output_sections_without_content == False:
          log.debug("Previous section not output (no content)")
          return

      # Check if section without header.
      if h1_text == "" and self.config.output_sections_without_header == False:
          log.debug("Previous section not output (no header)")
          return

      # Single spaces and strip leading and trailing whitespace.
      h1_text = re.sub("\s+", " ", h1_text).strip()
      h2_text = re.sub("\s+", " ", h2_text).strip()

      # Normalize spacing: no tabs, single spaces, newline = three spaces.
      content = re.sub("[ \t]+", " ", content).strip()
      content = re.sub("\s*\n\s*", "   ", content)

      # Join words that were hyphenated by a line break (TODO: simplistic).
      if self.config.join_hyphenated_words:
          content = re.sub("(\w\w)- (?!und)(?!oder)([a-zäöü]\w\w)", "\\1\\2", content)

      # Remove # from title because it makes problems (TODO: does it?).
      title = re.sub("#", "", self.title)

      # URL-encode the path (TODO: not just the #).
      path = re.sub("#", "%23", self.path)

      # Output the line of the TSV file.
      fields = [path, str(page_number), title, self.date,
                  h1_text, h2_text, content]
      print("\t".join(fields))


  def parse(self, debug_text = False, pdftotext_override = None):
      """
      Parse the file and return lines for CSV file

      Column 1: heading
      Column 2: text
      """

      # Get the text from the PDF using the configured tool. Depending on the
      # tool, this will contain additional information in the form of form feed
      # (\x0c), start of heading (\x0) and empty lines.
      # 
      # If program started with --pdftotext-override, the second argument will
      # contain the name of the tool. Note that the argument to that option is,
      # for example, pdftohtml, but the function is PdfToText.via_pdftohtml.
      if (pdftotext_override == None):
          text = self.config.pdftotext(filename)
      else:
          log.debug("Override PDF extraction, using: " + pdftotext_override)
          text = eval("PdfToText.via_" + pdftotext_override + "(filename)")

      # In debug mode, just output the text.
      if debug_text:
          print("Extracted text for \"" + filename + "\"")
          print("\n" + (">" * 80) + "\n")
          print(text)
          print("\n" + (">" * 80) + "\n")

      # The current page number.
      page_number = 1
      # The last H1 header and number.
      h1_text = ""
      h1_number = 0
      # The last H2 header and number.
      h2_text = ""
      h2_number = 0
      # The content of the current section (which may be spread over several
      # lines) and the page number when it began.
      content = ""
      content_page_number = 1
      # Does next line begin new paragraph.
      start_of_new_paragraph = True

      # Iterate over all lines in the file. Note that depending on the extaction
      # tool, a single paragraph of the PDF or even a single header can be
      # spread over several lines.
      for line in text.split("\n"):

          # log.debug("LINE = \"" + line + "\"")

          # Is empty line? TODO: should this come here or after processing the
          # control characters?
          if re.match("^ *$", line):
              start_of_new_paragraph = True
              if content != "":
                  content += "\n"
              continue

          # If line starts with \x0c ( = form feed), it's the end of the page
          # and also counts as a start of a new paragraph. If it's the last
          # character, it also marks the end of the whole document.
          if line.startswith("\x0c"):
              if re.match("\x0c\x0c$", line):
                  log.debug("End of document")
                  self.output_tsv_line(
                          h1_text, h2_text, content, content_page_number)
                  break
              else:
                  page_number += 1
                  log.debug("Starting new page: " + str(page_number))
                  line = re.sub("^\x0c", "", line)
                  start_of_new_paragraph = True

          # If line starts with \x01 (after possible \x0c removed above), it is
          # in bold. Depending on config.h1_must_be_emphasized this is a
          # requiment for a h1 header.
          h1_ok = line.startswith("\x01") or not self.config.h1_must_be_emphasized
          h1_cont_ok = h1_ok and (not self.config.h1_single_line_only)
          h1_start_ok = h1_ok and (start_of_new_paragraph
                            or not self.config.h1_must_be_new_par)
          # log.debug("H1 ok: " + str(h1_ok) + ", H1 cont ok: " + str(h1_cont_ok)
          #             + ", H1 start ok: " + str(h1_start_ok))

          # Replace all sequences of whitespace by single space. Remove all
          # control characters. Note that \x00 - \x1F cannot appear as bytes in
          # a multi-byte UTF-8 character because these are all >= \x80.
          line = re.sub("\s+", " ", line)
          line = re.sub("[\x00-\x1f]", "", line)

          # If new H1 header found, output previous content and start new
          # section.
          h1_match = self.config.h1_regex.search(line)
          if h1_match:
              log.debug("\n\n\x1b[1mH1 match: " + str(h1_match.groups())
                      + "\x1b[0m" + " [start ok = " + str(h1_start_ok) + "]")
              if h1_start_ok:
                  self.output_tsv_line(
                          h1_text, h2_text, content, content_page_number)
                  h1_text = h1_match.group(1)
                  h1_number = h1_match.group(2)
                  h2_text = ""
                  h2_number = 0
                  content = ""
                  start_of_new_paragraph = False
                  content_page_number = page_number
                  continue


          # If new H2 header found, output previous content and start new
          # content.
          #
          # NOTE: We consider the H2 header already as part of the content. Is
          # that always what we want? In the current output, the H2 header than
          # appears both in the header (H1 + H2) and the content.
          h2_match = self.config.h2_regex.search(line)
          if h2_match:
              log.debug("\n\x1b[34mH2 match: " + str(h2_match.groups())
                          + "\x1b[0m")
              self.output_tsv_line(
                      h1_text, h2_text, content, content_page_number)
              h2_text = h2_match.group(1) 
              h2_number = h2_match.group(2)
              content = line
              start_of_new_paragraph = False
              content_page_number = page_number
              continue

          # If new paragraph found before H2 header found, start new content.
          #
          # NOTE: This is for the case that there is content after a H1 header
          # but before the first H2 header in that section. Not sure whether
          # this is needed, but it sounded like a good idea. This can be
          # controlled by self.p_regex
          p_match = self.config.p_regex.search(line)
          if p_match:
              log.debug("P match: " + str(p_match.groups()))
              content = line
              start_of_new_paragraph = False
              content_page_number = page_number
              continue

          # If we come here, neither H1 nor H2 nor P was detected. This is, in 
          # fact, the normal case (most lines are normal contents).
          if h1_text != "":
              # Case 1: No H2 or content seen yet.
              if h2_text == "" and content == "":
                  log.debug("H1 cont ok = " + str(h1_cont_ok))
                  # Case 1.1: Continue header.
                  if h1_cont_ok:
                      h1_text += " " + line
                      log.debug("H1 continued: " + h1_text)
                  # Case 1.2: Start contents.
                  else:
                      content = line
              # Case 2: Add line to content.
              else:
                  content += " " + line
              start_of_new_paragraph = False


class MyArgumentParser(argparse.ArgumentParser):
    """
    Override the error message so that it prints the full help text if the
    script is called without arguments or with a wrong argument.
    """

    def error(self, message):
        print("ArgumentParser: %s\n" % message)
        self.print_help()
        sys.exit(1)


""" Main function """
if __name__ == "__main__":

    # Configure command line arguments + usage info.
    parser = MyArgumentParser(
        # description="Usage: python3 make_csv.py [PDF files]",
        # epilog="..."
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument(
            "--log-level", dest="log_level", type=str,
            choices=["INFO", "DEBUG", "ERROR"], default="INFO",
            help="Log level (INFO, DEBUG, ERROR)")
    parser.add_argument(
            "--debug-text", dest="debug_text",
            action="store_true", default=False,
            help="Show only the extracted text for each PDF")
    parser.add_argument(
            "--pdftotext-override", dest="pdftotext_override", type=str,
            choices=["pdftotext", "pdftohtml", "pdfact"],
            help="Force use of praticular PDF extraction tool"
                 ", independent of config (useful for debugging)")
    parser.add_argument(
            "filenames", metavar="PDF file", type=str, nargs='+',
            help="One or more PDF files")

    # Parse the command line arguments.
    args = parser.parse_args(sys.argv[1:])
    log.setLevel(eval("logging.%s" % args.log_level))
    log.debug("Log level is \x1b[1m%s\x1b[0m" % args.log_level)
    log.debug("Filenames are: " + str(args.filenames))

    # Print headers.
    if not args.debug_text:
        headers = ["path", "page", "title", "date", "h1", "h2", "text"]
        print("\t".join(headers))
    else:
        log.setLevel(logging.DEBUG)

    # Parse all the files.
    for filename in args.filenames:
        parser = PdfParser(filename)
        parser.parse(args.debug_text, args.pdftotext_override)
