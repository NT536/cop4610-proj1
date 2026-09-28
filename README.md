# Shell

[Description]

## Group Members
- **Malachi Davey**: mld24d@fsu.edu
- **Nathaniel Thompson**: ntt23@fsu.edu
## Division of Labor

### Part 1: Prompt
- **Responsibilities**: [Description]
- **Assigned to**: Nathaniel Thompson

### Part 2: Environment Variables
- **Responsibilities**: [Description]
- **Assigned to**: Nathaniel Thompson

### Part 3: Tilde Expansion
- **Responsibilities**: [Description]
- **Assigned to**: Nathaniel Thompson

### Part 4: $PATH Search
- **Responsibilities**: Search each directory in $PATH for the command and return the full path to the executable (`path-search.c`).
- **Assigned to**: Malachi Davey, Nathaniel Thompson

### Part 5: External Command Execution
- **Responsibilities**: [Description]
- **Assigned to**: Malachi Davey, Nathaniel Thompson

### Part 6: I/O Redirection
- **Responsibilities**: Parse `<` and `>` from a command, check the input file exists before forking, and redirect stdin/stdout in the child. Output files are created with `-rw-------` and overwritten (`redirection.c`).
- **Assigned to**: Malachi Davey, Nathaniel Thompson

### Part 7: Piping
- **Responsibilities**: Split a command on `|` into up to 3 commands, connect them with pipes, and run them together. Invalid pipelines such as `ls |` are rejected (`piping.c`).
- **Assigned to**: Malachi Davey, Nathaniel Thompson

### Part 8: Background Processing
- **Responsibilities**: Run commands ending in `&` without waiting, track them with job numbers, print `[job] [pid]` on start and `[job] + done [cmd]` when finished. Works with piping and redirection (`jobs.c`).
- **Assigned to**: Malachi Davey, Nathaniel Thompson

### Part 9: Internal Command Execution
- **Responsibilities**: [Description]
- **Assigned to**: Malachi Davey, Nathaniel Thompson

### Part 10: External Timeout Executable
- **Responsibilities**: [Description]
- **Assigned to**: Malachi Davey, Nathaniel Thompson

### Extra Credit
- **Responsibilities**: Support piping combined with I/O redirection.
- **Assigned to**: Malachi Davey, Nathaniel Thompson

## File Listing
```
shell/
│
├── src/
│   ├── main.c
│   ├── lexer.c
│   ├── path-search.c
│   ├── redirection.c
│   ├── piping.c
│   └── jobs.c
│
├── include/
│   ├── main.h
│   ├── lexer.h
│   ├── path-search.h
│   ├── redirection.h
│   ├── piping.h
│   └── jobs.h
│
├── README.md
└── Makefile
```
## How to Compile & Execute

### Requirements
- **Compiler**: gcc is the required compiler, as the work is in c.
- **Dependencies**: List any libraries or frameworks necessary (rust only).

### Compilation
For a C/C++ example:
```bash
make
```
This will build the executable in ...
### Execution
```bash
make run
```
This will run the program ...

## Development Log
Each member records their contributions here.

### Malachi Davey

| Date       | Work Completed / Notes |
|------------|------------------------|
| 2026-09-14 | Met with Nathaniel to plan the timeline. Reviewed the spec to confirm I could start on $PATH search once the base shell loop was done. |
| 2026-09-23 | Set up VSCode with SSH to linprog after running into connection and shell issues. Completed part 4 ($PATH search) and pushed it to its own branch. Found a few build issues in the shared files and let Nathaniel know. |
| 2026-09-27 | Completed part 6 (I/O redirection). Fixed some build errors that were stopping the shell from compiling and added a .gitignore for the build folders. Tested input, output, and both together on linprog4, all working. |
| 2026-09-27 | Completed part 7 (piping), including the extra credit for combining pipes with redirection. Tested multiple pipes and error cases, no hangs. |
| 2026-09-27 | Completed part 8 (background processing). Added a jobs file so Nathaniel can finish the jobs and exit commands. Tested background commands with pipes and redirection, all working. |
| 2026-09-28 | Fixed blank input printing an error. Pushed part 8 and coordinating with Nathaniel on merging everything into main before submission. |

### Nathaniel Thompson

| Date       | Work Completed / Notes |
|------------|------------------------|
| 2026-09-14 | Created the prompt in the main.c file. May move to lexer.c dependent on office hours tomorrow.  |
| 2026-09-16 | Was unable to locate Rasheeq Ishmam in LOV006, consulting with Ryan Schmidt today. |
| 2026-09-16 | After consulting with Ryan Schmidt, I have completed parts 1-3 of the base code in main.c file.  |
| 2026-09-23 | Testing execution with execvp(), will redo code with proper path search afterwards |
| 2026-09-25 | Redoing external execution with execv(), following up with internal commands |
| 2026-09-27 | Finishing testing of internals and externals. Require background processing complete before finishing execution of jobs and exit. Otherwise functional. Need to bugfix expansion of $ variables. |



## Meetings
Document in-person meetings, their purpose, and what was discussed.

| Date       | Attendees            | Topics Discussed | Outcomes / Decisions |
|------------|----------------------|------------------|-----------------------|
| 2026-09-14 | Nathaniel Thompson, Malachi Davey| Discussed timeline for basic shell development | Nathaniel will have the basic shell developed by tomorrow so Malachi can start on his part of the project.  |
| YYYY-MM-DD | [Names]              | [Agenda items]   | [Actions/Next steps]  |



## Bugs
- **Bug 1**: `&` must be its own token for background processing (`sleep 5 &` works, `sleep 5&` does not).
- **Bug 2**: This is bug 2.
- **Bug 3**: This is bug 3.

## Extra Credit
- **Extra Credit 1**: Piping combined with I/O redirection. Input redirection is allowed on the first command and output redirection on the last (e.g. `sort < in.txt | uniq > out.txt`).
- **Extra Credit 2**: [Extra Credit Option]
- **Extra Credit 3**: [Extra Credit Option]

## Considerations
[Description]