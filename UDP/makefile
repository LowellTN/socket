all: serv clt

serv: serveurUDP.c
	gcc -o serveurUDP serveurUDP.c

clt: clientUDP.c
	gcc -o clientUDP clientUDP.c

run_serv: serveurUDP
	./serveurUDP

run_clt: clientUDP
	./clientUDP 127.0.0.1 2222

clean:
	rm -f serveurUDP clientUDP