# CompleteSearch Docker Setup

CompleteSearch is a fast and interactive search engine for what we call *context-sensitive prefix search* on a given collection of documents.
The techniques and features behind CompleteSearch are described in full detail in [this paper](https://pdfs.semanticscholar.org/ba12/7643fadeed05eed91b0714a5f85444e8df71.pdf).
A fully-functional demo using [DBLP](https://dblp.uni-trier.de/) as document collection can be found [here](http://www.dblp.org).

This repository provides an easy-to-use installation setup for CompleteSearch using Docker, including information how to build an index, how to start a backend server and how to setup a simple frontend.

## 1. Checkout

Checkout the repository

    git clone https://github.com/ad-freiburg/completesearch
    cd completesearch

The following sections assume that you are in this directory.

## 2. Relevant files

For a particular collection, you need two directories. Let `DB` be an
environment variable, which contains the basename of all your files. This 
basename is used a lot in the commands below, so it is handy to have a
variable for it. 

1. A directory `applications/${DB}`, which contains your configuration of the
   indexing process. As a minimum, this includes a `Makefile` (which configures
   the parsing and the backend), and a file called `config.php` (which
   configures the front end).  More about that in the sections about indexing
   and running.

2. A directory `data/${DB}`, which contains your input data and where the various
   index files (from the index build process, so that your search is super fast)
   are written.  Your input can be given as an XML file or a CSV file. Using a
   CSV file is much simpler and almost as powerful.

Under codebase/example you find configuration and input files for both the XML
and the CSV way. The following explanations are for the CSV way.

## 2. Building an index

To build an index, you can use the following handy command line. Note that the
`pall` in the end is the name of a Makefile target. It stands for "precompute
all".

        export DB=example && docker build -f Dockerfile.backend -t completesearch.${DB} . && docker run -it --rm -e DB=${DB} -v $(pwd)/applications/${DB}:/configuration -v $(pwd)/data/${DB}:/data --name completesarch.${DB} completesearch.${DB} -c "make DB=/data/${DB} pall"

TODO: Explain the input format and the various configuration options in the
Makefile. For now, just look at `codebase/example/csv/example.csv` and
`codebase/example/csv/Makefile`. A lot of it is self-explanatory and you can
simply try to copy this for a new application and play around with some
parameters.

## 3. Starting the backend

To start the backend, you can use almost the same command line as above. Except
that you also have to specify a port (the port on which the backend will listen
to requests) and the make target `start` instead of `pall` in the end.

        export DB=example && export PORT=5001 & docker build -f Dockerfile.backend -t completesearch.${DB} . && docker run -it --rm -p ${PORT}:8080 -e DB=${DB} -v $(pwd)/applications/${DB}:/configuration -v $(pwd)/data/${DB}:/data --name completesarch.${DB} completesearch.${DB} -c "make DB=/data/${DB} start"

You can also build the index and run the server in one command if you like. For
that, you simply specify both targets `pall` and `start`. This is only
recommended for small collections though or if you are sure that your indexing
(which can take a while) runs through without error.

        export DB=example && export PORT=5001 & docker build -f Dockerfile.backend -t completesearch.${DB} . && docker run -it --rm -p ${PORT}:8080 -e DB=${DB} -v $(pwd)/applications/${DB}:/configuration -v $(pwd)/data/${DB}:/data --name completesarch.${DB} completesearch.${DB} -c "make DB=/data/${DB} pall start"

## 4. Setup the frontend

You can run the frontend with the following command line. If everything works,
the frontend will be available under `<hostname>:<port>`, where `<hostname>` is
the name of the machine on which you are running and `<port>` is the value you
assign to the environment variable `PORT`.

       export DB=example && export PORT=5000 && docker build -f Dockerfile.frontend --build-arg DB=${DB} -t completesearch-ui . && docker run -it --rm -v $(pwd)/applications/${DB}:/configuration -v $(pwd)/data/${DB}:/log -p 0.0.0.0:${PORT}:80 --name completesearch-ui completesearch-ui

## 5. (Optional) Setup a subdomain

To show off your CompleteSearch instance to your friends, you may want it to run
under a fancy URL. Let us assume you have an Apache webserver running on one of
your machine. Then you need the following section in your `apache.conf` or in file
included by `apache.conf`. You have to replace `servername` by the [fully qualified domain
name (FQDN)](https://en.wikipedia.org/wiki/Fully_qualified_domain_name) of the
machine on which your Apache webserver is running. You have to replace `hostname`
by the FQDN of the machine on which the CompleteSearch frontend is running. This
can be the same machine as `servername`, but does not have to be.

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
