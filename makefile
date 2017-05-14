make:   gerador.c sauna.c
	gcc gerador.c -o gerador -Wall -pthread
	gcc sauna.c -o sauna -Wall -pthread

clean:
	rm gerador
	rm sauna
