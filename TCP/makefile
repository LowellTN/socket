all: serv clt

serv: ./orchestrator/server.c
	gcc ./orchestrator/server_ssl.c -o server_ssl -lssl -lcrypto


clt: ./agent/client.c
	gcc ./agent/client_ssl.c -o client_ssl -lssl -lcrypto

run_serv: serv
	./server_ssl

run_clt: clt
	./client_ssl 192.168.100.1 2222

clean:
	rm -f server client