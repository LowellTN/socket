all: serv clt

serv: serveurTCP.c
	gcc -o serveurTCP serveurTCP.c

clt: clientTCP.c
	gcc -o clientTCP clientTCP.c

run_serv: serveurTCP
	./serveurTCP

run_clt: clientTCP
	./clientTCP 127.0.0.1 2222 "Bonjour, Serveur !"

clean:
	rm -f serveurTCP clientTCP