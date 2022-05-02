# PL-0-Compiler
PL/0 is similar to the general-purpose programming language Pascal. It serves as an example of how to construct a compiler.

To compile the Pl/0 Compiler using gcc:

	gcc -o compiler compiler.c -lm

To run the PL/0 Compiler, replace the "input.txt" with any input file you wish:

	./compiler

To view output files (for example, output.txt):

	vi output.txt



________



Lexical Analyzer/Scanner
	Input:  input.txt
	output: LAout.txt

Parser/Code Generator
	Input:  LAout.txt
	Output: code.txt

Virtual Machine
	Input:  code.txt
	Output: output.txt
