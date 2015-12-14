all: program1 program2 program3 program4 program5

program1:
	gcc server.c -pthread -g -o server indexandhashing.c filelochash.c

program2:
	gcc directory.c -g -pthread -o dir

program3:
	gcc client.c -g -o client

program4:
	gcc workerDirectory.c -g -pthread -o workDir
	
program5:
	gcc worker.c -g -pthread -o work indexandhashing.c
