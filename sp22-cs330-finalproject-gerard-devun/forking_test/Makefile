default: c

all: c go

CC=gcc

c: server-c client-c

go: client-go-sol server-go-sol #client-go server-go

client-c: client-c.c
	$(CC) client-c.c -o client-c

server-c: server-c.c
	$(CC) server-c.c -o server-c

client-c-sol: client-c-sol.c
	$(CC) client-c-sol.c -o client-c
server-c-sol: server-c-sol.c
	$(CC) server-c-sol.c -o server-c

client-go: client-go.go
	go build client-go.go

server-go: server-go.go
	go build server-go.go

client-go-sol: client-go-sol.go
	go build -o client-go client-go-sol.go

server-go-sol: server-go-sol.go
	go build -o server-go server-go-sol.go

clean:
	rm -f server-c client-c server-go client-go *.o
