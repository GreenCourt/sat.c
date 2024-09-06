COMPILER:=boncc2 # my compiler instead of gcc
CFLAGS:=-Wall -Wextra -Werror -O2 -g
LDFLAGS:=-znoexecstack

#LDFLAGS+=-fsanitize=address,undefined
#CFLAGS+=-fsanitize=address,undefined
#CFLAGS+=-DNDEBUG

TESTS=$(basename $(notdir $(wildcard test/*.c)))

sat: $(addprefix build/,main.o cnf.o solve.o subset.o)
	$(CC) -g -o $@ $^ $(LDFLAGS)

fmt:
	clang-format -i *.c *.h test/*.c

test: $(addprefix build/test/,$(TESTS))
	for i in $^; do echo "$$i"; "$$i" || exit $$?; done

build/%.o:%.c
	@mkdir -p $(dir $@)
	$(COMPILER) $(CFLAGS) -MMD -c -o $@ $<

build/test/%: $(addprefix build/, test/%.o cnf.o solve.o subset.o)
	$(CC) -g -o $@ $^ $(LDFLAGS)

clean:
	rm -rf sat build

-include build/*.d build/*/*.d

.PHONY: test clean fmt
