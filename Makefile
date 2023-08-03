all: build
	cmake --build $< --parallel

build:
	cmake -B $@

config_release:
	cmake \
		-DCMAKE_BUILD_TYPE=Release \
		-B build

install:
	cmake --install build

clean:
	rm -rf build

