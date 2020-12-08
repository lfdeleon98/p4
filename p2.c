/*Lauren DeLeon
John Carroll
CS570
Extra Credit: 11/30/2020
Regular: 12/05/2020
*/


/*
The purpose of this program is to build a simple UNIX Shell.
*/
#include "p2.h"
// Global Declarations
char char_array[MAXSTORAGE];      //Array of the characters entered (to stdin) by user
char *newargv[MAXITEM];           //Pointer to the start of each word from char_array
int pipe_pos[11];		          // Index of the address of the word after a pipe (|)  
char *infile, *outfile, *till_str, *env_str;           //points to infile in buffer and points to outfile in buffer
int num_char = 0, pipe_index = 0; //num_char is returned from getword() and pipe_index represent the right side of the pipe
//FLAGS
int in_flag = 0, out_flag = 0, amp_flag = 0, pipe_flag = 0, bg_flag = 0, till_flag = 0, cd_flag = 0, special_cd_flag = 0, hereis_flag = 0, error_flag = 0;
char *delim_name;
main() {
    pid_t pid, childpid, gchildpid, kidpid, greatgchildpid;         //These are the parent, child, and grandchild pids when we use fork()
    int num_words = 0, r = 0, w = 0, offset = 0;                    //This is the number of words per line returned from parse
    int infiledes = NULL;                           //This is the infile descriptor
	int outfiledes = NULL;                          //This is the outfile descriptor
    int filedes[20];                     //This holds the file descriptor stdin and stdout of each pipe (thats why *2)
    int close_flags = O_CREAT | O_RDWR | O_EXCL;    //These flags are for creating, reading and writing, and error checking when we open the outfile descriptor 
    int close_modes = S_IRUSR | S_IWUSR;            //These modes are for user write and user read permissions for the outfile descriptor
    char* delim_line;
    size_t buf = 0;
    FILE *delim_file;
    char current_dir[1024];
    setpgid(0, 0);
    
    (void) signal(SIGTERM, sighandler);
    
    /*
        We want to loop endlessly until EOF or -255 is returned from getword()
    */

    for (;;) {
        //issue PROMPT
        if (cd_flag == 1) { //this is to show the current directory name
            (void)getcwd(current_dir, 1024);
            printf("%s", basename(current_dir));
        } else if (special_cd_flag == 1) { //this will show if the directory is /
            printf("/");
        }
        
        printf(":570: ");
        num_words = parse();
        if (num_char == -255) { //terminate if EOF is reached
            break; //exit for loop
        }
        
        if (num_words == 0) { //reprompt if 0 words
            if (in_flag != 0 || out_flag != 0 || pipe_flag != 0 || hereis_flag != 0) { //check if only meta characters were signaled and newargv is NULL
                fprintf(stderr, "Error! Invalid null command.\n");
                in_flag = 0, out_flag = 0, pipe_flag = 0, hereis_flag = 0; //reset pointers and flags
                infile = NULL, outfile = NULL;
            }
            continue; //reprompt
        }
        if (error_flag > 0) { // catches the errors in parse
            continue;
        }
       /* 
        this part of code is for the << command. When there are 2 flags for the hereis_flag, the code will go into this. First we check if there is no delimiter, then we signal
        an error. Then we check if there is a < command that was signaled before. We cannot have < and << in the same line of code becasue it will cause an error of ambiguous 
        redirection, so therefore, we will prompt an error if we encounter an in_flag = 2. Next we will use our own temp infile that we named HEREISDELIM_FILE in which we will
        set to the delim_file. We will continue to read in input and store it in the HEREISDELIM_FILE until we reach the delimiter. Because we created our own temp infile, we
        will set HEREISDELIM_FILE equal to infile and set the in_flag to 2 to signal that we are using these things.
        */
        if (hereis_flag != 0) {
            if (delim_name == NULL) {
                fprintf(stderr, "Error! Missing hereis terminator.\n");
                continue;
            }
            if (in_flag == 2) {
                fprintf(stderr, "Error! Ambiguous input redirect.\n");
                continue;
            }
            strcat(delim_name, "\n"); //this is saved to be used in main
            delim_file = fopen(HEREISDELIM_FILE, "w+");
            while (getline(&delim_line, &buf, stdin) >= 0) { //this will allow us to parse our file to find our delimiter
                if (strcmp(delim_line, delim_name) == 0) {
                    break; //if we found the delimiter, we want to break out of the while loop
                }
                fprintf(delim_file, "%s", delim_line); //this allows us to print into the
                
            }
            free(delim_line); //free the space we allocated earlier
            delim_line = NULL;
            fclose(delim_file); //close the file

            infile = HEREISDELIM_FILE; //set our infile to the unique one we have
            in_flag = 2; //this will let us know that the input is being used
            hereis_flag = 0; //this resets the flag
        }
        
        if (newargv[0] == NULL) { //check if there is nothing in newargv at index 0
            fprintf(stderr,"Error! Invalid null command.\n");
            continue;
        }
        //error checking in pipe
        if (pipe_flag > 10) {
            fprintf(stderr, "Error! Too many pipes.\n");
            pipe_flag = 0, in_flag = 0, out_flag = 0; //reset pointers and flags
            infile = NULL, outfile = NULL;
            continue;
        } else if (pipe_flag != 0) { //looks for a null command after a pipe so like "| "
            for (w = 0; w < pipe_flag; w++) {
                if (newargv[pipe_pos[w]] == NULL){
                    fprintf(stderr, "Error! Missing pipe command.\n");
                    continue;
                }
            }
        }
        if (in_flag > 2 || out_flag > 2)  { //check if too many < or >
            fprintf(stderr, "Error! Ambiguous redirection.\n");
            in_flag = 0, out_flag = 0; //reset pointers and flags
            infile = NULL, outfile = NULL;
            continue;
        }

        if (in_flag != 0) { //if in_flag is signaled
            if(infile == NULL) { //if in_flag was 1 but there is nothing infile
			    fprintf(stderr,"Error! Missing name for redirection.\n");
                continue;
		    } else if((infiledes = open(infile, O_RDONLY)) == -1) { //failed to open the infile
                fprintf(stderr,"Error! Failed to open file %s.\n", infile);
                continue;
            }
        }

        if (out_flag != 0) { //if out_flag is signaled
            if(outfile == NULL) { //if out_flag was 1 but there is nothing outfile
			    fprintf(stderr,"Error! Missing name for redirection.\n");
                continue;
            }
            if((outfiledes = open(outfile, close_flags, close_modes)) == -1) { //failed to open the outfile
                fprintf(stderr,"Error! File %s already exists.\n", outfile);
                continue;
            }
        }
        /*
        The environ command is a command that either changes the environment of one directory to another, or prints out the directory. It is a command that takes in 1 or 2
        arguments, and will prompt the correct corresponding errors.
        */
        if (strcmp(newargv[0], "environ") == 0) {
            if (num_words == 1) {
                fprintf(stderr,"Error! environ needs arguments.\n");
                continue; 
            } else if (num_words > 3) {
                fprintf(stderr,"Error! Too many arguments for environ.\n"); 
                continue; 
            } else if (num_words == 3) {
                if ((setenv(newargv[1], newargv[2], 1)) != 0) {
                    fprintf(stderr,"Error! Cannot set environ to %s.\n", newargv[2]); 
                    continue;
                }
            } else if (num_words == 2) { //show the environment if there is only 2 words on the line
                if (getenv(newargv[1]) != NULL) {
                    printf("%s\n",getenv(newargv[1]));
                }
            } 
            continue;
        }
        /*
        cd is a command that will print the current directory that the user is in or it will change the change the current directory that the user is in
        */
        if (strcmp(newargv[0], "cd") == 0) {
            if (num_words > 2) { //if there are more than 2 words in this line of code, cause and error
              fprintf(stderr,"Error! Too many arguments for cd.\n"); 
            } else if (num_words == 1) { //if it is just cd and nothing else 
                if (chdir(getenv("HOME")) != 0) { //if it can't change directory to home
                        fprintf(stderr,"Error! Could not find home directory.\n");
                } else if (strcmp(getenv("HOME"), "/") == 0) { //this will show the root directory
                    special_cd_flag = 1;
                    cd_flag = 0;
                } else {
                    cd_flag = 1; //this is the flag to show the current directory that the user is in
                    special_cd_flag = 0;
                }
            } else if (num_words == 2) {
                if (chdir(newargv[1]) != 0){ //change current directory to the desired one (at newargv[1]) else throw an error
                    fprintf(stderr,"Error! No such file or directory named %s.\n", newargv[1]); 
                } else {
                    special_cd_flag = 0; //this resets the variable for the root directory
                    cd_flag = 1;
                    
                }
            }
            continue; //reprompt if one of these errors is true
        }
        /*
        This bit of code is for multiple vertical piping, meaning that a process can fork to create a child process and that child can fork itself to create
        a grandchild process, and so on, and so forth. The logic we have here is that the youngest grandchild perfroms the task at newargv, then the child before
        that does the process at the next pipe_index (or the command after the pipe meta character). It is able to find these indexes where we have the command
        after the pipe becasue of how we set up the pipe_flag in our parse. We also make sure that we are opening and closing the right file descriptors based on
        the formula that I have set up: stdin is at every even index of our filedes array, and every stdout is at every odd index in that array.
        */
        if (pipe_flag > 0){
            fflush(stdout);
            fflush(stderr);
            if ((childpid = fork()) == 0){ //forks the child
                CHK(pipe(filedes)); //pipe the filedescriptor
                fflush(stdout);//must always flush before we fork a child process
                fflush(stderr);
                if ((gchildpid = fork()) == 0){//fork grandchild
                    for (r = 1; r <= pipe_flag; r++){//this will help us forking more children and getting their process done
                        CHK(pipe(filedes + (2 * r)));
                        if (r < pipe_flag){
                            if ((greatgchildpid = fork()) == 0){
                                continue; //continue the loop by forking;
                            }else {
                                CHK(dup2(filedes[(2 * r) - 1], STDOUT_FILENO)); //replace greatgrandchild's stdin to grandchild's stdout
                                CHK(dup2(filedes[2 * r], STDIN_FILENO));//replace stdin of grandchild's to child's stdout
                                for (w = 0; w < (2 * r) + 2; w++) { //closes the pipes
                                    CHK(close(filedes[w])); 
                                }
                                CHK(offset = (pipe_flag - 1) - r);
                                CHK(execvp(newargv[pipe_pos[offset]], newargv+pipe_pos[offset]));
                                exit(0); //close the grandchild
                            }
                        } else{
                            if (in_flag == 2){ 
                                CHK(dup2(infiledes, STDIN_FILENO));
                                CHK(close(infiledes));
                            }
                            CHK(dup2(filedes[2 * r - 1], STDOUT_FILENO));
                            for (w = 0; w < (2 * r + 2); w++) { 
                                CHK(close(filedes[w])); 
                            }
                            CHK(execvp(newargv[0], newargv));
                            }
                        } //end of g for loop
                } else{
                    if (out_flag == 2){
                        CHK(dup2(outfiledes, STDOUT_FILENO));
                        CHK(close(outfiledes));
                    }
                    CHK(dup2(filedes[0], STDIN_FILENO));
                    CHK(close(filedes[0]));
                    CHK(close(filedes[1]));
                    offset = pipe_flag - 1;
                    CHK(execvp(newargv[pipe_pos[offset]], newargv + pipe_pos[offset]));
                    exit(0);
                }
            } //end of successfuly child fork
            for (;;) {
                pid = wait(NULL); // the parent process waits for the child process to end
                if (pid == childpid) {
                break;
                }
            }
        }
        /*
        our fork command functions similarly as pipe with the dup2 and close for the file descriptors. However, we must make sure that our fork does not function if there is a pipe_flag because they will execute
        the same command. Fork executes commands. It looks at what is at newargv[0] becasue that is where the actual command is stored at. It then reads in the other words in newargv[n] to execute the command.
        */
        fflush(stdout);
		fflush(stderr);
        if ((kidpid = fork()) == -1) {
            fprintf(stderr,"Error! Cannot fork\n");
            exit(3);
        
        } else if (kidpid == 0) {
        
            if (pipe_flag != 0) { //want to do this if there is no pipe_flag signaled, or else will have double output or errors
                exit(0);
            }
            if(in_flag == 0 && bg_flag == 1){ //this does the background job and will prints out the child pid and does the child process in the background
                int dev_null;
                if((dev_null = open("/dev/null", O_RDONLY)) == -1){ // allows you to do things while the child process is running
                    fprintf(stderr, "Error! Failed to open /dev/null file.\n");
                    exit(4);
                }
                dup2(dev_null, STDIN_FILENO);
                close(dev_null);
            }
            if (in_flag == 2) {
                dup2(infiledes, STDIN_FILENO);
                if (close(infiledes) == -1) {
                    fprintf(stderr, "Error! Unable to close file descriptor.\n");
                    exit(5);
                } 
            }
            if (out_flag == 2) {
                dup2(outfiledes, STDOUT_FILENO);
                if (close(outfiledes) == -1) {
                    fprintf(stderr,"Error! Unable to close file descriptor.\n");
                    exit(8);
                }
            }
            if (execvp(newargv[0], newargv) == -1) {
                fprintf(stderr,"Error! Cannot execute %s command.\n", newargv[0]);
                exit(9);
            }
        } else if (bg_flag != 0) { //if there is no child process, print the parent pid
                printf("%s [%d]\n", newargv[0], kidpid);
                continue;
        } else {
            for (;;) {
                pid = wait(NULL);
                if (kidpid == pid) {
                    break;
                }
            }
        }

        in_flag = 0, out_flag = 0, amp_flag = 0, pipe_flag = 0, bg_flag = 0, till_flag = 0; //reset flags
    }
    killpg(getpgrp(), SIGTERM);// Terminate any children that are
                                      // still running. WARNING: giving bad args
                                      // to killpg() can kill the autograder!
    printf("p2 terminated.\n");// MAKE SURE this printf comes AFTER killpg
    exit(0);
    }
/*
parse() basically sets the appropriate flags when getword() encounters a metacharacter and inputs all other words in the pointer array newargv.
It returns an array of words per line of code. 
*/
int parse() {
    int word_count = 0, ptr = 0, i = 0, abs_num_char = 0;
    in_flag = 0, out_flag = 0, amp_flag = 0, pipe_flag = 0, bg_flag = 0, till_flag = 0, hereis_flag = 0, error_flag = 0;
    infile = NULL, outfile = NULL, till_str = NULL;
    
    for(;;) {   
        num_char = getword(char_array + ptr); //returns number of character per word
        if (num_char == 0 || num_char == -255) { //to prevent 0 and eof from being reached
            break;
        }
        if (word_count + 1 == MAXITEM) {    //to make sure 100 words in the array aren't entered
            fprintf(stderr,"Error! Too many args entered.\n");
            error_flag++;
        } if (amp_flag != 0) { // if the right & is seen and continue thru parse
            bg_flag++;
            amp_flag = 0;
            continue; 
        /*
        The way we set up our getword(), will set the value of the characters to negative if a $ is at the beginning of the line. So, if we encounter a negative value, we know that we are at a $ in the parser.
        We want to get the environment of the the current word we are at in parser so we will store it in the char pointer, env_str. If < or > is seen, but the outfile or infile is set to null, we want to set 
        one of those, depending on which was signaled, to env_str, else set it to newargv[i] (i is the current index). 
        */
        } else if (num_char < 0) { //if we found the $ at the front of a word
            char *env_str = char_array + ptr;
                if (getenv(env_str) == NULL) { //get the environment 
                    fprintf(stderr, "Error! No such file or directory named: %s.\n", (char_array + ptr)); //print if the file does not exist
                    error_flag++;
                } else {
                    if (out_flag == 1) { // at the first sighting of the out_flag we want to store the environ in it just in case there is also an in_flag, we dont want to accidentally set it to the infile
                        outfile = getenv(env_str);
                    } else if (in_flag == 1) {// at the first sighting of the in_flag we want to store the environ in it just in case there is also an out_flag, we dont want to accidentally set it to the outfile
                        infile = getenv(env_str);
                    } else {
                        newargv[i] = getenv(env_str); //else store the environment in newargv at the current index
                    }
                    word_count++; //increment the number of words there are in the line
                    i++; //increment the index
                }
        /*
        This is for the << command. If < is the current character and < is the next pointer it will set the hereis_file to 1 and set the delim_name to NULL to get it set up for when we parse again and store the delimiter name.
        Because we see that the hereis_flag == 1, we will set our delim_name eqaual to the current pointer in our parser.
        */
        } else if (((*(char_array + ptr) == '<') && (*(char_array + ptr + 1) == '<')) || hereis_flag == 1 ) {
            hereis_flag++; //we want to save the index after << so this will help us achieve that. If we just saw the << we want this flag to help us get to the next word
            if (hereis_flag == 2) { //we have incremented to the word after << and now we will save this delimiter
                delim_name = char_array + ptr; //find delim argument
            }    
            if (hereis_flag == 1){
                delim_name = NULL; //sets up the delim name
            }
        /*
        This operates the same way as << as it looks for the < command, sets the flag to 1, and after it iterates once more, the current pointer will be set to the infile and we will increment the flag to 2.
        */
        } else if (*(char_array + ptr) == '<' || in_flag == 1) {     
            in_flag++;
            if (in_flag == 2) {
                if (infile == NULL)
                    infile = (char_array + ptr); //save the file after < has been seen
            }
            if (in_flag == 1) {
                infile = NULL;
            }
        /*
        This operates almost identical to the < command, but we have to check for the >> case. If we come across >> in our parser, we want to make sure that we do not set the out_flag and outfile. Instead we want to store
        it in newargv[i] and newargv[i+1] and increment i and word_count.
        */
        } else if (*(char_array + ptr) == '>' || out_flag == 1) {
            if (*(char_array + ptr + 1) == '>') { //this is if there so happens to be a />/> in the code, we dont want it to function like we expect > to
                newargv[i] = char_array + ptr;
                newargv[i+1] = char_array + ptr + 1;
                i++;//iterate over
            }  else {
                out_flag++;
                if (out_flag == 2) {
                    if (outfile == NULL){
                        outfile = (char_array + ptr);//save the file after > has been see
                    }
                }
                if (out_flag == 1) {
                    outfile = NULL;
                }
            }
        /*
        This checks to see if there is a pipe, and if there is, we want to set the current index, i, to NULL in the newargv array so that execvp knows when to stop reading. Then we want to store the next word in an int
        variable called "pipe_index" so that we know where the command is at. We also created a pipe_pos array that stores the index of that pipe command to that corralating pipe_flag. then we increment pipe_flag and i.
        */  
        } else if (*(char_array + ptr) == '|') {
            newargv[i] = NULL; // so that execvp knows stop reading
            pipe_index = i+1; //sets the index after the pipe for second command
            pipe_pos[pipe_flag] = pipe_index;
            pipe_flag++;
            i++;
        /*
        This checks for a ~ that we set in getword(). It functions almost the same way as the << in main(). 
        */
        } else if (till_flag != 0) { //in this whole piece of code, we want to get the absolute path when ~ is next to some characters in a line
            FILE *pw_str = NULL;
            char *line_buf = NULL;
            char *token = NULL, *final_token = NULL, *root = NULL, *after_root = NULL;
            size_t line_buf_size = 0;
            int u = 0;
            till_str = char_array + ptr;
            final_token = NULL;
            pw_str = fopen("/etc/passwd", "r"); // we want to open the file to access a list of all the directories in the system
            if (!pw_str) {
                fprintf(stderr, "Error! Cannot open /etc/passwd file.\n");
                error_flag++;
            } else {
                root = strtok(till_str,"/"); //this grabs the root
                after_root = strtok(NULL, " "); //this grabs the stuff after the root
                while (getline(&line_buf, &line_buf_size, pw_str) >= 0) { //we use a while loop to search through the entire file to find the directory we want
                    token = strtok(line_buf,":"); //strtok() which divides the list by a delimiter. This just stores the first index in that huge list
                    if (strcmp(token, till_str) == 0) { //we found the desired index in the list
                        for (u = 0; u < 5; u++) {
                            token = strtok(NULL, ":"); //we use this to grab the absolute address from that line by dividing that line again by the delimiter
                        }
                        final_token = token; //this is the absolue address
                        break; //break out of the while loop
                    }
                }
                free(line_buf); //we have to free the space allocated
                line_buf = NULL;
                fclose(pw_str); //close the file
                if (final_token == NULL) { 
                    fprintf(stderr, "Error! Illegal name for absolute path: %s.\n", final_token);
                    error_flag++;
                } else {
                    if (after_root != NULL) { //if there is a root, concatinate everything together
                        strcat(final_token, "/");
                        strcat(final_token, after_root);
                    }
                newargv[i] = final_token; //add the absolute path at the current index in newargv
                }
            }
            i++; //increment this and word_count so our parser functions as expected
            word_count++;
            till_flag = 0; //reset the flag
        } else {
            newargv[i] = char_array + ptr; //put word in newargv
            // printf("newargv[i] = %s\n", i, newargv[i]);
            word_count++; //increment the number of words there are in the line
            i++; //increment the index
        }
        abs_num_char = abs(num_char); //for $ if -number of characters
        ptr += (abs_num_char + 1); //increment pointer to next word
    }

    newargv[i] = NULL; //put NUll at end of newargv
    return word_count; //return number of words
}
void sighandler() {

}