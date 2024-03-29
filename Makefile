FILES_TO_BACKUP = my402list.c my402list.h warmup2.c warmup2.h
BACKUP_FNAME = warmup2-backup-`date +%d%b%Y-%H%M%S`.tar.gz
BACKUP_DIR = $(HOME)/Shared-ubuntu

warmup2: warmup2.o my402list.o
	gcc -o warmup2 -g warmup2.o my402list.o -lpthread -lm

warmup2.o: warmup2.c my402list.h
	gcc -g -c -Wall warmup2.c

my402list: my402list.o
	gcc -o my402list -g my402list.o

my402list.o: my402list.c my402list.h
	gcc -g -c -Wall my402list.c

clean:
	rm -f *.o *.submitted *.csh *.sh *.out warmup2 my402list f* analyze-trace.txt

test:
	cp w2data/scripts/section-A.csh .
	cp w2data/scripts/section-B.csh .
	cp w2data/scripts/analyze-trace.txt .
	chmod 755 *.csh analyze-trace.txt

backup:
	tar cvzf $(BACKUP_FNAME) $(FILES_TO_BACKUP)
	@if [ -d $(BACKUP_DIR)/ ]; then \
		mv $(BACKUP_FNAME) $(BACKUP_DIR)/$(BACKUP_FNAME); \
		echo ; \
		echo "Backup file created in shared folder: $(BACKUP_DIR)/$(BACKUP_FNAME)"; \
		/bin/ls -l $(BACKUP_DIR)/$(BACKUP_FNAME); \
	else \
		echo ; \
		echo "$(BACKUP_DIR) inaccessible, local backup file created: $(BACKUP_FNAME)"; \
		/bin/ls -l $(BACKUP_FNAME); \
	fi

