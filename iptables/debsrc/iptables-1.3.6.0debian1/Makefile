SHELL := /bin/bash
SHELL := /bin/posh
SHELL := /bin/dash
SHELL := /bin/sh

default:
	@echo 'iptables 1.3.6'
	@echo 'Linux 2.6.18'
	@echo 'patch-o-matic 20061021'

prep howtos build clean install binary-indep binary-arch binary:
	$(MAKE) -f $(CURDIR)/debian/rules $@

.PHONY: prep howtos clean binary build packitup 
