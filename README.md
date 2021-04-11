# P2
A basic UNIX shell that handles a variety of commands that can be found on a regular UNIX shell.
## How It Works
I created a **getword()** that parses each letter from a file looking for white spaces or EOF. It returns the number of characters per line before hitting EOF. It also separates metacharacters so that they can be used in the **parse()** and **main()** later. **Parse()** sets up metacharacter flags so that these metacharacters can do the desired operations. **Main()** executes forking, multiple vertical pipelining, I/O redirection, and error checking.
## The Metacharacters and Commands
`<` Infile redirection. Stores the word afterwards as the input file.<br />
`<<` Hereis redirection. Stores the word afterwards as the hereis delimiter.<br />
`>` Outfile redirection. Stores the word afterwards as the output file.<br />
`$` Makes the number of character negative and also prints the current environment of the word attached to the $.<br />
`|` Allows for multiple processes to run through vertical pipelining.<br />
`~` Prints the absolute path of the directory it is in front of.<br />
`cd` Allows the user to change the current directory and prompts the directory name the user is in.<br />
`environ` Allows the user to change the environment of a directory if desired, and allows the user to print out the environemnt of the directory.<br />

