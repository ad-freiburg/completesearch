# Options to startCompletionServer 

`startCompletionServer` is the executable to start the CompleteSearch query
engine. It comes with a very large number of options, which are explained in
this document. For example usage, see the example applications in the subfolder
`application`

**-p, --port**
	Listen to requests under the specified port. Default: none.

**-K, --kill-running-server-false**
	In case, there is already a server running under the port specified with
	-p or --port, do not kill it. The PID of the process is read from a
	file, see --pid-file. Default: kill running server.

**-k, --kill**
	Takes a port as argument. If a server is already running under that
	port, kill it. The PID of the process is read from a file, see
	--pid-file. Use case: restarting a server under a different port without
	time lag.

**-P, --pid-file**
	The format string of the file conaiting the PID of the process. Default:
	~/.completesearch_%s_%s.pid

**-l, --log-file**
	Write the log to the the given file. Default: no log.

**-c, --cache-size-excerpts**
	Cache for the documents databases from which excerpts are generated. In
	Bytes, or with suffix K or M or G. Default: 16M.
	
**-h, --max-size-history**
	Cache for query reults. In Bytes, or with suffix K or M or G. Default:
        32M.

**-q, --max-queries-history**
	Maximum number of query results in the cache. Default: 200.	

**-A, --keep-in-history-queries**
	File with queries (one per line), for which results should not be
	removed from the cache, once they are in there. Default: no such
	queries.
	
**-I, --warm-history-queries**
	File with queries (one per line) launched upon start-up. Often used in
	conjunction with --keep-in-history-queries for the same file. Default:
	no such queries.

**-V, --no-statistics**
	Do not compute query statistics. Default: compute (recommended).

**-t, --index-type**
	HYB (for fast search *and* autocompletion) or INV (standard inverted
	index). Default: HYB.
	
**-F, --no-double-fork**
	Normally, the process forks twice. The grandchild runs the actual server
	and the child watches it and restarts it if something goes wrong. The
	parent exits to the shell. With this option, there is only the child and
	the grandchild.

**-Z, --zero-fork**
	Run in the foreground and write log to the console.

**-r, --auto-restart**
	If something happens to the server, restart it. Unless the signal was
	SIGUSR1 (intentional termination) or the server was run in the
	foreground with --zero-fork. Default: do not restart automatically.

**-m, --multi-threaded**
	One thread per request. This has bugs when concurrent queries access the
	cache. Default: single threaded.
	
**-v, --verbosity**
	Log level (0 = ZERO, 1 = NORMAL, 2 = HIGH, 3 = HIGHER, 4 = HIGHEST).
	Default: 1.

**-d, --how-to-rank-docs**
	Default ranking of the hits in query result. This can also be controlled
	with the URL parameter rd, see the documentation there.

**-w, --how-to-rank-words**
	Default ranking the completions in the query results. This can also be
	controlled with the URL parameter rw, see the documentation there.

**-s, --score-aggregations**
	Four-letter string that specifies how the scores for hits and
	completions are aggregated. Each letter is either S (Sum) or M
	(Maximum). The first letter stands for the aggregation of scores for one
	hit from different occurrences of the same word. The second letter stand
	for the aggreation of scores for one hit for the scores from different
	words (think prefix search). The third letter stands for the aggregation
	of scores for one word (think completion) from occurrences in the same
	document. The fourth letter stands for the aggreation of scores for one
	word from the scores from different documents. Default: SSSS.
	
**-X, --exe-command**
	For URL parameter exe=<cmd> make a system call <cmd>. Warning: only do
	this, when you control the URLs, otherwise this can pose a security
	riks. Default: exe=<cmd> in the URL has no effect.

**-D, --document-root**
	When URL path does not start with ?, consider as a file name or path and
	server from the directory specified as the argument of this option.
	Default: return error messagen when URL path does not start with ?.

**-O, --enable-cors**
	Add header "Access-Control-Allow-Origin: *\r\n" to query results, so
	that results can be processed by a script from a different domain or
	port than the server. Default: no such header.
	
**-L, --locale**
	In the code, call setLocale with the argument of this option. This
	affects sort order and must be in sync with the sort order of the index.
	Default: do not call setLocale in the code.

**-S, --enable-synonym-search**
	Read file `<name>.synonym-groups`. The file format is as follows:
	1. One synonym group per line.
	2. Words are separated by a comma.
	3. Any whitespace is ignored.
	4. Lines starting with # are treated as comment lines and are ignored.
	5. Words with a trailing '*' will be treated such that a search for that
	   word will *not* return the other words in the synonym group
	Default: no synonym search.

**-Y, --enable-fuzzy-search**
**-W, --fuzzy-normalize-words**
**-G, --use-generalized-edit-distance-slow**
**-B, --use-baseline-fuzzysearch**
	See example applications. The baseline fuzzy search is good enough.
	Default: no fuzzy search.

**-N, --normalize-words**
	Normalize query words, for example ä -> ae. This should be consistent
	with how the index was built. Default: do not normalize.

**-M, --maps-directory**
	Directory with the files for the maps for normalization, one for utf-8
	and one for iso-8859-1. Default: codebase/utility, which contains the
	files utf8.map and iso8859-1.map
	
**-Q, --show-query-result**
	Show (a part of the) query result in the log. Default: no not show.

**-C, --cleanup-query-before-processing**
	Fix some obvious mistakes in the query. See function cleanupQuery in
	server/CompletionServer.cpp. Better done by the UI. Default: do not
	clean up.

**-U, --use-suffix-for-exact-query**
	Deprecated (the code that used this is commented out). Default: option
	not activated.

**-H, --disable-cdata-tags**
	In the XML result, do not use `<![CDATA[...]]>` for each completion to
	save some space. Risks malformed XML. Default: enabled.

**-o, --query-timeout**
	Query timeout in milliseconds. At certain checkpoints, abort the
	processing of the current query if this time is exceeded. Default: 5000
	milliseconds.
	
**-b, --word-part-separator-backend**
	The separator used for special words, e.g. :facet:author:Hannah_Bast or
	info:facet:author . Default: ! (but : looks nicer, but should not be
	used if this is a regular character in index words).

**-f, --word-part-separator-frontend**
	Default: Same as that of --word-part-separator-backend (leave it like
	that).
	
**-0, --read-custom-scores**
	Read scores from a file `<name>.custom-scores`. Default: use thes scores
	from the index.

**-i, --info-delimiter**
