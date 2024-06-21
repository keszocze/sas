EXT_SYMM_SRC := src/ext-sas/src
SRC += \
    $(EXT_SYMM_SRC)/init.cpp \
    $(EXT_SYMM_SRC)/componentwise.cpp \
    $(EXT_SYMM_SRC)/wae_factors.cpp \
    \
    $(EXT_SYMM_SRC)/aig/circuits.cpp \
    $(EXT_SYMM_SRC)/aig/network.cpp \
    $(EXT_SYMM_SRC)/aig/symmetric.cpp \
    \
    $(EXT_SYMM_SRC)/bdd/bdd.cpp \
    $(EXT_SYMM_SRC)/bdd/ch.cpp \
    $(EXT_SYMM_SRC)/bdd/storage.cpp \
    $(EXT_SYMM_SRC)/bdd/symmetric.cpp \
    \
    $(EXT_SYMM_SRC)/commands/commands.cpp \
    $(EXT_SYMM_SRC)/commands/common.cpp \
    $(EXT_SYMM_SRC)/commands/gbdd.cpp \
    $(EXT_SYMM_SRC)/commands/netgen.cpp \
    $(EXT_SYMM_SRC)/commands/symmetrize.cpp \
    \
    $(EXT_SYMM_SRC)/utils/truth_table.cpp \