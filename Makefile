# This is an example makefile that you can use with your project.
#
# To use it, just type "make" on the command line in the same directory as 
# the Makefile. Feel free to add compile-time options here for better
# performance.
#
# The default rule to be executed is the first one (all). If you specify the 
# name of a different rule, then it will be used instead. For example, you can
# compile the sequential nbody program by typing "make seq". 
#
# To override the default number of threads (1), you can also specify this on the 
# command line as well:
#
#  $ make threads=8
# 
threads = 1

all: upc_nbody.c
	upcc -O -Wc,-O3 -fupc-threads-$(threads) -o lab3.$(threads) upc_nbody.c -lm

upc: upc_nbody.c
	upc -O3 -fupc-threads-$(threads) -o lab3.$(threads) upc_nbody.c -lm

seq: nbody.c
	clang -O3 -o nbody nbody.c -lm

clean:
	rm -f lab3 lab3.[0-9] lab3.[0-9][0-9] nbody
