COMMON_SRC = src/recipe.c src/mash.c src/ibu.c src/yeast.c src/srm.c src/linenoise.c
MAINSPRING_SRC = src/tokenizer.c src/langs/brewshell.c
DEBUG ?= -DDEBUG -g -Og
CFLAGS ?= -Wno-implicit-function-declaration -Wno-unused-result -I./include -I./include/mainspring
LDFLAGS ?= -lm -lsqlite3
CC ?= gcc

DATABASE ?= /var/db/brewshell.sqlite

all: brewshell

brewshell: $(COMMON_SRC) $(MAINSPRING_SRC) src/brewshell.c
	$(CC) $^ $(LDFLAGS) $(DEBUG) $(CFLAGS) -o $@

install: brewshell
	install -m 0555 -o $(WEBUSER) src/builder_ui.js include/global.css include/builder.css $(WEBROOT)/include
	install -m 0777 -o $(WEBUSER) $(PAGES) $(WEBROOT)

install_database:
	sqlite3 $(DATABASE) < db/default.sql
	sqlite3 $(DATABASE) < db/styles.sql
	chown $(WEBUSER):$(WEBGROUP) $(DATABASE)

clean:
	rm $(PAGES)

clean_database:
	rm $(DATABASE)

.PHONY: all install clean clean_database install_database
