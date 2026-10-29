all: funcionalidade_1.o programaTrab.o funcao-fornecida.o 
	gcc -o programaTrab *.c 

funcionalidade_1.o:
	gcc -c funcionalidade_1.c -o funcionalidade_1.o
	
funcao-fornecida.o:
	gcc -c funcao-fornecida.c -o funcao-fornecida.o

programaTrab.o:
	gcc -c programaTrab.c -o programaTrab.o

run:
	./programaTrab
	 
clear:
	rm *.o programaTrab

zip:
	zip -R Trabalho_01.zip . '*.c' '*.h' 'Makefile'