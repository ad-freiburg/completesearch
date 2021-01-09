// Copyright 2010, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Authors: Ina Baumgarten <baumgari>.

#include <stdio.h>
#include <string>
#include <fstream>
#include <vector>
#include "./XmlToJson.h"

int main(int argc, char** argv)
{
  std::string xml = "<hit score=\"2\" id=\"2493727\"><info><authors><author>Ján Manuch</author><author>Daya Ram Gaur</author></authors><title ee=\"http://www.comp.nus.edu.sg/~wongls/psZ/apbc2007/apbc226a.pdf\">Fitting Protein Chains to Cubic Lattice is NP-Complete. </title><venue url=\"db/confäcäc2007.html#ManuchG07\" conference=\"ABC\" pages=\"153-164\">APBC 2007:153-164</venue><year>2007</year><type>inproceedings</type></info><url>http://www.dblp.org/rec/bibtex/conf/abpc/ManuchG07</url></hit>";
  std::string xml2 = "<dblp:authors><dblp:author>Henry G. Dietz</dblp:author></dblp:authors><dblp:title ee=\"http://dx.doi.org/10.1109/ISPASS.2008.4510737\">Computer Aided Engineering of Cluster Computers. </dblp:title><dblp:venue url=\"db/conf/ispass/ispass2008.html#DieterD08\">ISPASS 2008:44-53</dblp:venue><dblp:year>2008</dblp:year><dblp:type>inproceedings</dblp:type>"; // NOLINT
  std::string xml3 = "<dblp:authors></dblp:authors><dblp:title ee=\"http://dx.doi.org/10.1109/ISPASS.2008.4510737\">Computer Aided Engineering of Cluster Computers. </dblp:title><dblp:venue url=\"db/conf/ispass/ispass2008.html#DieterD08\">ISPASS 2008:44-53</dblp:venue><dblp:year>2008</dblp:year><dblp:type>inproceedings</dblp:type>"; // NOLINT
  std::string xml4 = "<dblp:authors><dblp:author>Jing Kong</dblp:author><dblp:author>Christopher A. White</dblp:author><dblp:author>Anna I. Krylov</dblp:author><dblp:author>David Sherrill</dblp:author><dblp:author>Ross D. Adamson</dblp:author><dblp:author>Thomas R. Furlani</dblp:author><dblp:author>Michael S. Lee</dblp:author><dblp:author>Aaron M. Lee</dblp:author><dblp:author>Steven R. Gwaltney</dblp:author><dblp:author>Terry R. Adams</dblp:author><dblp:author>Christian Ochsenfeld</dblp:author><dblp:author>Andrew T. B. Gilbert</dblp:author><dblp:author>Gary S. Kedziora</dblp:author><dblp:author>Vitaly A. Rassolov</dblp:author><dblp:author>David R. Maurice</dblp:author><dblp:author>Nikhil Nair</dblp:author><dblp:author>Yihan Shao</dblp:author><dblp:author>Nicholas A. Besley</dblp:author><dblp:author>Paul E. Maslen</dblp:author><dblp:author>Jeremy P. Dombroski</dblp:author><dblp:author>Holger Daschel</dblp:author><dblp:author>Weimin Zhang</dblp:author><dblp:author>Prakashan P. Korambath</dblp:author><dblp:author>Jon Baker</dblp:author><dblp:author>Edward F. C. Byrd</dblp:author><dblp:author>Troy A. Van Voorhis</dblp:author><dblp:author>Manabu Oumi</dblp:author><dblp:author>So Hirata</dblp:author><dblp:author>Chao-Ping Hsu</dblp:author><dblp:author>Naoto Ishikawa</dblp:author><dblp:author>Jan Florián</dblp:author><dblp:author>Arieh Warshel</dblp:author><dblp:author>Benny G. Johnson</dblp:author><dblp:author>Peter M. W. Gill</dblp:author><dblp:author>Martin Head-Gordon</dblp:author><dblp:author>John A. Pople</dblp:author></dblp:authors><dblp:title ee=\"http://dx.doi.org/10.1002/1096-987X(200012)21:16<1532::AID-JCC10>3.0.CO;2-W\">Q-Chem 2.0: a high-performance ab initio electronic structure program package. </dblp:title><dblp:venue url=\"db/journals/jcc/jcc21.html#KongWKSAFLLGAOGKRMNSBMDDZKBBVOHHIFWJGHP00\">Journal of Computational Chemistry (JCC) 21(16):1532-1548 (2000)</dblp:venue><dblp:year>2000</dblp:year><dblp:type>article</dblp:type>"; //NOLINT
  std::string xml5 = "<dblp:authors><dblp:author>Baoyindureng Wu</dblp:author><dblp:author>Li Zhang 0009</dblp:author><dblp:author>Zhao Zhang</dblp:author></dblp:authors><dblp:title ee=\"http://dx.doi.org/10.1016/j.disc.2005.04.002\">The transformation graph <i>G<sup>xyz</sup></i> when <i>xyz</i>=-++. </dblp:title><dblp:venue test=\"mal sehen ob Leerzeichen stören\" url=\"db/journals/dm/dm296.html #WuZZ05\">Discrete Mathematics (DM) 296(2-3):263-270 (2005)</dblp:venue><dblp:year>2005</dblp:year><dblp:type>article</dblp:type>"; // NOLINT
  std::string xml6 = "<dblp:authors><dblp:author>Dieter Baum</dblp:author></dblp:authors><dblp:title ee=\"\">Räumliche Markov-additive \\\"Prozesse und\\\" Bedienstationen ohne Warteraum </dblp:title><dblp:venue url=\"\" journal=\" Universität \\\"Trier\\\", Mathematik/Informatik, Forschungsbericht (TRIER)\" year=\"2003\" volume=\"03-01\">Universität Trier, Mathematik/Informatik, Forschungsbericht (TRIER) 03-01 (2003)</dblp:venue><dblp:year>2003</dblp:year><dblp:type>article</dblp:type>"; // NOLINT
  std::string xml7 = "<info><show field=\"dblp-record\"><article key=\"journals/et/ThibeaultHH12\" mdate=\"2012-04-23\"><![CDATA[<author>Claude Thibeault</author><author>Yassine Hariri</author><author>C. Hobeika</author><title>Tester Memory Requirements and Test Application Time Reduction for Delay Faults with Digital Captureless Test Sensors.</title><pages>229-242</pages><year>2012</year><volume>28</volume><journal>J. Electronic Testing</journal><number>2</number><ee>http://dx.doi.org/10.1007ß0836-011-5271-2</ee><url>db/journals/et/et28.html#ThibeaultHH12</url>]]></article></show></info>";
  std::string xml8 = "<dblp:authors><dblp:author>Henry G. Dietz</dblp:author><dblp:author>Agent X</dblp:author><dblp:author>Hans Friedrich</dblp:author><dblp:author>HanneMarie dubida</dblp:author></dblp:authors><dblp:title ee=\"http://dx.doi.org/10.1109/ISPASS.2008.4510737\">Computer Aided Engineering of \"Cluster Computers.\" </dblp:title><dblp:venue url=\"db/conf/ispass/ispass2008.html#DieterD08\">ISPASS 2008:44-53</dblp:venue><dblp:year>2008</dblp:year><dblp:type>inproceedings</dblp:type>"; // NOLINT

  vector<string> multiple;
  multiple.push_back("\"dblp:author\":");
  XmlToJson xj, xj2(multiple);
  printf("\n\n%s\n", xj.xmlToJson(xml).c_str());
  printf("\n\n%s\n", xj.xmlToJson(xml2).c_str());
  printf("\n\n%s\n", xj.xmlToJson(xml3).c_str());
  printf("\n\n%s\n", xj.xmlToJson(xml4).c_str());
  printf("\n\n%s\n", xj.xmlToJson(xml6).c_str());
  printf("\n\n%s\n", xj.xmlToJson(xml7).c_str());
  printf("\n\n%s\n", xj.xmlToJson(xml8).c_str());
  printf("\n\n%s\n", xj2.xmlToJson(xml5).c_str());

/*
  std::ifstream ifs("longXml.xml");
  std::string content((std::istreambuf_iterator<char>(ifs)),
                      (std::istreambuf_iterator<char>()));
  for (size_t i = 0; i < 1000; i++) xj.xmlToJson(content);
*/
}
