# CompleteSearch

CompleteSearch is a fast and interactive search engine for *context-sensitive prefix search* on a given collection of documents.
It does not only provide search results, like a regular search engine,
but also completions for the last (maybe only partially typed) query word that lead to a hit.
This can be used to provide very efficient support for a variety of features:
query autocompletion, faceted search, synonym search, error-tolerant search, semantic search.
A list of publications on the techniques behind CompleteSearch and the many applications is provided at the end of this page.

For a demo on various datasets, just checkout this repository and follow the instructions below.
With a single command line, you get a working demo on one from a selection of datasets
(each of the size of a few million documents, so not paticularly large, but also not small).
CompleteSearch easily scales to collections with tens or even hundreds of millions of documents,
without losing its interactivity.

## 1. Checkout

Checkout the repository

    git clone https://github.com/ad-freiburg/completesearch
    cd completesearch

## 2. Demos

Just run the following command line, where for the value of DB you can choose
between a number of demo datasets (one for every subdirectory of `applications`).
A generic UI will then be available under the specified `PORT`.
Note that the CompleteSearch backend simultaneously provides an API for answering search and completion queries,
and servers as a simple HTTP server at the same time.

        export DB=movies && PORT=1622 && docker build -t completesearch . && docker run -it --rm -e DB=${DB} -p ${PORT}:8080 -v $(pwd)/applications:/applications -v $(pwd)/data/:/data -v $(pwd)/ui:/ui --name completesearch.${DB} completesearch -c "make DATA_DIR=/data/${DB} DB=${DB} pall start"

This command line will build the index and start the server, all in one go.
If you have already built the index once, you can omit the *pall* (which stand for *precompute all*).

## 3. Relevant files

Read this section if you want to understand a little deeper of what's going on with the fancy command line above.
The command line first builds a docker image from the code in this repository.
So far so good.
It then runs a docker container, which mounts three volumes, which we briefly explain next:

**applications** This folder contains the configuration for each application.
Each configuration just contains two files.
A `Makefile` that specifies how to build the index (this is highly customizable, see below).
And a `config.js` for customizing the generic UI.

**data** This folder contains the CSV file with the original data (one record per line, in columns) and the index files.
They all have a common prefix. See below for more information on the index.

**ui** This folder contains the code for the generic UI.
If you just want to use CompleteSearch as backend and build your own UI,
you don't have to mount this volume.
It's nice, however, to always have a working UI available for testing, without any extra work.

## 4. The CompleteSearch index

Like all search engines, CompleteSearch builds an index with the help of which it can then answer queries efficiently.
It is not an ordinary inverted index, but something more fancy: a half-inverted index or *hybird (HYB)* index.
You don't have to understand this if you just want to use CompleteSearch.
But if you are interested, you can learn more about it in the publications below.

To build the index, CompleteSearch requires two input files, one with suffix `.words` and one with suffix `.docs`.
The first contains the contents of your documents split into words.
The second contains the data that you want to display as search engine hits.
The two are usually related, but not exactly the same.
The format is very simple and is described by example [here](https://ad-wiki.informatik.uni-freiburg.de/completesearch/QuickIntro).

If you have special wishes, you can build these two input files yourself, from whatever your data is.
Then you have full control over what CompleteSearch will and can do for you.
However, in most applications, you can use our *generic CSV parser*.
It takes a CSV file (one record per line, with a fixed number of columns per line) as input,
and from that produce the *.words* and the *.docs* file.

The CSV parse is very powerful and highly customizable.
You can see how it is used in the *Makefile* of the various example applications
(in the subdirectories of the directory `applications`).
A subset of the options is described in more detail [here](https://ad-wiki.informatik.uni-freiburg.de/completesearch/CsvParser).
For a complete list, look at the [code that parses the options](https://github.com/ad-freiburg/completesearch/blob/master/codebase/parser/CsvParserOptions.cpp).

## 4. The CompleteSearch engine

The binary to start the CompleteSearch engine is called `startCompletionServer`.
It is very powerful and has a lot of options.
For some example uses, you can have a look at the `Makefile` in the director
`applications` and at the included `Makefile` of one of the example applications.
A detailed documentation of all the options can be found in a `README.md` in the directory `src`.

Once started, you can either ask queries using our generic and customizable UI (see above).
Or you can ask the backend directly, via the HTTP API provided by `startCompletionServer`.
The API is very simple and described [at the end of this page](https://ad-wiki.informatik.uni-freiburg.de/completesearch/QuickIntro).
Play around with it for one the example applications to get a feeling for what it does.
You can also look at the (rather simple) JavaScript code of the generic UI
to get a feeling for how it works and what it can be used for.

## 5. (Optional) Setup a subdomain

To show off your CompleteSearch instance to your friends, you may want it to run
under a fancy URL, and not `http://my.weird.hostname.somewhere:76154`.
Let us assume you have an Apache webserver running on your machine.
Then you can add the following section in your `apache.conf` or in a separte
config file included by `apache.conf`.
You have to replace `servername` by the
[fully qualified domain name (FQDN)](https://en.wikipedia.org/wiki/Fully_qualified_domain_name) of the
machine on which your Apache webserver is running.
You have to replace `hostname` by the FQDN of the machine on which the CompleteSearch frontend is running.
This can be the same machine as `servername`, but does not have to be.

```xml
<VirtualHost *:80>
  ServerName example.cs.uni-freiburg.de
  ServerAlias dblp example.cs.uni-freiburg.de
  ServerAdmin webmaster@localhost

  ProxyPreserveHost On
  ProxyRequests Off

  ProxyPass / http://<hostname>:5000/
  ProxyPassReverse / http://<hostname>:5000>/

  ...
</VirtualHost>
```

## 6. Publications

Here are some of the publications explaining the techniques behind CompleteSearch and what it can be used for.
This work was done at the [Max-Planck-Institute for Informatics](https://www.mpi-inf.mpg.de/departments/algorithms-complexity).
It's already a while ago, but turns out that the features and the efficiency
provided by CompleteSearch are still very much state of the art.

[Type Less, Find More: Fast Autocompletion with a Succinct Index](https://www.researchgate.net/publication/47841931_Type_Less_Find_More_Fast_Autocompletion_Search_with_a_Succinct_Index) @ SIGIR 2006

[The CompleteSearch Engine: Interactive, Efficient, and Towards IR&DB Integration](http://cidrdb.org/cidr2007/papers/cidr07p09.pdf) @ CIDR 2007

[ESTER: efficient search on text, entities, and relations](http://researchgate.net/publication/47842303_ESTER_efficient_search_on_Text_Entities_and_Relations) @ SIGIR 2007

[Efficient interactive query expansion with complete search](https://dl.acm.org/doi/10.1145/1321440.1321560) @ CIKM 2007

[Output-Sensitive Autocompletion Search](https://link.springer.com/article/10.1007/s10791-008-9048-x) @ Information Retrieval 2008

[Semantic Full-Text Search with ESTER: Scalable, Easy, Fast](https://www.suchanek.name/work/publications/icdm2008.pdf) @ ICDM 2008 
