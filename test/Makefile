check:
	./dotest -k
	$(MAKE) clean
test: check
clean:
	for a in * ; do case $$a in \
            README | Makefile | dotest | pngtype.pl ) ;; \
	    * ) [ -d $$a ] || rm $$a ;; esac ; done
