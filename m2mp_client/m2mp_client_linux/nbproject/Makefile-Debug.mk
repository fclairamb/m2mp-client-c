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
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/1360937237/logging.o \
	${OBJECTDIR}/_ext/1360937237/m2mp_client.o \
	${OBJECTDIR}/_ext/1360937237/str.o \
	${OBJECTDIR}/_ext/1360937237/memwatcher.o \
	${OBJECTDIR}/_ext/1360937237/m2mp_client_settings.o \
	${OBJECTDIR}/_ext/1360937237/m2mp_client_files.o \
	${OBJECTDIR}/_ext/1360937237/dictionnary.o \
	${OBJECTDIR}/_ext/1360937237/m2mp_client_commands.o \
	${OBJECTDIR}/_ext/1360937237/linkedlist.o


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
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libm2mp_client_linux.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libm2mp_client_linux.a: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libm2mp_client_linux.a
	${AR} -rv ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libm2mp_client_linux.a ${OBJECTFILES} 
	$(RANLIB) ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libm2mp_client_linux.a

${OBJECTDIR}/_ext/1360937237/logging.o: ../src/logging.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} $@.d
	$(COMPILE.c) -Werror -DDEBUG -I../src -I. -I. -I. -I. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1360937237/logging.o ../src/logging.c

${OBJECTDIR}/_ext/1360937237/m2mp_client.o: ../src/m2mp_client.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} $@.d
	$(COMPILE.c) -Werror -DDEBUG -I../src -I. -I. -I. -I. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1360937237/m2mp_client.o ../src/m2mp_client.c

${OBJECTDIR}/_ext/1360937237/str.o: ../src/str.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} $@.d
	$(COMPILE.c) -Werror -DDEBUG -I../src -I. -I. -I. -I. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1360937237/str.o ../src/str.c

${OBJECTDIR}/_ext/1360937237/memwatcher.o: ../src/memwatcher.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} $@.d
	$(COMPILE.c) -Werror -DDEBUG -I../src -I. -I. -I. -I. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1360937237/memwatcher.o ../src/memwatcher.c

${OBJECTDIR}/_ext/1360937237/m2mp_client_settings.o: ../src/m2mp_client_settings.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} $@.d
	$(COMPILE.c) -Werror -DDEBUG -I../src -I. -I. -I. -I. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1360937237/m2mp_client_settings.o ../src/m2mp_client_settings.c

${OBJECTDIR}/_ext/1360937237/m2mp_client_files.o: ../src/m2mp_client_files.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} $@.d
	$(COMPILE.c) -Werror -DDEBUG -I../src -I. -I. -I. -I. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1360937237/m2mp_client_files.o ../src/m2mp_client_files.c

${OBJECTDIR}/_ext/1360937237/dictionnary.o: ../src/dictionnary.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} $@.d
	$(COMPILE.c) -Werror -DDEBUG -I../src -I. -I. -I. -I. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1360937237/dictionnary.o ../src/dictionnary.c

${OBJECTDIR}/_ext/1360937237/m2mp_client_commands.o: ../src/m2mp_client_commands.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} $@.d
	$(COMPILE.c) -Werror -DDEBUG -I../src -I. -I. -I. -I. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1360937237/m2mp_client_commands.o ../src/m2mp_client_commands.c

${OBJECTDIR}/_ext/1360937237/linkedlist.o: ../src/linkedlist.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} $@.d
	$(COMPILE.c) -Werror -DDEBUG -I../src -I. -I. -I. -I. -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1360937237/linkedlist.o ../src/linkedlist.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libm2mp_client_linux.a

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
