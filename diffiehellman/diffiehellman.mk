##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=diffiehellman
ConfigurationName      :=Debug
WorkspacePath          := "/home/djataga/workspace/security/project/irm5-ec_crypto"
ProjectPath            := "/home/djataga/workspace/security/project/irm5-ec_crypto/diffiehellman"
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Djataga Rodrigue OUOBA
Date                   :=08/01/16
CodeLitePath           :="/home/djataga/.codelite"
LinkerName             :=/usr/bin/g++
SharedObjectLinkerName :=/usr/bin/g++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.i
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
ObjectsFileList        :="diffiehellman.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            :=  -lgmp -lgmpxx -lstdc++ -lcrypto -lpthread
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
AR       := /usr/bin/ar rcu
CXX      := /usr/bin/g++
CC       := /usr/bin/gcc
CXXFLAGS :=  -g -O0 -Wall $(Preprocessors)
CFLAGS   :=  -g -O0 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/utils.cpp$(ObjectSuffix) $(IntermediateDirectory)/sha256.cpp$(ObjectSuffix) $(IntermediateDirectory)/CE_point.cpp$(ObjectSuffix) $(IntermediateDirectory)/CE_curve.cpp$(ObjectSuffix) $(IntermediateDirectory)/AES.cpp$(ObjectSuffix) $(IntermediateDirectory)/main.cpp$(ObjectSuffix) $(IntermediateDirectory)/ECDSA.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

$(IntermediateDirectory)/.d:
	@test -d ./Debug || $(MakeDirCommand) ./Debug

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/utils.cpp$(ObjectSuffix): utils.cpp $(IntermediateDirectory)/utils.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/djataga/workspace/security/project/irm5-ec_crypto/diffiehellman/utils.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/utils.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/utils.cpp$(DependSuffix): utils.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/utils.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/utils.cpp$(DependSuffix) -MM "utils.cpp"

$(IntermediateDirectory)/utils.cpp$(PreprocessSuffix): utils.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/utils.cpp$(PreprocessSuffix) "utils.cpp"

$(IntermediateDirectory)/sha256.cpp$(ObjectSuffix): sha256.cpp $(IntermediateDirectory)/sha256.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/djataga/workspace/security/project/irm5-ec_crypto/diffiehellman/sha256.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/sha256.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/sha256.cpp$(DependSuffix): sha256.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/sha256.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/sha256.cpp$(DependSuffix) -MM "sha256.cpp"

$(IntermediateDirectory)/sha256.cpp$(PreprocessSuffix): sha256.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/sha256.cpp$(PreprocessSuffix) "sha256.cpp"

$(IntermediateDirectory)/CE_point.cpp$(ObjectSuffix): CE_point.cpp $(IntermediateDirectory)/CE_point.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/djataga/workspace/security/project/irm5-ec_crypto/diffiehellman/CE_point.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/CE_point.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/CE_point.cpp$(DependSuffix): CE_point.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/CE_point.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/CE_point.cpp$(DependSuffix) -MM "CE_point.cpp"

$(IntermediateDirectory)/CE_point.cpp$(PreprocessSuffix): CE_point.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/CE_point.cpp$(PreprocessSuffix) "CE_point.cpp"

$(IntermediateDirectory)/CE_curve.cpp$(ObjectSuffix): CE_curve.cpp $(IntermediateDirectory)/CE_curve.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/djataga/workspace/security/project/irm5-ec_crypto/diffiehellman/CE_curve.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/CE_curve.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/CE_curve.cpp$(DependSuffix): CE_curve.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/CE_curve.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/CE_curve.cpp$(DependSuffix) -MM "CE_curve.cpp"

$(IntermediateDirectory)/CE_curve.cpp$(PreprocessSuffix): CE_curve.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/CE_curve.cpp$(PreprocessSuffix) "CE_curve.cpp"

$(IntermediateDirectory)/AES.cpp$(ObjectSuffix): AES.cpp $(IntermediateDirectory)/AES.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/djataga/workspace/security/project/irm5-ec_crypto/diffiehellman/AES.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/AES.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/AES.cpp$(DependSuffix): AES.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/AES.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/AES.cpp$(DependSuffix) -MM "AES.cpp"

$(IntermediateDirectory)/AES.cpp$(PreprocessSuffix): AES.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/AES.cpp$(PreprocessSuffix) "AES.cpp"

$(IntermediateDirectory)/main.cpp$(ObjectSuffix): main.cpp $(IntermediateDirectory)/main.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/djataga/workspace/security/project/irm5-ec_crypto/diffiehellman/main.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/main.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/main.cpp$(DependSuffix): main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/main.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/main.cpp$(DependSuffix) -MM "main.cpp"

$(IntermediateDirectory)/main.cpp$(PreprocessSuffix): main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/main.cpp$(PreprocessSuffix) "main.cpp"

$(IntermediateDirectory)/ECDSA.cpp$(ObjectSuffix): ECDSA.cpp $(IntermediateDirectory)/ECDSA.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/djataga/workspace/security/project/irm5-ec_crypto/diffiehellman/ECDSA.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ECDSA.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ECDSA.cpp$(DependSuffix): ECDSA.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ECDSA.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ECDSA.cpp$(DependSuffix) -MM "ECDSA.cpp"

$(IntermediateDirectory)/ECDSA.cpp$(PreprocessSuffix): ECDSA.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ECDSA.cpp$(PreprocessSuffix) "ECDSA.cpp"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Debug/


