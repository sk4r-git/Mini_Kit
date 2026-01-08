.PHONY: all kernel user gui test clean

all: kernel user test gui
	@echo "Tout est compilé !"

# Appelle les Makefiles des sous-dossiers
kernel:
	$(MAKE) -C kernel

user:
	$(MAKE) -C user

gui:
	$(MAKE) -C gui

test:
	$(MAKE) -C test

# Nettoyage global
clean:
	$(MAKE) -C kernel clean
	$(MAKE) -C user clean
	$(MAKE) -C gui clean
	$(MAKE) -C test clean
	@echo "Nettoyage terminé !"