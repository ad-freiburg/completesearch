# CompleteSearch Docker Setup

CompleteSearch is a fast and interactive search engine for what we call *context-sensitive prefix search* on a given collection of documents.
The techniques and features behind CompleteSearch are described in full detail in [this paper](https://pdfs.semanticscholar.org/ba12/7643fadeed05eed91b0714a5f85444e8df71.pdf).
A fully-functional demo using [DBLP](https://dblp.uni-trier.de/) as document collection can be found [here](http://dblp.informatik.uni-freiburg.de).

This repository provides a very easy to use installation setup for CompleteSearch using Docker, including information how to build an index, how to start a backend server and how to setup a simple frontend.

## Quickstart

We now show how this Docker setup can be used to install an instance of CompleteSearch using DBLP as the document collection.
The information given are not specific to DBLP and can be also used, in principle, to install other CompleteSearch instances based on other document collections (e.g., Wikipedia or the [Vorlesungsverzeichnis of the University of Freiburg](http://vvz.tf.uni-freiburg.de/)).  

### Checkout
Checkout the repository

    git clone https://ad-git.informatik.uni-freiburg.de/ad/completesearch-docker
    cd completesearch-docker                                                      

### Building an index

Building an index requires the following two arguments:
* *<DB\>* :
The name of the database.
This name must correspond to the name of a folder in the `./databases` directory that provides all source code files specific to this database (e.g., the source code files of the parser to use for parsing the database files in *<DATA_DIR>*).
* *<DATA_DIR\>* :
The path to the directory that provides the database file(s) to parse.
The basename of the database file(s) must be *<DB\>*, and the file type must match the type expected by the used parser.
For example, if the value of *<DB\>* is "dblp", and the expected type is xml, the directory must contain a file `dblp.xml`.
All index- and intermediate files will be stored in this directory, so make sure that the container has write access to the directory.

#### docker-compose

To build an index by using **docker-compose**, open docker-compose.yml and change the values of *<DB\>* and *<DATA_DIR\>* in the *index-builder* service:
```yml
index-builder:
  build:
    dockerfile: ./Dockerfile.index-builder
  image: completesearch:index-builder
  environment:
    - DB=<DB>
  volumes:
    - <DATA_DIR>:/data:rw
```
Afterwards, type

    docker-compose up -d index-builder

#### docker

To build an index by using **docker**, type:

    # Build a container.
    docker build -f Dockerfile.index-builder -t completesearch:index-builder .
    # Run the container.
    docker run -e DB=<DB> -v <DATA_DIR>:/data -it completesearch:index-builder

### Starting the backend

Starting the backend requires the following three arguments:
* *<DB\>* :
The name of the database.
This name must correspond to the database name used in the *Building an index* step.
* *<PORT\>* :
The port on which the backend server should be available on the host.
* *<DATA_DIR\>* :
The path to the directory that provides the index files created in the *Building an index* step above.

#### docker-compose
To start the backend by using **docker-compose**, open `docker-compose.yml` and change the values of *<DB\>*, *<PORT\>* and *<DATA_DIR\>* in the *backend* service:

```yml
backend:
  build:
    dockerfile: ./Dockerfile.backend
  image: completesearch:backend
  environment:
    - DB=<DB>
  ports:
    - 0.0.0.0:<PORT>:8181
  volumes:
    - <DATA_DIR>:/data:rw
```
Afterwards, type:

    docker-compose up -d backend

#### docker
To start the backend by using **docker**, type:

    # Build a container.
    docker build -f Dockerfile.backend -t completesearch:backend .
    # Run the container.
    docker run -e DB=<DB> -p 0.0.0.0:<PORT>:8181 -v <DATA_DIR>:/data -it completesearch:backend

### Setting up the frontend

Setting up the frontend for searching DBLP requires the following four arguments:
* *<BACKEND_HOST\>* :
The host name of the backend (using docker-compose, this is equal to the name of the backend service).
* *<BACKEND_PORT\>* :
The port of the backend **within** the container.
* *<FRONTEND_PORT\>* :
The port of the frontend under which the frontend is available **outside** the container.
* *<SERVER_NAME\>* :
TODO

#### docker-compose
To start the frontend by using **docker-compose**, open `docker-compose.yml` and change the values of *<BACKEND_HOST\>*, *<BACKEND_PORT\>*, *<FRONTEND_PORT\>* and *<SERVER_NAME\>* in the *frontend-dblp* service:

```yml
frontend-dblp:
  build:
    context: .
    dockerfile: ./Dockerfile.frontend-dblp
    args:
      - BACKEND_HOST=<BACKEND_HOST>
      - BACKEND_PORT=<BACKEND_PORT>
      - SERVER_NAME=<SERVER_NAME>
  image: completesearch:frontend-dblp
  ports:
    - 0.0.0.0:<FRONTEND_PORT>:80
```
Afterwards, type:

    docker-compose up -d frontend-dblp

Using **docker**:

To start the frontend by using docker, type:

    # Build a container.
    docker build -f Dockerfile.frontend-dblp --build-arg BACKEND_HOST=<BACKEND_HOST> --build-arg BACKEND_PORT=<BACKEND_PORT> --build-arg SERVER_NAME=<SERVER_NAME> -t completesearch:frontend-dblp .
    # Run the container.
    docker run -p 0.0.0.0:<FRONTEND_PORT>:80 -it completesearch:frontend-dblp
