httpserver:
	g++ -g -o httpserver http_response.cc http_context.cc http_server.cc main.cc -lmymuduo -lutils -lpthread -std=c++17

debug:
	g++ -g -o httpserver http_response.cc http_context.cc http_server.cc main.cc -lmymuduo -lutils -lpthread -std=c++17 -D MYDEBUG

clean:
	rm -f httpserver
