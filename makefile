build:
	@echo "building..."
	@gcc main.c -o termusic -ldl -lm -lpthread

run:
	@echo "building..."
	@gcc main.c -o termusic -ldl -lm -lpthread
	@echo "running..."
	@./termusic