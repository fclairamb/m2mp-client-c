#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/1360937237/main.o


# C Compiler Flags
CFLAGS=-g

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=../../m2mp_client/m2mp_client_linux/dist/Debug/GNU-Linux-x86/libm2mp_client_linux.a

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/m2mp_client_test

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/m2mp_client_test: ../../m2mp_client/m2mp_client_linux/dist/Debug/GNU-Linux-x86/libm2mp_client_linux.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/m2mp_client_test: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.c} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/m2mp_client_test ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/_ext/1360937237/main.o: ../src/main.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} $@.d
	$(COMPILE.c) -Werror -DDEBUG -I../../m2mp_client/src -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1360937237/main.o ../src/main.c

# Subprojects
.build-subprojects:
	cd ../../m2mp_client/m2mp_client_linux && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/m2mp_client_test

# Subprojects
.clean-subprojects:
	cd ../../m2mp_client/m2mp_client_linux && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
