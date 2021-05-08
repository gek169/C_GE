
LIB = lib/libTinyGL.a

all: $(LIB) CODE
	@echo Done!

$(LIB):
	cd src_tgl && $(MAKE) && cd ..
	cp src_tgl/*.a ./lib/
lua/liblua.a:
	cd lua && $(MAKE) && cd ..
	cp lua/*.a ./lib/

CODE: $(LIB) lua/liblua.a
	cd src && $(MAKE) && cd ..
	-mv src/main ./build/
	-mv src/*.bin ./build/
	-mv src/*.exe ./build/
clean:
	cd src_tgl && $(MAKE) clean && cd ..
	cd lua && $(MAKE) clean && cd ..
	cd src && $(MAKE) clean && cd ..
	cd lib && rm -f *.a && cd ..
	cd build && rm -f main && cd ..
	cd build && rm -f *.exe && cd ..
	cd build && rm -f *.bin && cd ..
update_tinygl: clean
	rm -rf src_tgl
	mkdir src_tgl
	mkdir temp
	cd temp && git clone https://github.com/gek169/tinyglplusplus.git && cd ..
	cp -r temp/tinyglplusplus/src/* ./src_tgl/
	cp -r temp/tinyglplusplus/include-demo/* ./include/
	cp -r temp/tinyglplusplus/include/* ./include/
	rm -rf temp
