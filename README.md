Seamless Shell - SLSH - /SLUSH/
--------------

Seamless Shell, known as slsh is a simple, lightweight command-line tool designed to optimize the execution of [pipelines](http://en.wikipedia.org/wiki/Pipeline_%28Unix%29) across different remote machines from which files are used in the pipeline.

When executed normally, the files required for a pipeline when on remote machines are downloaded locally for execution. So, it is easy to imagine that when large files are involved, the time taken to completely execute the pipeline can be significant. Seamless Shell, aims to minimize this execution by organizing the execution of the individual commands so that, there is the least amount of transferred bytes thereby reducing the execution times.

It is designed to work with remote directories that are mounted using SSHFS. It requires, password less, key-based login without any passphrase involved.

Seamless Shell is still a work in progress. There are many scenarios to be explored and different test cases to be passed before it is finialized as complete.

This is a project that I developed as a Master's student at UIC under the guidance of Prof. Jakob Eriksson.

Source Code
-----------

Seamless Shell is written in C and uses Flex and Bison for scanning and parsing.

Compile
----------

a. Dependencies
	You will need to install [Flex](http://flex.sourceforge.net/) and [Bison](http://www.gnu.org/software/bison/) to compile.

b. Compilation
	Running make inside the src directory will get you the executeable for slsh.