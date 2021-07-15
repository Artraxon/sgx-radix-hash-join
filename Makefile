
PROJECT_NAME		:= cahj-bin
App_Name			:= hashj-app
Enclave_Name 		:= enclave.so
Signed_Enclave_Name := enclave.signed.so
Enclave_Config_File := src/enclave/Enclave.config.xml
ROOT_DIR			:= $(shell pwd)

########################################

MPI_FOLDER			= /usr
COMPILER_FLAGS 		= -O3 -std=c++0x -mavx -lpthread -lpapi -D MEASUREMENT_DETAILS_HISTOGRAM -D MEASUREMENT_DETAILS_NETWORK -D MEASUREMENT_DETAILS_LOCALPART -D MEASUREMENT_DETAILS_LOCALBP
PAPI_FOLDER			= /bin
CC					= $(MPI_FOLDER)/bin/mpic++
CXX					= $(CC)
MPICXX				= $(MPI_FOLDER)/bin/mpic++

########################################

SOURCE_FOLDER		= src
BUILD_FOLDER		= build
RELEASE_FOLDER		= release
ENCLAVE_FOLDER 		= enclave

########################################
			

#INCLUDE_ENCLAVE 	:= $(dir $(wildcard src/enclave/**/*.h))
######## SGX SDK Settings ########

SGX_MODE ?= HW
SGX_ARCH ?= x64
SGX_DEBUG ?= 1

include $(SGX_SDK)/buildenv.mk

ifeq ($(shell getconf LONG_BIT), 32)
	SGX_ARCH := x86
else ifeq ($(findstring -m32, $(CXXFLAGS)), -m32)
	SGX_ARCH := x86
endif

ifeq ($(SGX_ARCH), x86)
	SGX_COMMON_FLAGS := -m32
	SGX_LIBRARY_PATH := $(SGX_SDK)/lib
	SGX_ENCLAVE_SIGNER := $(SGX_SDK)/bin/x86/sgx_sign
	SGX_EDGER8R := $(SGX_SDK)/bin/x86/sgx_edger8r
else
	SGX_COMMON_FLAGS := -m64
	SGX_LIBRARY_PATH := $(SGX_SDK)/lib64
	SGX_ENCLAVE_SIGNER := $(SGX_SDK)/bin/x64/sgx_sign
	SGX_EDGER8R := $(SGX_SDK)/bin/x64/sgx_edger8r
endif

ifeq ($(SGX_DEBUG), 1)
ifeq ($(SGX_PRERELEASE), 1)
$(error Cannot set SGX_DEBUG and SGX_PRERELEASE at the same time!!)
endif
endif

ifeq ($(SGX_DEBUG), 1)
        SGX_COMMON_FLAGS += -O0 -g
else
        SGX_COMMON_FLAGS += -O2
endif

SGX_COMMON_FLAGS += -Wall -Wextra -Winit-self -Wpointer-arith -Wreturn-type \
                    -Waddress -Wsequence-point -Wformat-security \
                    -Wmissing-include-dirs -Wfloat-equal -Wundef -Wshadow \
                    -Wcast-align -Wcast-qual -Wconversion -Wredundant-decls
SGX_COMMON_CFLAGS := $(SGX_COMMON_FLAGS) -Wjump-misses-init -Wstrict-prototypes -Wunsuffixed-float-constants -v
SGX_COMMON_CXXFLAGS := $(SGX_COMMON_FLAGS) -Wnon-virtual-dtor -std=c++11


######## app Settings ########

ifneq ($(SGX_MODE), HW)
	Urts_Library_Name := sgx_urts_sim
else
	Urts_Library_Name := sgx_urts
endif

App_Cpp_Files :=  $(shell find ./src/untrusted -name "*.cpp")
App_C_Files :=  $(shell find ./src/untrusted -name "*.c")
App_Include_Paths := -I$(SGX_SDK)/include \
					 -I/usr/include/c++/9 \
					 -I/usr/lib/x86_64-linux-gnu/openmpi/include \
					 -Igenerated/untrusted

App_C_Flags := -fPIC -fopenmp -Wno-attributes $(App_Include_Paths) -DOMPI_IGNORE_CXX_SEEK
App_C_Flags += $(CFLAGS)

# Three configuration modes - Debug, prerelease, release
#   Debug - Macro DEBUG enabled.
#   Prerelease - Macro NDEBUG and EDEBUG enabled.
#   Release - Macro NDEBUG enabled.
ifeq ($(SGX_DEBUG), 1)
        App_C_Flags += -DDEBUG -UNDEBUG -UEDEBUG
else ifeq ($(SGX_PRERELEASE), 1)
        App_C_Flags += -DNDEBUG -DEDEBUG -UDEBUG
else
        App_C_Flags += -DNDEBUG -UEDEBUG -UDEBUG
endif

App_Cpp_Flags := $(App_C_Flags)
App_Link_Flags := -L$(SGX_LIBRARY_PATH) -l$(Urts_Library_Name) -lpthread \
				  -lgomp \

App_Cpp_Objects := $(App_Cpp_Files:src/%.cpp=build/%.o)
App_Cpp_Objects += $(App_C_Files:src/%.c=build/%.o)


App_Name := app

######## Enclave Settings ########
ifneq ($(SGX_MODE), HW)
	Trts_Library_Name := sgx_trts_sim
	Service_Library_Name := sgx_tservice_sim
else
	Trts_Library_Name := sgx_trts
	Service_Library_Name := sgx_tservice
endif
Crypto_Library_Name := sgx_tcrypto

Enclave_Cpp_Files :=  $(shell find ./src/enclave -name "*.cpp")
Enclave_C_Files :=  $(shell find ./src/enclave -name "*.c")

Enclave_Include_Paths := -I$(SGX_SDK)/include -I$(SGX_SDK)/include/tlibc -I$(SGX_SDK)/include/libcxx -I generated/trusted
# $(addprefix -I,$(INCLUDE_ENCLAVE)) \

Enclave_C_Flags := $(Enclave_Include_Paths) $(CFLAGS) -nostdinc -fvisibility=hidden -fpie -ffunction-sections \
					-fdata-sections $(MITIGATION_CFLAGS) -fopenmp -Wno-missing-field-initializers
CC_BELOW_4_9 := $(shell expr "`$(CC) -dumpversion`" \< "4.9")
ifeq ($(CC_BELOW_4_9), 1)
	Enclave_C_Flags += -fstack-protector
else
	Enclave_C_Flags += -fstack-protector-strong
endif

Enclave_Cpp_Flags := $(Enclave_C_Flags) -nostdinc++

# Enable the security flags
Enclave_Security_Link_Flags := -Wl,-z,relro,-z,now,-z,noexecstack

# To generate a proper enclave, it is recommended to follow below guideline to link the trusted libraries:
#    1. Link sgx_trts with the `--whole-archive' and `--no-whole-archive' options,
#       so that the whole content of trts is included in the enclave.
#    2. For other libraries, you just need to pull the required symbols.
#       Use `--start-group' and `--end-group' to link these libraries.
# Do NOT move the libraries linked with `--start-group' and `--end-group' within `--whole-archive' and `--no-whole-archive' options.
# Otherwise, you may get some undesirable errors.
Enclave_Link_Flags := $(Enclave_Security_Link_Flags) \
    -Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -L$(SGX_TRUSTED_LIBRARY_PATH) \
	-Wl,--whole-archive -l$(Trts_Library_Name) -Wl,--no-whole-archive \
	-Wl,--whole-archive -lsgx_tcmalloc -Wl,--no-whole-archive \
	-Wl,--start-group  -lsgx_tstdc -lsgx_tcxx -lsgx_omp -lsgx_pthread -lsgx_tkey_exchange -l$(Crypto_Library_Name) \
					   -l$(Service_Library_Name) -Wl,--end-group \
	-Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined \
	-Wl,-pie,-eenclave_entry -Wl,--export-dynamic  \
	-Wl,--defsym,__ImageBase=0 -Wl,--gc-sections   \
	-Wl,--version-script=Enclave/Enclave.lds

Enclave_Cpp_Objects := $(sort $(Enclave_Cpp_Files:.cpp=.o))
Enclave_Cpp_Objects += $(sort $(Enclave_C_Files:.c=.o))

Enclave_Name := enclave.so
Signed_Enclave_Name := enclave.signed.so
Enclave_Config_File := Enclave/Enclave.config.xml

ifeq ($(SGX_MODE), HW)
ifeq ($(SGX_DEBUG), 1)
	Build_Mode = HW_DEBUG
else ifeq ($(SGX_PRERELEASE), 1)
	Build_Mode = HW_PRERELEASE
else
	Build_Mode = HW_RELEASE
endif
else
ifeq ($(SGX_DEBUG), 1)
	Build_Mode = SIM_DEBUG
else ifeq ($(SGX_PRERELEASE), 1)
	Build_Mode = SIM_PRERELEASE
else
	Build_Mode = SIM_RELEASE
endif
endif

########################################

OBJECT_FILES		= $(patsubst $(SOURCE_FOLDER)/%.cpp,$(BUILD_FOLDER)/%.o,$(ENCLAVE_SOURCE_FILES))
SOURCE_DIRECTORIES	= $(dir $(App_Cpp_Files) $(App_C_Files) $(Enclave_Cpp_Files) $(Enclave_C_Files))
BUILD_DIRECTORIES	= $(patsubst $(SOURCE_FOLDER)/%,$(BUILD_FOLDER)/%,$(SOURCE_DIRECTORIES))

########################################

all: program

.PHONY: all target run
all: sgx

ifeq ($(Build_Mode), HW_RELEASE)
sgx: .config_$(Build_Mode)_$(SGX_ARCH) $(App_Name) $(Enclave_Name)
	@echo "The project has been built in release hardware mode."
	@echo "Please sign the $(Enclave_Name) first with your signing key before you run the $(App_Name) to launch and access the enclave."
	@echo "To sign the enclave use the command:"
	@echo "   $(SGX_ENCLAVE_SIGNER) sign -key <your key> -enclave $(Enclave_Name) -out <$(Signed_Enclave_Name)> -config $(Enclave_Config_File)"
	@echo "You can also sign the enclave using an external signing tool."
	@echo "To build the project in simulation mode set SGX_MODE=SIM. To build the project in prerelease mode set SGX_PRERELEASE=1 and SGX_MODE=HW."


else
sgx: .config_$(Build_Mode)_$(SGX_ARCH) $(App_Name) $(App_Name) $(Signed_Enclave_Name)
ifeq ($(Build_Mode), HW_DEBUG)
	@echo $(CXX)
	@echo "The project has been built in debug hardware mode."
else ifeq ($(Build_Mode), SIM_DEBUG)
	@echo "The project has been built in debug simulation mode."
else ifeq ($(Build_Mode), HW_PRERELEASE)
	@echo "The project has been built in pre-release hardware mode."
else ifeq ($(Build_Mode), SIM_PRERELEASE)
	@echo "The project has been built in pre-release simulation mode."
else
	@echo "The project has been built in release simulation mode."
endif

endif

run: sgx

ifneq ($(Build_Mode), HW_RELEASE)
	@$(CURDIR)/$(App_Name)
	@echo "RUN  =>  $(App_Name) [$(SGX_MODE)|$(SGX_ARCH), OK]"
endif

########################################

#Untrusted Part
.config_$(Build_Mode)_$(SGX_ARCH):
	@rm -f .config_* $(App_Name) $(Enclave_Name) $(Signed_Enclave_Name) $(App_Cpp_Objects) app/Enclave_u.* $(Enclave_Cpp_Objects) Enclave/Enclave_t.*
	@touch .config_$(Build_Mode)_$(SGX_ARCH)


generated/untrusted/Enclave_u.h: $(SGX_EDGER8R) src/enclave/Enclave.edl
	@cd generated/untrusted && $(SGX_EDGER8R) --untrusted $(ROOT_DIR)/src/enclave/Enclave.edl --search-path $(ROOT_DIR)/src/enclave$(SOURCE_FOLDER)/$(ENCLAVE_FOLDER) --search-path $(SGX_SDK)/include
	@echo "GEN  =>  $@"

generated/untrusted/Enclave_u.c: generated/untrusted/Enclave_u.h

generated/untrusted/Enclave_u.o: generated/untrusted/Enclave_u.c
	$(CC) $(SGX_COMMON_CFLAGS) $(App_Cpp_Flags) $(COMPILER_FLAGS) -c $< -o $@
	@echo "CC   <=  $<"

build/untrusted/%.o: src/untrusted/%.cpp generated/untrusted/Enclave_u.h
	mkdir -p $(@D)
	$(MPI_FOLDER)/bin/mpic++ $(SGX_COMMON_CXXFLAGS) $(App_Cpp_Flags) $(COMPILER_FLAGS) -c $< -I $(ROOT_DIR)/src/untrusted -I $(PAPI_FOLDER) -I $(ROOT_DIR)/generated/untrusted -o $@

build/untrusted/%.o: src/untrusted/%.c generated/untrusted/Enclave_u.h
	mkdir -p $(@D)
	$(MPI_FOLDER)/bin/mpic++ $(SGX_COMMON_CXXFLAGS) $(App_Cpp_Flags) $(COMPILER_FLAGS) -c $< -I $(ROOT_DIR)/src/untrusted -I $(PAPI_FOLDER) -I $(ROOT_DIR)/generated/untrusted -o $@

$(App_Name): generated/untrusted/Enclave_u.o $(App_Cpp_Objects)
	@echo "Enter: LINK =>  $@"
	@echo "CPP Files: $(App_Cpp_Files)"
	$(CXX) $^ -o $@ $(App_Link_Flags)
	@echo "LINK =>  $@"


######## Enclave Objects ########

generated/trusted/Enclave_t.h: $(SGX_EDGER8R) src/enclave/Enclave.edl
	@cd generated/trusted && $(SGX_EDGER8R) --trusted $(ROOT_DIR)/src/enclave/Enclave.edl --search-path $(ROOT_DIR)/src/enclave$(SOURCE_FOLDER)/$(ENCLAVE_FOLDER) --search-path $(SGX_SDK)/include
	@echo "GEN  =>  $@"

generated/trusted/Enclave_t.c: generated/trusted/Enclave_t.h

generated/trusted/Enclave_t.o: generated/trusted/Enclave_t.c
	@$(CC) $(SGX_COMMON_CFLAGS) $(Enclave_C_Flags) -c $< -o $@
	@echo "CC   <=  $<"

src/enclave/%.o: src/enclave/%.c generated/trusted/Enclave_t.h
	mkdir -p $(@D)
	$(CXX) $(SGX_COMMON_CXXFLAGS) $(Enclave_Cpp_Flags) -c $< -o $@ -I $(ROOT_DIR)/src/enclave -I /usr/include/x86_64-linux-gnu -I /usr/include -I /usr/lib/x86_64-linux-gnu/openmpi/include -DOMPI_IGNORE_CXX_SEEK
	@echo "CXX  <=  $<"

#Enclave/%.o: Enclave/%.cpp Enclave/Enclave_t.h
src/enclave/%.o: src/enclave/%.cpp generated/trusted/Enclave_t.h
	mkdir -p $(@D)
	$(CXX) $(SGX_COMMON_CXXFLAGS) $(Enclave_Cpp_Flags) -c $< -o $@ -I $(ROOT_DIR)/src/enclave -I /usr/include/x86_64-linux-gnu -I /usr/include -I /usr/lib/x86_64-linux-gnu/openmpi/include -DOMPI_IGNORE_CXX_SEEK
	@echo "CXX  <=  $<"

$(Enclave_Name): generated/trusted/Enclave_t.o $(Enclave_Cpp_Objects)
	@$(CXX) $^ -o $@ $(Enclave_Link_Flags)
	@echo "LINK =>  $@"

$(Signed_Enclave_Name): $(Enclave_Name)
	@$(SGX_ENCLAVE_SIGNER) sign -key src/enclave/Enclave_private_test.pem -enclave $(Enclave_Name) -out $@ -config $(Enclave_Config_File)
	@echo "SIGN =>  $@"
########################################

program: $(OBJECT_FILES)
	mkdir -p $(RELEASE_FOLDER)
	$(MPI_FOLDER)/bin/mpic++ $(OBJECT_FILES) $(COMPILER_FLAGS) -L $(PAPI_FOLDER) -o $(RELEASE_FOLDER)/$(PROJECT_NAME)
	make public
	

########################################

clean:
	rm -rf $(BUILD_FOLDER)
	rm -rf $(RELEASE_FOLDER)
	@rm -f .config_* $(App_Name) $(Enclave_Name) $(Signed_Enclave_Name) $(App_Cpp_Objects) app/Enclave_u.* $(Enclave_Cpp_Objects) Enclave/Enclave_t.*

########################################

public:
	chmod 777 -R $(BUILD_FOLDER)
	chmod 777 -R $(RELEASE_FOLDER)
