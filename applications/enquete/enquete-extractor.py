"""
Copyright 2020, University of Freiburg,
Chair of Algorithms and Data Structures
Author: Hannah Bast <bast@cs.uni-freiburg.de>
"""

import docx
import re
import sys
import inspect

VERBOSE = False

filenames = {
         "Mantelbericht"           : "enquete-mantel.docx",
         "PG1: KI und Wirtschaft" : "enquete-pg1.docx",
         "PG2: KI und Staat"      : "enquete-pg2.docx",
         "PG3: KI und Gesundheit" : "enquete-pg3.docx",
         "PG4: KI und Arbeit"     : "enquete-pg4.docx",
         "PG5: KI und Mobilität"  : "enquete-pg5.docx",
         "PG6: KI und Medien"     : "enquete-pg6.docx"
         }

# filenames = ["enquete-pg5.docx"]


class SectionCounter:
    """
    Class for maintaining section counts in a document and for each paragraph,
    all headings relevant at that point.

    NOTE: The title is at level 0, the heading levels start at 1.
    """

    def __init__(self, counter = [0] * 5):
        """ Initialize counter with given value.
        >>> SectionCounter().counter
        [0, 0, 0, 0, 0]
        """
        self.counter = counter
        self.headings = [""] * len(counter)

    def section_count_string(self, level):
        """ Get string representation of counter at given level. Ignore level 0,
        which is the title.
        >>> sc = SectionCounter([1, 3, 1, 5, 2])
        >>> sc.section_count_string(2)
        '3.1'
        >>> sc.section_count_string(4)
        '3.1.5.2'
        """
        return ".".join([str(_) for _ in self.counter[1:(level + 1)]])

    def new_heading(self, level, heading = ""):
        """ Increase count at given level and reset all counts at higher levels
        + remember the heading. If level is too high, ignore the heading.
        >>> sc = SectionCounter([1, 3, 1, 5, 2])
        >>> sc.new_heading(2, "New Section at Level 2")
        >>> sc.counter
        [1, 3, 2, 0, 0]
        >>> sc.headings[2]
        '3.2 New Section at Level 2'
        """
        if level < len(self.counter):
          self.counter[level] += 1
          for i in range(level + 1, len(self.counter)):
              self.counter[i] = 0
          self.headings[level] = heading if level == 0 else \
              self.section_count_string(level) + " " + heading

    def headings_list(self, level):
        """ Get all headings relevant at current level.
        >>> sc = SectionCounter([0, 0, 0, 0])
        >>> sc.new_heading(0, "Title")
        >>> sc.new_heading(1, "Section 1")
        >>> sc.new_heading(2, "Section 1.1")
        >>> sc.new_heading(3, "Section 1.1.1")
        >>> sc.new_heading(3, "Section 1.1.2")
        >>> sc.headings_list(3)
        ['Title', 'Section 1', 'Section 1.1', 'Section 1.1.2']
        """
        return self.headings[:(level + 1)]

if __name__ == "__main__":
    if VERBOSE:
        print()
        print("! BEWARE: You are running with VERBOSE %d" % VERBOSE)
        print()

    print("%s\t%s\t%s\t%s" % ("berichtsteil", "abschnitt", "text", "dateiname"))

    for title, filename in filenames.items():
        # Treat each file as one document with one title. Ignore the actual
        # title of the document (the actual titles are not 100% consistent and
        # the document "Mantelbericht" has not title at all).
        section_counter = SectionCounter([0, 0, 0, 0, 0])
        section_counter.new_heading(0, title)
        current_level = 0

        # Parse the document using the python-docx library
        doc = docx.Document("documents/" + filename)
        if VERBOSE:
            print("! NUMBERING details: %s" % inspect.getmembers(

                doc.part.numbering_part.numbering_definitions._numbering))

        # Iterate over all paragraphs
        for paragraph in doc.paragraphs:
            text = re.sub("\s+", " ", paragraph.text)
            style_name = paragraph.style.name
            if VERBOSE:
                print("! NEW PARAGRAPH: style = %s, text = %s" % (style_name,
                          text if len(text) < 100 else text[:97] + "..."))
                # print("! Format details: %s" % paragraph._p.pPr.numPr)

            # CASE 1: New title -> ignore it, we take our own titles (see above)
            if style_name.startswith("Title"):
                pass
                # current_level = 0
                # section_counter.new_heading(current_level, text)
            # CASE 2: New heading -> remember it and update section counter.
            elif style_name.startswith("Heading"):
                try:
                    current_level = int(re.findall("\d+", style_name)[0])
                except (TypeError, ValueError) as e:
                    print("WARNING: Style name '%s' starts with 'Heading', "
                          "but no number follows" % style_name, file=sys.stderr)
                    current_level = 0
                if current_level > 0:
                    section_counter.new_heading(current_level, text)
            # CASE 2: Normal paragraph -> output as one text record with title =
            # all the relevant headings (at the current level or above).
            elif (style_name.startswith("Normal") or \
                 style_name.startswith("Text") or \
                 style_name.startswith("Title")) and \
                 not re.match("^\s*$", text):
                # Print four columns:
                # COLUMN 1: The title
                # COLUMN 2: The current headings concatenated (with <BR>)
                # COLUMN 3: The text
                # COLUMN 4: The filename (not really needed)
                title = section_counter.headings_list(0)[0]
                headings = section_counter.headings_list(current_level)[1:] \
                        if current_level > 0 else ["UNKNOWN SECTION"]
                print("%s\t%s\t%s\t%s" %
                        (title, " &mdash; ".join(headings), text, filename))
            else:
                pass
                # print()
                # print("Non-heading paragraph with style: %s" % style)
                # print()
