#! /usr/bin/make -f

################################################################################
# Copyright (C) 2019	Alejandro Colomar Andrés
# SPDX-License-Identifier:	GPL-2.0-only
################################################################################
# *AUTHOR*
# FULL NAME	"Alejandro Colomar Andrés"
# EMAIL		"1903716@gmail.com"
################################################################################

################################################################################
# dependencies

OBJ		=							\
		label.o							\
		main.o							\
		parse.o

LABEL_INC	=							\
		$(INC_DIR)/label/label.h
LABEL_SRC	=							\
		$(SRC_DIR)/label.c

MAIN_INC	=							\
		$(INC_DIR)/label/label.h				\
		$(INC_DIR)/label/parse.h
MAIN_SRC	=							\
		$(SRC_DIR)/main.c

PARSE_INC	=							\
		$(INC_DIR)/label/parse.h
PARSE_SRC	=							\
		$(SRC_DIR)/parse.c

################################################################################
# target: dependencies
#	action

PHONY := all
all: $(OBJ)


label.s: $(LABEL_SRC) $(LABEL_INC)
	@echo	"	CC	label/tmp/$@"
	$(Q)$(CC) $(CFLAGS) -I $(INC_DIR) -S $< -o $@
label.o: label.s
	@echo	"	AS	label/tmp/$@"
	$(Q)$(AS) $< -o $@

main.s: $(MAIN_SRC) $(MAIN_INC)
	@echo	"	CC	label/tmp/$@"
	$(Q)$(CC) $(CFLAGS) -I $(INC_DIR) -S $< -o $@
main.o: main.s
	@echo	"	AS	label/tmp/$@"
	$(Q)$(AS) $< -o $@

parse.s: $(PARSE_SRC) $(PARSE_INC)
	@echo	"	CC	label/tmp/$@"
	$(Q)$(CC) $(CFLAGS) -I $(INC_DIR) -S $< -o $@
parse.o: parse.s
	@echo	"	AS	label/tmp/$@"
	$(Q)$(AS) $< -o $@


PHONY += clean
clean:
	@echo	"	RM	*.o *.s"
	$(Q)rm -f	*.o *.s

################################################################################
# Declare the contents of the .PHONY variable as phony.
.PHONY: $(PHONY)


################################################################################
######## End of file ###########################################################
################################################################################
