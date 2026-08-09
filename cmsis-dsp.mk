CMSIS_DSP = $(LIBDAISY_DIR)/Drivers/CMSIS-DSP

C_INCLUDES += \
	-I$(CMSIS_DSP)/Include \
	-I$(CMSIS_DSP)/PrivateInclude

C_SOURCES += \
	$(CMSIS_DSP)/Source/BasicMathFunctions/BasicMathFunctions.c \
	$(CMSIS_DSP)/Source/CommonTables/CommonTables.c \
	$(CMSIS_DSP)/Source/InterpolationFunctions/InterpolationFunctions.c \
	$(CMSIS_DSP)/Source/MatrixFunctions/MatrixFunctions.c \
	$(CMSIS_DSP)/Source/ComplexMathFunctions/ComplexMathFunctions.c \
	$(CMSIS_DSP)/Source/FastMathFunctions/FastMathFunctions.c \
	$(CMSIS_DSP)/Source/SupportFunctions/SupportFunctions.c \
	$(CMSIS_DSP)/Source/FilteringFunctions/FilteringFunctions.c \
	$(CMSIS_DSP)/Source/TransformFunctions/TransformFunctions.c \
	$(CMSIS_DSP)/Source/WindowFunctions/WindowFunctions.c

#$(CMSIS_DSP)/Source/BayesFunctions/BayesFunctions.c \
#$(CMSIS_DSP)/Source/QuaternionMathFunctions/QuaternionMathFunctions.c \
#$(CMSIS_DSP)/Source/ControllerFunctions/ControllerFunctions.c \
#$(CMSIS_DSP)/Source/SVMFunctions/SVMFunctions.c \
#$(CMSIS_DSP)/Source/DistanceFunctions/DistanceFunctions.c \
#$(CMSIS_DSP)/Source/StatisticsFunctions/StatisticsFunctions.c \ 

C_DEFS += -DARM_CORTEX

CFLAGS += -Wsign-compare
CFLAGS += -Wdouble-promotion
CFLAGS += -Ofast -ffast-math
CFLAGS += -DNDEBUG
CFLAGS += -Wall -Wextra -Werror
CFLAGS += -fshort-enums -fshort-wchar
CFLAGS += -mfloat-abi=hard