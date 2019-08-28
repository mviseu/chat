TOPTARGETS := all clean

SUBDIRS := $(wildcard */.)

.PHONY: $(TOPTARGETS) $(SUBDIRS)


$(TOPTARGETS): $(SUBDIRS)


$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)