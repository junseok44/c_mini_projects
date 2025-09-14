# C Programming Practice Projects

A collection of practice projects created while learning C programming and systems programming.


## 📁 Projects

| Project | What I Learned | Difficulty | Description |
|---------|----------------|------------|-------------|
| [**mycp**](mycp/) | File handling, recursion | ⭐⭐ | Implementation of cp command |
| [**stack-calculator**](stack-calculator/) | Stack, algorithms | ⭐⭐⭐ | Calculator with parentheses support |
| [**json-parser**](json-parser/) | Parsing, memory management | ⭐⭐⭐⭐ | JSON parser and value extractor |
| [**mini-bash-like-shell**](mini-bash-like-shell/) | Processes, pipes | ⭐⭐⭐⭐ | Simple shell implementation |
| [**socket-server**](socket-server/) | Network programming | ⭐⭐⭐⭐⭐ | Multi-client chat server |

## 🤔 What I Learned

### Systems Programming
- **File Operations**: Low-level functions like open, read, write
- **Processes**: Fork child processes and exec other programs
- **Pipes**: Connect programs to exchange data
- **Networking**: Create servers and manage clients with sockets

### Data Structures
- **Stack**: LIFO structure (used in calculator)
- **String Parsing**: Breaking text into meaningful tokens
- **Trees**: Storing JSON data in tree structures

### C Language
- **Pointers**: Double pointers, function pointers, etc.
- **Memory Management**: Proper use of malloc/free
- **Structures**: Creating complex data structures
- **Modularization**: Organizing code into .h and .c files

## 🛠️ How to Run

### Prerequisites
- Linux or macOS (or WSL)
- gcc compiler
- make

### Building
```bash
# Run make in each directory
cd json-parser && make && cd ..
cd mini-bash-like-shell && make && cd ..
cd mycp && make && cd ..
cd socket-server && make && cd ..
cd stack-calculator && make && cd ..
```

### Quick Testing
```bash
# 1. File copy tool test
echo "Hello World" > test.txt
./mycp/bin/mycp test.txt backup.txt

# 2. Calculator test  
echo "2 + 3 * 4" | ./stack-calculator/bin/calc

# 3. JSON parser test
echo '{"name": "test", "value": 42}' > test.json
./json-parser/bin/json-parser parse test.json

# 4. Shell test
./mini-bash-like-shell/bin/minishell
# In shell: echo "hello" | cat

# 5. Socket server test
./socket-server/bin/socket-server 8080 &
telnet localhost 8080
```

## 📖 Project Details

### 1. [mycp](mycp/) - cp Command Implementation
```bash
./bin/mycp source.txt dest.txt      # Copy file
./bin/mycp source_dir/ dest_dir/    # Copy directory
```
**Learned**: File I/O, recursive directory traversal

### 2. [stack-calculator](stack-calculator/) - Calculator
```bash
./bin/calc
enter the expression: ( 2 + 3 ) * 4
result: 20
```
**Learned**: Stack data structure, expression evaluation algorithms

### 3. [json-parser](json-parser/) - JSON Parser
```bash
./bin/json-parser parse data.json           # Parse JSON
./bin/json-parser get data.json "user.name" # Extract specific value
```
**Learned**: String parsing, tree structures, memory management

### 4. [mini-bash-like-shell](mini-bash-like-shell/) - Shell Implementation
```bash
./bin/minishell
$ ls | grep txt > results.txt
$ cd /tmp && pwd
$ exit
```
**Learned**: Process creation, pipes, redirection

### 5. [socket-server](socket-server/) - Chat Server
```bash
./bin/socket-server 8080
# When client connects:
/nick Alice
Hello everyone!
```
**Learned**: Network programming, handling multiple clients concurrently

## 📁 Directory Structure

All projects follow a similar structure:

```
project/
├── src/           # .c source files 
├── include/       # .h header files
├── build/         # Compiled .o object files
├── bin/           # Final executables
├── Makefile       # Build configuration
└── README.md      # Documentation
```

### Make Commands
```bash
make           # Compile
make clean     # Clean build files
```


## 💡 Helpful Resources

Useful learning resources:
- [Learn C Programming](https://www.learn-c.org/) - Interactive C tutorial
- [C Programming Tutorial](https://www.tutorialspoint.com/cprogramming/) - Comprehensive C guide
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) - Socket programming made easy
- [The Linux Programming Interface](http://man7.org/tlpi/) - Systems programming examples
- [GeeksforGeeks C Programming](https://www.geeksforgeeks.org/c-programming-language/) - Practice problems and concepts

## 📝 Notes

- Started from C basics and gradually tackled more challenging projects
- Memory management was the most difficult part; pointers are still confusing
- Network programming turned out to be more interesting than expected
