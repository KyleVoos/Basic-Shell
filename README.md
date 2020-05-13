# Basic-Shell
This is a basic shell program that allows the user to run commands as if in a terminal such as
bash. After the user enters a command and any optional arguments it will find the file path
to the executable by iterating through the directories in the PATH environment variable. If the
executable exists a new process will be forked and the executable will be executed with any optional arguments
with execv(). 
## How to:
1. Make the program via ```make``` in the terminal.
2. execute esh
3. Enter commands as if it's a regular terminal

## Sample Output:
kyle@LAPTOP-T16SRG68:/mnt/c/Users/Tormentt/Documents/GitHub/Basic-Shell$ ./esh  

esh> ls  
CMakeLists.txt  Makefile  README.md  cmake-build-debug  esh  main.c  path  

esh> ls -a  
.   .git            .gitignore  CMakeLists.txt  README.md          esh     path  
..  .gitattributes  .idea       Makefile        cmake-build-debug  main.c  

esh> ls -al  
total 44  
drwxrwxrwx 1 kyle kyle  4096 May 13 12:06 .  
drwxrwxrwx 1 kyle kyle  4096 May 11 12:14 ..  
drwxrwxrwx 1 kyle kyle  4096 May 13 12:06 .git  
-rwxrwxrwx 1 kyle kyle    66 Feb 12 08:56 .gitattributes  
-rwxrwxrwx 1 kyle kyle   469 Feb 12 09:05 .gitignore  
drwxrwxrwx 1 kyle kyle  4096 May 13 12:06 .idea  
-rwxrwxrwx 1 kyle kyle   107 Jan 31 13:06 CMakeLists.txt  
-rwxrwxrwx 1 kyle kyle    40 Apr  7 15:26 Makefile  
-rwxrwxrwx 1 kyle kyle   147 Apr  7 15:26 README.md  
drwxrwxrwx 1 kyle kyle  4096 Feb 12 09:03 cmake-build-debug  
-rwxrwxrwx 1 kyle kyle 13816 May 13 12:06 esh  
-rwxrwxrwx 1 kyle kyle  8225 May 13 12:06 main.c  
-rwxrwxrwx 1 kyle kyle 13816 Feb 24 15:38 path  

esh> ls ../BackItUp  
BackItUp  CMakeLists.txt  Makefile  README.md  TEST  backup  cmake-build-debug  main.c  

esh> ls echo hello  
ls: cannot access 'echo': No such file or directory  
ls: cannot access 'hello': No such file or directory  

esh> echo hello  
hello  

esh> echo hello world  
hello world

esh> hello  
ERROR: hello not found!  

esh> exit  
kyle@LAPTOP-T16SRG68:/mnt/c/Users/Tormentt/Documents/GitHub/Basic-Shell$  
