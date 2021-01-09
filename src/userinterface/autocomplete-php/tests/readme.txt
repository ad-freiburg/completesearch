Tests use an own PHP deployment and a completion server running on its own collection (because most of the tests are collection depending).
The used PHP application is located in geek:/var/www/search.mpi-sb.mpg.de/markus.tests.
Before testing copy the latest version of the PHP files from the CVS repository to /var/www/search.mpi-sb.mpg.de/markus.tests.
Check whether the comletion server for this application is still running on port 8095 (if not start it by using /var/www/search.mpi-sb.mpg.de/markus.tests/completion_server/start).
