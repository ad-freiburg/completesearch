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
environment variable, which contains the basename of all your files. Since this
basename is used a lot in the commands below, it is handy to have a variable for
that. 

1. A directory `applications/${DB}` containing your configuration of the
   indexing process. As a minimum, this includes a `Makefile` (which configures
   the parsing and the backend), and a file called `config.php` (which
   configures the front end).  More about that in the sections about indexing
   and running.

2. A directory `data/${DB}` containing your input data and where the various
   index files (from the index build process, so that your search is super fast)
   are written.  Your input can be given as an XML file or a CSV file. Using a
   CSV file is much simpler and almost as powerful.

Under codebase/example you find configuration and input files for both the XML
and the CSV way. The following explanations are for the CSV way.

## 2. Building an index

To build an index, you can use the following handy command line. Note that the
`pall` in the end is the name of a Makefile target. It stands for "precompute
all".

        export DB=example && docker build -f Dockerfile.backend -t completesearch.${DB} . && docker run -it --rm -p 5001:8080 -e DB=${DB} -v $(pwd)/applications/${DB}:/configuration -v $(pwd)/data/${DB}:/data --name completesarch.${DB} completesearch.${DB} -c "make DB=/data/${DB} pall"

TODO: Explain the input format and the various configuration options in the
Makefile. For now, just look at `codebase/example/csv/example.csv` and
`codebase/example/csv/Makefile`. A lot of it is self-explanatory and you can
simply try to copy this for a new application and play around with some
parameters.

### docker-compose

To start the backend by using **docker-compose**, open `docker-compose.yml` and change the value of DB.

TODO: The CMD is still missing

```yml
backend:
  build:
    context: .
    dockerfile: ./Dockerfile.backend
  image: completesearch:backend
  environment:
    - DB=example
  volumes:
    - $(pwd)/applications/${DB}:/configuration:rw
    - $(pwd)/data/{DB}:/data:rw
  restart: unless-stopped
```

Afterwards, type:

    docker-compose up -d backend

## 3. Starting the backend

To start the backend, you can use almost the same command line as above. Except
that you also have to specify a port (the port on which the backend will listen
to requests) and `start` instead of `pall` in the end.

        export DB=example && docker build -f Dockerfile.backend -t completesearch.${DB} . && docker run -it --rm -p 5001:8080 -e DB=${DB} -v $(pwd)/applications/${DB}:/configuration -v $(pwd)/data/${DB}:/data --name completesarch.${DB} completesearch.${DB} -c "make DB=/data/${DB} start"

You can also build the index and run the server in one command if you like. For
that, you simply specify both targets `pall` and `start`. This is only
recommended for small collections though or if you are sure that your indexing
(which can take a while) runs through without error.

        export DB=example && docker build -f Dockerfile.backend -t completesearch.${DB} . && docker run -it --rm -p 5001:8080 -e DB=${DB} -v $(pwd)/applications/${DB}:/configuration -v $(pwd)/data/${DB}:/data --name completesarch.${DB} completesearch.${DB} -c "make DB=/data/${DB} pall start"

### docker-compose

To start the backend by using **docker-compose**, open `docker-compose.yml` and change the value of DB.

TODO: The CMD is still missing

```yml
backend:
  build:
    context: .
    dockerfile: ./Dockerfile.backend
  image: completesearch:backend
  environment:
    - DB=example
  volumes:
    - $(pwd)/applications/${DB}:/configuration:rw
    - $(pwd)/data/{DB}:/data:rw
  restart: unless-stopped
```

Afterwards, type:

    docker-compose up -d backend

## 4. Setup the frontend

You can run the frontend with the following command line. If everything works,
the frontend will be available under `<hostname>:<port>`, where `<hostname>` is
the name of the machine on which you are running and `<port>` is the port
specified in the command line.

       export DB=example && export PORT=5000 && docker build -f Dockerfile.frontend -t completesearch-ui . && docker run -it --rm -v $(pwd)/applications/${DB}:/configuration -v $(pwd)/data/${DB}:/log -p 0.0.0.0:${PORT}:80 --name completesearch-ui completesearch-ui

### docker-compose

To start the frontend by using **docker-compose**, open `docker-compose.yml` and change the 

```yml
frontend:
  build:
    context: .
    dockerfile: ./Dockerfile.frontend
  image: completesearch:frontend
  environment:
    - PORT=5000
  ports:
    - 0.0.0.0:${PORT}:80
```

Afterwards, type:

    docker-compose up -d frontend

## 5. (Optional) Setup a subdomain

To show off your CompleteSearch instance to your friends, you may want it to run
under a fancy URL. For that that you need something like the following in your 
`apache.conf`:, where `vulcano` is an example hostname.

```xml
<VirtualHost *:80>
  ServerName example.cs.uni-freiburg.de
  ServerAlias dblp example.cs.uni-freiburg.de
  ServerAdmin webmaster@localhost

  ProxyPreserveHost On
  ProxyRequests Off

  ProxyPass / http://vulcano:5001/
  ProxyPassReverse / http://vulcano:5001>/

  ...
</VirtualHost>
```
