#! /usr/bin/make -f

# dependencies

OBJS		=							\
		$(TMP_DIR)/label.o					\
		$(TMP_DIR)/main.o					\
		$(TMP_DIR)/parse.o

ALL	= $(BIN_NAME) size


# target: dependencies
#	action

PHONY := all
all: $(ALL)


$(BIN_NAME): $(OBJS)
	@echo	"	CXX	$@"
	$(Q)$(CXX) $(OBJS) -o $@ $(LIBS)

size: $(BIN_NAME)
	@echo	"	SZ	$(BIN_NAME)"
	$(Q)$(SZ) $(BIN_NAME)


clean:
	@echo	"	RM	$(ALL)"
	$(Q)rm -f	$(ALL)

################################################################################
# Declare the contents of the .PHONY variable as phony.
.PHONY: $(PHONY)

################################################################################
######## End of file ###########################################################
################################################################################
