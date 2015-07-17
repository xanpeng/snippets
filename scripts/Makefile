### clone from https://github.com/nybuxtsui/Makefile/blob/master/Makefile
### 一个通用makefile框架
# ?= 如果不存在，则赋值
# := 不展开变量
# += 追加
# = 展开变量

TARGET ?= a.out
SRCDIR ?= src
ifeq ($(OS), Windows_NT) 
	CFLAGS += -DWIN32 -D_WIN32
	LIBS += -lws2_32
else
	LIBS += -L../boost/stage/lib -lboost_system -lboost_thread -lpthread -lrt
endif
INCS += -I../boost
LIBS +=
MYLIBS +=

OBJDIR ?= obj

SRCS := $(foreach dir, $(SRCDIR), $(wildcard $(dir)/*.cpp))
OBJS := $(addprefix $(OBJDIR)/, $(patsubst %.cpp, %.o, $(SRCS)))
DEPS := $(addprefix $(OBJDIR)/, $(patsubst %.cpp, %.d, $(SRCS)))

MAKE_OBJECT_DIR := $(shell mkdir -p $(addprefix $(OBJDIR)/, $(SRCDIR)))

CC := gcc
CXX := g++
AR := ar

CFLAGS := -Wall 
CXXFLAGS := -D__cplusplus
ARFLAGS := r

DEBUGFLAGS := -g -O0 -D_DEBUG
RELEASEFLAGS := -g -O2 -DNODEBUG

ifdef RELEASE
	CFLAGS += $(RELEASEFLAGS) 
else    
	CFLAGS += $(DEBUGFLAGS) 
endif  


.PHONY: all gch clean cleanall dep test run

all: $(TARGET)

gch:
	@echo Creating precompiled header...
	@$(CXX) $(CFLAGS) $(CXXFLAGS) stdheaders.h $(INCS)

# $@ : 代表规则中的目标文件名
# $% : The target member name, when the target is an archive member
# $< : 规则的第一个依赖文件名
# $? : 所有比目标文件更新的依赖文件列表，空格分割
# $^ : 规则的所有依赖文件列表，使用空格分隔
# $+ : 类似“$^”，但是它保留了依赖文件中重复出现的文件

$(TARGET): $(OBJS) $(MYLIBS)
	@echo Linking [$@]
	@$(CXX) $(CFLAGS) $(CXXFLAGS) $(OBJS) -o $@ $(MYLIBS) $(LIBS)
#@echo Archiving [$@]
#@$(AR) $(ARFLAGS) $@ $^


$(OBJDIR)/%.o: %.cpp $(OBJDIR)/%.d
	@echo Compiling [$<]
	@$(CXX) $(CFLAGS) $(CXXFLAGS) -c $< -o $@ $(INCS)

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),cleanall)
ifneq ($(MAKECMDGOALS),gch)
-include $(DEPS)
endif
endif
endif

$(OBJDIR)/%.d: %.cpp
	@echo Making depends file [$<]
	@printf $(OBJDIR)/$(dir $<) > $@
	@$(CXX) $(CFLAGS) $(CXXFLAGS) -MM $< >> $@ $(INCS)

clean:
	@echo Removing depends files
	@rm -rf $(DEPS)
	@echo Removing object files
	@rm -rf $(OBJS)
	@echo Removing target files
	@rm -f $(TARGET)

cleanall:
	@echo Removing depends files
	@rm -rf $(DEPS)
	@echo Removing object files
	@rm -rf $(OBJS)
	@echo Removing target files
	@rm -f $(TARGET)
	@echo Removing precompiled header 
	@rm -f stdheaders.h.gch
