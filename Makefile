TARGETS=uuc
.PHONY=all uuc clean

all: $(TARGETS)

uuc: libu/libu.a
	make -C src/c/
	ln -s src/c/uuc ./uuc -f

test: uuc
	./examples/test.sh

libu/libu.a:
	MAP=linear make -C libu/

clean:
	make -C src/c/ clean
	make -C libu/ clean


fullclean: clean
	$(RM) uuc
	make -C libu/ fullclean
