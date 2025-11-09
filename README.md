# Menu

A minimal interactive menu program written in C.
It takes command-line arguments as menu options, allows navigation using the **Up** and **Down** arrow keys, and highlights the currently selected item in magenta. 
The program returns the index of the selected option upon exiting.

## Compilation

Use the provided Makefile to compile the project:
```
make
```
This will create the executable file named menu.

## Usage

Run the compiled executable and pass the menu options as command-line arguments:
```
./menu "Option A" "Option B" "Option C" "Option D"
```
