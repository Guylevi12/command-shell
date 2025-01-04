**Custom Shell Program**
**Overview**
This project is a simple custom shell implementation in C that supports various features like aliases, job management, error handling, and script execution. The shell can execute commands, manage background jobs, and handle custom aliases. It also supports redirecting error outputs to files and running scripts.

**Features**
**Job Management:** You can run background processes and view the list of jobs.
**Alias Support:** Create, remove, and list aliases.
**Error Handling:** Redirect error output to files.
**Script Execution:** Run shell scripts from files.
**Multiple Commands:** Supports &&, ||, and & for chaining commands.
**Interactive Prompt:** The shell provides a prompt showing the number of executed commands, aliases, and script lines.
Key Commands
**exit_shell:** Exits the shell.
**jobs:** Lists the currently running background jobs.
**alias [name]='[command]':** Defines an alias.
**unalias [name]:** Removes an alias.
**source [script_name]:** Executes a script.
&&, ||, &: Chaining commands based on success (&&), failure (||), or running in the background (&).
2>: Redirects error output to a file.

**Usage**

**Run the Shell:**

**Compile the program:**
gcc -o custom_shell custom_shell.c
**Execute the shell:**
./custom_shell

**Exit the Shell:**
Type exit_shell to exit the shell.

**Create an Alias:**
alias [name]='[command]'
Example: alias ls='ls -l'

**Remove an Alias:**
unalias [name]
Example: unalias ls

**List Jobs:**
Type jobs to list the background jobs.

**Run a Script:**
Create a script file with the extension .sh.
Use the source command to run the script:
Example: source script.sh

**Redirect Error Output:**
Use 2> to redirect error output to a file.
Example: ls nonexistent_dir 2> error.log

**Code Structure**

**1. Main Shell Logic:**
main(): The entry point that starts the interactive shell loop, handles user input, and calls necessary functions to process commands.
runShell(): Handles the execution of commands, managing jobs, and aliases.

**2. Job Management:**
addJob(): Adds a job to the job list when a background job is initiated.
deleteJob(): Removes a job from the job list when the job completes.
printJobs(): Prints the list of background jobs.

**3. Alias Management:**
addAlias(): Adds a new alias.
removeAlias(): Removes an alias.
printAlias(): Prints all defined aliases.

**4. Error Handling:**
error_handler_to_file(): Handles redirection of error output to a file.
sigchld_handler(): Handles the termination of child processes and updates job status.

**5. Script Execution:**
read_Script(): Reads and executes a shell script.

**6. Input Parsing:**
input_to_argument(): Converts user input into arguments for command execution.
find_argument(): Finds individual arguments in the user input.

**7. Helper Functions:**
trimSpaces(): Trims spaces from the input string.
free_arg(): Frees allocated memory for arguments.

**Error Handling**
General Errors: If an error occurs during command execution, the shell prints ERR.
File Errors: If an invalid file is provided, or a command cannot be executed, an error message is shown.
Alias Errors: If there are issues with alias definitions or removals, the shell handles these gracefully.

**Requirements**
C Compiler (e.g., GCC)
Linux/Unix-based system for running the shell (since it uses fork(), execvp(), etc.)

**Example**
less
Copy code
#cmd:0|#alias:0|#script lines:0> alias ls='ls -l'
#cmd:1|#alias:1|#script lines:0> ls
total 8
-rwxr-xr-x 1 user user 8488 Jan  4 12:34 custom_shell
-rw-r--r-- 1 user user 1206 Jan  4 12:34 custom_shell.c
#cmd:2|#alias:1|#script lines:0> jobs
#cmd:3|#alias:1|#script lines:0> exit_shell

**License**
This project is open-source and available under the MIT License.
