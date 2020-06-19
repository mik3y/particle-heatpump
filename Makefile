clean:
	rm -f *.bin

toc:
	doctoc --notitle --github README.md

.PHONY: clean toc

