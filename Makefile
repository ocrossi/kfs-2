## Parameters:
## ----------
SRCDIR:=src

VERBOSE:=## Assign a value to verbose to enable logging.
DISTDIR:=dist## The image's destination directory.
DISTVOL:=kfs-dist## The destination docker-volume.
NAME:=kfs-1.iso## The name of the image.
##
CMD:=./build_iso.sh## Command to run in the docker container.

SHELL=/bin/bash
MAKEFILE:=$(CURDIR)/$(firstword $(MAKEFILE_LIST))

RE_SECTION:=^\s*\#\#
RE_RULE:=^.*:.*\#\#\s+
RE_VARIABLE:=^.*=.*\#\#\s+

SED_SECTION:=s/^\s*\#\#\s*(.*)/\1/
SED_RULE:=s/(.*):.*\#\#\s*(.*)/\1|\2/
SED_VARIABLE:=s/(.*)=(.*)\#\#\s*(.*)/\1|\3 Defaults to "\2"./

SED_MATCH:=$(RE_SECTION)|$(RE_RULE)|$(RE_VARIABLE)
SED_SUBST:={$(SED_SECTION);$(SED_RULE);$(SED_VARIABLE);p}

##
##
## Rules:
## -----
all: $(DISTDIR)/$(NAME) ## Alias for $(DISTDIR)/$(NAME).

##

help: ## Show available parameters and rules.
	@sed -n -E \
		-e '/$(RE_SECTION)/{$(SED_SECTION); p}' \
		-e '/$(RE_RULE)/{$(SED_RULE); p}' \
		-e '/$(RE_VARIABLE)/{$(SED_VARIABLE); p}' \
		$(MAKEFILE) \
	| column -L -t -s'|'

##

kfs-1: $(SRCDIR) ## Build the host docker image.
	@echo "BUILD kfs-1"
	docker build -t kfs-1 .

$(DISTDIR):
	@echo "MKDIR $(DISTDIR)"
	mkdir -p "$(DISTDIR)"

$(DISTDIR)/$(NAME): kfs-1 $(DISTDIR) ## Build the kfs image.
	if [ ! -f $(DISTDIR)/$(NAME) ] || [ ! -f "$(DISTDIR)/build-id" ] \
	   || [ "$$(cat "$(DISTDIR)/build-id")" != "$$(docker inspect -f '{{.Id}}' kfs-1)" ]; \
	then \
		set -euo pipefail; \
\
		echo "RUN $(CMD)"; \
		docker run --rm \
			-v /dev:/hostdev:ro \
			-v "$(DISTVOL):/dist:rw" \
			--name=kfs-1 \
			-it kfs-1 $(CMD); \
\
		if [ "$(CMD)" = "./build_iso.sh" ]; \
		then \
			echo "CP $(NAME) $(DISTDIR)"; \
			docker run --rm -d \
				--cap-drop=all \
				-v "$(DISTVOL):/dist:rw" \
				--name=kfs-1-dist \
				kfs-1 \
				tail -f /dev/null; \
	\
			docker cp --quiet=false kfs-1-dist:/dist/kfs-1.iso \
				$(DISTDIR)/$(NAME) 2>&1 \
			| awk '{printf "\r%s", $$0; fflush();}'; echo; \
	\
			echo "$$(docker inspect -f '{{.Id}}' kfs-1)" \
				> "$(DISTDIR)/build-id"; \
	\
			docker stop kfs-1-dist; \
	\
			printf '\a'; \
		fi \
	fi

##

qemu-run: $(DISTDIR)/$(NAME) ## Run the build image using qemu.
	@echo "Running '$(DISTDIR)/$(NAME)' using qemu..."
	qemu-system-i386 -cdrom $(DISTDIR)/$(NAME)

check-scripts:
	shellcheck $(shell find "$(SRCDIR)" -type f -name '*.sh')

.PHONY: all help kfs edit qemu-run check-scripts

$(VERBOSE).SILENT:
