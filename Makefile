all: client serve

client:
	gcc DUMBclient.c -o DUMBclient

serve:
	gcc DUMBserver.c -o DUMBserve
	
clean:
	rm -f DUMBclient
	rm -f DUMBserve
