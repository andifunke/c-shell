# Shell in C

My first somewhat more comprehensive assignment during my CS course (*09/2014*).
The task was to write a shell in C that supports the following subset of commands:


`date` : return current date-time. Example:

```
/home/max/hhush $ date
Wed Jan 01 12:34:00 2014
/home/max/hhush $
/home/max/hhush $ date
Wed Jan 01 12:34:32 2014
```


`echo` : print arguments to stdout. Example:

```
/home/max/hhush $ echo hello world!
hello world!
/home/max/hhush $ echo

/home/max/hhush $
```


`ls` : list current working directory. Example:

```
/home/max/hhush $ ls
test
test.c
/home/max/hhush $
```


`cd` : navigate the file system tree. Example:

```
/home/max/hhush $ cd /home/max
/home/max $ cd Documents
/home/max/Documents $ cd ..
/home/max $
```


`grep` : search for a string pattern within a given file. Example:

```
/home/max/hhush $ grep include test.c
#include <stdio.h>
#include <stdlib.h\>
/home/max/hhush $
```


`pipe` : pipe the output of any command to grep. Example:

```
/home/max/hhush $ ls | grep test
test
test.c
/home/max/hhush $ echo this is a test | grep is
this is a test
/home/max/hhush $
```


`history [n]` : print the shell history (including the history command). Can be restricted to the last ***n*** commands. The history will be persisted to the file system when closing the shell. However, the history can not be navigated.

`history -c` : clear the entire history. Example:

```
/home/max/hhush $ sl
command not found
/home/max/hhush $ date
Wed Jan 01 12:45:48 2014
/home/max/hhush $ history
0 sl
1 date
2 history
/home/max/hhush $ history 3
1 date
2 history
3 history 3
/home/max/hhush $ history -c
/home/max/hhush $ history
0 history
```


`exit` : terminate the shell

```
/home/max/hhush $ exit
```

##### Constraints:

C99 compatibility was required using only a restricted set of libraries.
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
