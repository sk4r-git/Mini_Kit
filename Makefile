.PHONY: all kernel user gui clean

all: kernel user gui
	@echo "Tout est compilé !"

# Appelle les Makefiles des sous-dossiers
kernel:
	$(MAKE) -C kernel

user:
	$(MAKE) -C user

gui:
	$(MAKE) -C gui

# Nettoyage global
clean:
	$(MAKE) -C kernel clean
	$(MAKE) -C user clean
	$(MAKE) -C gui clean
	@echo "Nettoyage terminé !"