NAME = webserver
VPATH = HttpServer HttpRequestParser HttpResponseParser ConfigFileParser Utils CGI

override SRCS = main.cpp \
								Server.cpp \
								HttpRequestParser.cpp \
								HttpRequestParserExcpt.cpp \
								HttpResponseParserExcept.cpp \
								HttpResponseParser.cpp \
								GetResponseParser.cpp \
								AllowedMethods.cpp \
								ClientMaxBodySize.cpp \
								ConfigFileParser.cpp \
								ConfigFileParserExcept.cpp \
								ErrorPage.cpp \
								Index.cpp \
								Location.cpp \
								Return.cpp \
								ServersParser.cpp \
								Utils.cpp \
								HttpServer.cpp \
								PostResponseParser.cpp \
								DeleteResponseParser.cpp \
								CGI.cpp

HEADERS = $(addprefix -I ,$(VPATH))
OBJDIR = obj
OBJ = $(SRCS:%.cpp=$(OBJDIR)/%.o)
CXXFLAGS = -std=c++98 -Wall -Wextra -Werror -g3
override RM = rm -rf
CXX = c++

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(HEADERS) -c $< -o $@

$(OBJDIR):
	mkdir -p $@

clean:
	$(RM) $(OBJDIR)

fclean: clean
	$(RM) $(NAME)

docker/run: docker/buildImage
	docker run --rm -v $$PWD:/webserver -p 8088:8088 -p 8089:8089 -p 8090:8090 -it webserver:latest bash

docker/buildImage:
	docker build -t webserver:latest .

docker/build: docker/buildImage
	docker run --rm webserver:latest make

docker/test: docker/buildImage
	docker run --rm webserver:latest make test

nginx:
	docker build -t nginx_webserver:latest -f Dockerfile.nginx .
	docker run --rm -it -p 8000:8000 nginx_webserver:latest

re: fclean all

.PHONY: all clean fclean re docker/build docker/test docker/buildImage
