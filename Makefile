BUILD_DIR=build

build:
	cmake --build ${BUILD_DIR} --parallel

config:
	cmake \
		-DCMAKE_BUILD_TYPE=Debug \
		-B ${BUILD_DIR}

config-release:
	cmake \
		-DCMAKE_BUILD_TYPE=Release \
		-B ${BUILD_DIR}

all: config build

install:
	cmake --install ${BUILD_DIR}

clean:
	rm -rf build

.PHONY: build
