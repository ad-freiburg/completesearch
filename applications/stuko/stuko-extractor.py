"""
Copyright 2020, University of Freiburg,
Chair of Algorithms and Data Structures
Author: Hannah Bast <bast@cs.uni-freiburg.de>
"""

import docx
import re
import sys
import os
import inspect

VERBOSE = False


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
    if len(sys.argv) == 2 and sys.argv[1] == "V":
        VERBOSE = True

    if VERBOSE:
        print()
        print("! BEWARE: You are running with VERBOSE %d" % VERBOSE)
        print()


    column_headers = ("titel", "stuko", "year", "top", "text", "typ", "url")
    print("\t".join(["%s"] * len(column_headers)) % column_headers)

    base_path = "applications/stuko/documents"
    for filename in sorted(os.listdir(base_path)):
        # Only files ending with Protokoll.pdf
        if not filename.endswith("Protokoll.docx"):
            continue

        # We assume that the file name contains Studienkommission #<number> am <date>
        match = re.search("((Studienkommission.*\d+)\s*am.*(\d\d\d\d))", filename)
        if match == None or len(match.groups()) != 3:
            print("! Could not parse filename:" + filename, file=sys.stderr)
            print(match.groups())
            continue
        title = match.group(1)
        stuko = match.group(2)
        year = match.group(3)

        # No need to use SectionCounter as in enquete-extractor.py . Instead,
        # just remember the last header (= paragraph with style "Heading ?")
        current_heading = ""
        current_level = 0

        # Parse the document using the python-docx library
        doc = docx.Document(base_path + "/" + filename)
        if VERBOSE:
            pass
            # print("! NUMBERING details: %s" % inspect.getmembers(
            #     doc.part.numbering_part.numbering_definitions._numbering))

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
            # CASE 2: New heading -> read all 
            elif style_name.startswith("Heading ") and \
                    "Tagesordnung" not in text \
                    and not re.match("^\s*$", text):
                try:
                    heading_level = int(re.findall("\d+", style_name)[0])
                except (TypeError, ValueError, IndexError) as e:
                    print("WARNING: Style name '%s' starts with 'Heading' or 'Überschrift', "
                          "but no number follows" % style_name, file=sys.stderr)
                    heading_level = 0
                current_heading = re.sub("^\s+", "", text)
            # CASE 2: Normal paragraph -> output as one text record with title =
            # all the relevant headings (at the current level or above).
            elif (style_name.startswith("Normal") or \
                  style_name.startswith("BESCHLUSS")) and \
                  not re.match("^\s*$", text) and \
                  current_heading != "":
                text_type = "Text" if style_name != "BESCHLUSS" else "Beschluss"
                # Print  columns:
                columns = (title, stuko, year, current_heading, text, text_type,
                           re.sub(".docx", ".pdf", filename))
                print("\t".join(["%s"] * len(columns)) % columns)
            else:
                pass
                # print()
                # print("Non-heading paragraph with style: %s" % style)
                # print()
