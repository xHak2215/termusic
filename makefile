build:
	@echo "building..."
	@gcc main.c -o termusic -ldl -lm -lpthread
