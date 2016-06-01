
all : speedtest_server speedtest_client

% : %.c
	gcc -o $@ $^
