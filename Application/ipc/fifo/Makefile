all:fifo pipe

pipe:
	@gcc pipe.c -o pipe
	@mv pipe out

fifo:
	@gcc fifo_sev.c -o fifo_serv
	@gcc fifo_cli.c -o fifo_cli
	@mv fifo_serv fifo_cli out
