##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=add
ConfigurationName      :=Debug
WorkspacePath          := "/home/tom/crypto/code/ce_crypto"
ProjectPath            := "/home/tom/crypto/code/ce_crypto/add"
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=TOM
Date                   :=19/11/15
CodeLitePath           :="/home/tom/.codelite"
LinkerName             :=gcc
SharedObjectLinkerName :=gcc -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.o.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=$(IntermediateDirectory)/$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E 
ObjectsFileList        :="add.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            :=  -lgmp -lgmpxx -lstdc++
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). 
IncludePCH             := 
RcIncludePath          := 
Libs                   := 
ArLibs                 :=  
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := ar rcus
CXX      := gcc
CC       := gcc
CXXFLAGS :=  -g -O0 -Wall $(Preprocessors)
CFLAGS   :=  -g -O0 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/main.cpp$(ObjectSuffix) $(IntermediateDirectory)/CE_point.cpp$(ObjectSuffix) $(IntermediateDirectory)/CE_curve.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

MakeIntermediateDirs:
	@test -d ./Debug || $(MakeDirCommand) ./Debug


$(IntermediateDirectory)/.d:
	@test -d ./Debug || $(MakeDirCommand) ./Debug

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/main.cpp$(ObjectSuffix): main.cpp $(IntermediateDirectory)/main.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tom/crypto/code/ce_crypto/add/main.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/main.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/main.cpp$(DependSuffix): main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/main.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/main.cpp$(DependSuffix) -MM "main.cpp"

$(IntermediateDirectory)/main.cpp$(PreprocessSuffix): main.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/main.cpp$(PreprocessSuffix) "main.cpp"

$(IntermediateDirectory)/CE_point.cpp$(ObjectSuffix): CE_point.cpp $(IntermediateDirectory)/CE_point.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tom/crypto/code/ce_crypto/add/CE_point.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/CE_point.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/CE_point.cpp$(DependSuffix): CE_point.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/CE_point.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/CE_point.cpp$(DependSuffix) -MM "CE_point.cpp"

$(IntermediateDirectory)/CE_point.cpp$(PreprocessSuffix): CE_point.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/CE_point.cpp$(PreprocessSuffix) "CE_point.cpp"

$(IntermediateDirectory)/CE_curve.cpp$(ObjectSuffix): CE_curve.cpp $(IntermediateDirectory)/CE_curve.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/tom/crypto/code/ce_crypto/add/CE_curve.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/CE_curve.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/CE_curve.cpp$(DependSuffix): CE_curve.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/CE_curve.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/CE_curve.cpp$(DependSuffix) -MM "CE_curve.cpp"

$(IntermediateDirectory)/CE_curve.cpp$(PreprocessSuffix): CE_curve.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/CE_curve.cpp$(PreprocessSuffix) "CE_curve.cpp"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Debug/


