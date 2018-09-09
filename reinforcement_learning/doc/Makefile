.PHONY: clean docs

all: clean docs

clean:
	rm -rf ./python/html
	rm -rf ./cpp/html

docs:
	cd cpp; doxygen
	cd python; doxygen
