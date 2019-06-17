/*
 *
 * Implements the central function 
 * computeExcerptPositions()
 * of the excerpt generator as desribed on
 *
 * http://search.mpi-sb.mpg.de/wiki/CompleteSearch/ExcerptGenerator/CentralFunction
 *
 * On the command prompt, type
 * $ try-CentralFunction
 * for a usage overview.
 *
 * Additionally takes the radius into consideration.
 * Additionally takes maxMatchingSegmentsOutput into consideration.
*/

#include <queue>
#include <vector>
#include <iostream>
#include <list>
#include <utility>
#include <string>

using std::vector;
using std::priority_queue;
using std::cout;
using std::endl;
using std::list;
using std::pair;
using std::string;


typedef unsigned int Position; // a position in the document


// a helper struct for the priority queue, which contains pairs of
// positions (=priorities) and position lists

struct ListPrio {
public:
  Position _pos;         // position in the document (=priority)
  unsigned int _poslist; // number of the position list from where the
			 // position comes from

  ListPrio(Position l, Position p) : _poslist(l), _pos(p) {}

  // higher positions have lower priority in the PQ
  bool operator < (const ListPrio& lp) const {  return _pos > lp._pos; }
};


// the following static variables should later become members of the
// class ExcerptGenerator or parameters of the central function

static bool doHighlighting = true;

static vector< pair<string,string> > highlightingTags;

static string partsSeparator("...");

static const int NOPOSLIST = -1;

static inline void output(vector< pair<Position, int> >& o,
	    Position pos, int poslist = NOPOSLIST) {
  o.push_back( pair<Position, int>(pos, poslist) );
}


// The central function.
// PRECONDITION: there is a sentinel value for the rightmost position at the
// end of the segment bounds lists (i.e., largest position value + 1)

void computeExcerptPositions(const vector<Position>& segmentBounds,  
		     const vector< vector<Position> >& positionLists,
		     unsigned int radius,
		     int maxMatchingSegmentsOutput,     
		     vector< pair<Position, int> >& result)
{
  // fill priority queue with positions from position lists 

  // initially insert only 1 position from each list into the PQ.
  // later insert the next position from list i immediately after a
  // position from list i has been taken out of the queue. thus, the
  // PQ will always contain at most positionsLists.size() elements,
  // making inserts and lookups very fast.

  priority_queue< ListPrio > PQ;

  // insert the 1st position from each position list into the PQ
  // remember iterator to next unprocessed position in each list
  // remember list ends for fast comparisons later

  size_t numPosLists = positionLists.size();
  vector<Position>::const_iterator poslistIt[numPosLists];
  vector<Position>::const_iterator poslistEndIt[numPosLists];

  for(int i = 0; i < numPosLists; i++) {
    poslistIt[i]    = positionLists[i].begin();
    poslistEndIt[i] = positionLists[i].end();
    if( poslistIt[i] !=  poslistEndIt[i] ) { // iff poslist not empty
      PQ.push( ListPrio(i, *poslistIt[i]) );
      poslistIt[i]++;
    }
  }


  // INVARIANT: [left, right) is the current segment
  Position left, right;

  // INVARIANT: docpos is the next unread position in the document
  Position docpos; 

  // the "window" is the sequence of segments according to the radius
  // it always contains the current segment as its middle segment
  // its leftmost position is windowLeft
  // its rightmost position is segmentRightCount segments ahead to the right
  // in the first iterations, the window must be filled with
  // radius segments to the left (window filling phase)
  Position windowLeft;
  int segmentRightCount = 0;
  int windowFillPhaseCount = radius;

  // step through segment bounds with iterators
  // initialize left, right, docpos, and windowLeft
  vector<Position>::const_iterator leftIt, rightIt, windowLeftIt;

  leftIt = windowLeftIt = segmentBounds.begin(); 
  docpos = left  = *leftIt;
  windowLeft = *windowLeftIt;

  rightIt = leftIt;
  rightIt++;
  right  = *rightIt;

  // has the current segment been shifted in the last iteration?
  bool curSegmentShifted = true;

  // how many matching segments have we discovered so far?
  unsigned int matchingSegmentsCount = 0;

  // main loop: extract the next position from the PQ

  while( !PQ.empty() ) {

    // get next matching position and its position list
    ListPrio lp = PQ.top();
    PQ.pop();
    Position pos = lp._pos;
    unsigned int poslist = lp._poslist;

    // insert next position from poslist into PQ (if there still is one)
    if (poslistIt[poslist] != poslistEndIt[poslist] ) {
      PQ.push( ListPrio(poslist, *poslistIt[poslist]) );
      poslistIt[poslist]++;
    }
    
    // on first reading, skip the while loop; come back later and understand 

    // while pos not in the current segment
    while(! (left <= pos && pos < right) ) {

      // if there are segments to the right to be output while the
      // window is shifted to the right
      if(segmentRightCount > 0) {
	segmentRightCount--;
	// output the remainder of the current segment
	while(docpos < right) {
	  output(result, docpos);
	  docpos++;
	}
      }
      
      // shift current segment 1 to the right
      left = *++leftIt;
      right = *++rightIt; // always works by PRECONDITION
      curSegmentShifted = true;

      // cout << "\nstepping to [" << left << "," << right << "]" << endl;

      // shift windowLeft if not in the window filling phase
      if(windowFillPhaseCount > 0)
	windowFillPhaseCount--;
      else
	windowLeft = *++windowLeftIt;
    }

    // have we found another matching segment?
    // do not output more than maxMatchingSegmentsOutput matching segments
    if(curSegmentShifted)
        matchingSegmentsCount++;
    if(maxMatchingSegmentsOutput >= 0
       && matchingSegmentsCount > maxMatchingSegmentsOutput)
      break;

    // now pos is in the current segment. output everything in the
    // window from docpos to pos that has not been output yet
    
    if(docpos < windowLeft)
      docpos = windowLeft;

    while(docpos < pos) {
      output(result, docpos);
      docpos++;
    }

    // output highlighted pos
    output(result, docpos, poslist);
    docpos++;


    // if the while loop has shifted the currrent segment, compute
    // the new right end of the window

    if(curSegmentShifted) {
      segmentRightCount = 1 + radius;
      //cout << "\nSet segmentRightCount to " << segmentRightCount<< endl;
      curSegmentShifted = false;
    }

  } // end of main loop: while PQ not empty

  // output remaining segments of the window
  // TODO if there are matching positions in the remaining segments, output
  // them highlighted
  vector<Position>::const_iterator rightEndIt = segmentBounds.end();

  while(segmentRightCount > 0) {
    segmentRightCount--;
    while(docpos < right) {
      output(result, docpos);
      docpos++;
    }
    if(++rightIt == rightEndIt)
      break;
    else
      right = *rightIt;
  }

}



void computePCounters(const vector<Position>& segmentBounds,  
		      const vector< vector<Position> >& positionLists,
		      vector<int>& p)
{
  // fill priority queue with positions from position lists 

  // initially insert only 1 position from each list into the PQ.
  // later insert the next position from list i immediately after a
  // position from list i has been taken out of the queue. thus, the
  // PQ will always contain at most positionsLists.size() elements,
  // making inserts and lookups very fast.

  priority_queue< ListPrio > PQ;

  // insert the 1st position from each position list into the PQ
  // remember iterator to next unprocessed position in each list
  // remember list ends for fast comparisons later

  size_t numPosLists = positionLists.size();
  vector<Position>::const_iterator poslistIt[numPosLists];
  vector<Position>::const_iterator poslistEndIt[numPosLists];

  for(int i = 0; i < numPosLists; i++) {
    poslistIt[i]    = positionLists[i].begin();
    poslistEndIt[i] = positionLists[i].end();
    if( poslistIt[i] !=  poslistEndIt[i] ) { // iff poslist not empty
      PQ.push( ListPrio(i, *poslistIt[i]) );
      poslistIt[i]++;
    }
  }


  // number of last segment that a position from poslist has been seen
  int lastSegment[numPosLists];

  for(int i = 0; i < numPosLists; i++) 
    lastSegment[i] = -1;
  

  // INVARIANT: [left, right) is the current segment
  Position left, right;


  // step through segment bounds with iterators
  // initialize left, right docpos, and windowLeft
  vector<Position>::const_iterator leftIt, rightIt;

  leftIt = segmentBounds.begin(); 
  left  = *leftIt;

  rightIt = leftIt;
  rightIt++;
  right  = *rightIt;

  // number of current segment
  unsigned int curSegment = 0;

  // main loop: extract the next position from the PQ

  while( !PQ.empty() ) {

    // get next matching position and its position list
    ListPrio lp = PQ.top();
    PQ.pop();
    Position pos = lp._pos;
    unsigned int poslist = lp._poslist;

    // insert next position from poslist into PQ (if there still is one)
    if (poslistIt[poslist] != poslistEndIt[poslist] ) {
      PQ.push( ListPrio(poslist, *poslistIt[poslist]) );
      poslistIt[poslist]++;
    }
    
    // on first reading, skip the while loop; come back later and understand 

    // while pos not in the current segment
    while(! (left <= pos && pos < right) ) {

      // shift current segment 1 to the right
      left = *++leftIt;
      right = *++rightIt; // always works by PRECONDITION
      curSegment++;

    }


    // now pos is in the current segment. increment p counter for
    // poslist if this is the first occurence of a position from
    // poslist in this segment
    if( lastSegment[poslist] != curSegment ) {
      p[poslist]++;
      lastSegment[poslist] = curSegment;
    }

  } // end of main loop: while PQ not empty

}


// compute the a[i] from the c[i] w.r.t m = maxMatchingSegmentsOutput
// abstraction: divide m units to n buckets; bucket i must
// contain <= c[i] units 
// a[i] is the amount of units filled into bucket i
// see the wiki for a description

void computePortioning(const vector<int>& c_in, 
		       vector<int>& a_out, 
		       unsigned int m) 
{
  int n = c_in.size(); 

  // sort the c[i] in increasing order, retaining original position
  // info (remeber permutation made while sorting)
  vector< pair<int, int> > c(n);
  for(int i = 0; i < n; i++)
    c[i] = pair<int,int>(c_in[i], i);
  // pairs are compared lexicographically, so there is no need to
  // define a cmp fcuntion of its own:
  stable_sort(c.begin(), c.end()); 

  vector<int> a(n);
  for(int j = 0; j < n; j++)
    a[j] = 0;
  

  int imin = 0;

  while( m != 0  ) {

    while (imin < n && a[imin] == c[imin].first)
      imin++;

    int nbar = n - imin;

    if( nbar == 0 ) { // all buckets filled, some units left unportioned
      break;
    }

    int min = c[imin].first - a[imin];

    if( nbar * min <= m ) {
      for(int j = imin; j < n; j++)
	a[j] += min;
      m = m - nbar * min;
    }
    else {
      int quot = m / nbar;
      for(int j = imin; j < n; j++)
	a[j] += quot;
      for(int j = imin; j < imin + (m % nbar); j++)
	a[j] = a[j] + 1;
      m = 0;
    }

  }

  // undo permutation made while sorting
  for(int j = 0; j < n; j++)
    a_out[c[j].second] = a[j];

}



static void computeExcerptText(const vector< pair<Position, int> >& res,
			       const vector<Position>& segmentBounds) 
{
  for (unsigned int i = 0; i < res.size(); ++i)
    cout << "(" << res[i].first << "," << res[i].second << ") " << std::flush;
  cout << endl;

  size_t n = res.size();
  size_t numTags = highlightingTags.size();
  Position pos, oldpos;
  int poslist;

  vector<Position>::const_iterator boundsIt = segmentBounds.begin();
  Position boundsPos = *boundsIt++;
  bool newPart = true;

  for(int i = 0; i < n; i++) {
    oldpos = pos;
    pos  = res[i].first;
    poslist   = res[i].second;

    if(i != 0 && oldpos != pos - 1) {
      cout << partsSeparator << " ";
      newPart = true;
    }


    if(pos >= boundsPos) {
      if(!newPart)
	cout << "| ";
      while(pos >= boundsPos)
	boundsPos = *boundsIt++;
      newPart = false;
    }

    if(poslist == NOPOSLIST || !doHighlighting)
      cout << pos << " ";
    else {
      cout << highlightingTags[poslist % numTags].first  << pos 
	   << highlightingTags[poslist % numTags].second << " ";
    }
  }
}



// helper function for debugging purposes

void printPositionLists( vector< vector<Position> >& positionLists ) {
  size_t numPosLists = positionLists.size();
  vector<Position>::const_iterator poslistIt;
  vector<Position>::const_iterator poslistEndIt;

  for(int i = 0; i < numPosLists; i++) {
    poslistIt    = positionLists[i].begin();
    poslistEndIt = positionLists[i].end();

    while(poslistIt != poslistEndIt) {
      cout << highlightingTags[i].first 
	   << *poslistIt++ 
	   << highlightingTags[i].second
	   << " ";
    }

  }
  
  cout << endl;
}



int main(int argc, char* argv[])
{
  const int numFixArgs = 5;
  const int numPoslists = 3;

  if(argc < numFixArgs) {
    cout << "Usage: " 
	 << argv[0] 
	 << " <boundStep> <maxBound> <radius> <maxMatchingSegmentsOutput> <matchPos>|<highlightingTag> <matchPos>|<highlightingTag>...\n";
    cout << "\nA highlighting tag must start with a non-digit char and starts a new position list\n";
    cout << "\nExample:\n"
	 << argv[0]
	 << " 5 100 0 -1 a 3 7 9 b 5 11 c 15\n";
    exit(1);
  }

  unsigned int boundStep = atoi(argv[1]);
  unsigned int maxBound  = atoi(argv[2]);
  unsigned int radius    = atoi(argv[3]);
  int maxMatchingSegmentsOutput  = atoi(argv[4]);

  // fill list of segment bounds
  vector<Position> segmentBounds;
  for(int i = 0; i <= maxBound; i += boundStep )
    segmentBounds.push_back(i);

  // fill position lists and highlighting tags
  // step through remaining args

  vector< vector<Position> > positionLists; 
  vector<Position>  emptyList; 

  for(int i = numFixArgs + 1; i <= argc; i++) {
    if(!isdigit(*argv[i-1])) { // first char of arg is not digit?
      // create new highlighting tag and new position list  
      highlightingTags.push_back( 
              pair<string,string>(string(argv[i-1]), string(argv[i-1])) );
      positionLists.push_back( emptyList );
    }
    else {
      if(highlightingTags.empty()) {
	// default highlighting tag is '*'
	highlightingTags.push_back( pair<string,string>("*", "*") );
	positionLists.push_back( emptyList );
      }
      positionLists.back().push_back( atoi(argv[i-1]) );
    }
  }

  // output positionlists
  // printPositionLists( positionLists );

    
  vector<int> p(numPoslists);
  computePCounters(segmentBounds, positionLists, p);
  for(int i = 0; i < numPoslists; i++)
    cout << "p[" << highlightingTags[i].first << "]=" << p[i] << "  ";  
  cout << endl;

  if(maxMatchingSegmentsOutput > 0) {
    vector<int> a(numPoslists);
    computePortioning(p, a, maxMatchingSegmentsOutput);
    for(int i = 0; i < numPoslists; i++)
      cout << "a[" << highlightingTags[i].first << "]=" << a[i] << "  ";  
    cout << endl;
  }
  

  vector< pair<Position, int> > result;

  // call the central function
  computeExcerptPositions(segmentBounds, positionLists, 
			  radius, maxMatchingSegmentsOutput, result);

  computeExcerptText( result, segmentBounds );


  cout << endl;
}


