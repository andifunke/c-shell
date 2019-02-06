# Shell in C

My first somewhat more comprehensive assignment during my CS course (09/2014).
The task was to write a shell in C that supports the following subset of commands:

`date` return current date-time

`echo` print arguments to stdout

`ls` list current working directory

`cd` naviagte the file system tree

`grep` pipe the output of any command to grep. Example:

```
/home/max/hhush $ echo this is a test | grep is
this is a test
/home/max/hhush $
```

`exit` terminate shell

##### Constraints:

C99 compatibility using only a restricted set of libraries.
System calls via `system()` were not allowed and of course flawless dynamic memory allocation was mandatory.

#### Compile code

run either

```
$ make
```

or

```
$ gcc -std=c99 hhush.c -o hhush
```

#### Run shell

```
$ make run
```

or

```
$ ./hhush
```
