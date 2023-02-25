CX = g++ -std=c++11 -g 
LDFLAGS=-lboost_system -lpthread
objects = main.o socket.o proxy.o request.o cache.o response.o
%.o:%.cpp 
	$(CX) -O -c $< 
all:main
main: $(objects)
	$(CX) -O -o main $(objects) $(LDFLAGS)

.Phony:clean

clean:
	rm -rf *.o deamon *.o~ *.cpp~ *#
