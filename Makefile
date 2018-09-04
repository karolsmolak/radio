
CXX := g++

CXXFLAGS += -c -Wall -O2 -std=c++17

LIBFLAGS := -pthread -lboost_program_options

EXE := sikradio-sender
EXE1 := sikradio-receiver

SENDERHEADERDIR := sender/include
RECEIVERHEADERDIR := receiver/include

SENDERSRCDIR := sender/src
RECEIVERSRCDIR := receiver/src

OBJDIR := obj

BINDIR := .

SENDERHEADERS := $(wildcard $(SENDERHEADERDIR)/*.h)
SENDERSRCS := $(wildcard $(SENDERSRCDIR)/*.cpp)
SENDEROBJS := $(subst $(SENDERSRCDIR)/,$(OBJDIR)/sender/,$(SENDERSRCS:.cpp=.o))

RECEIVERHEADERS := $(wildcard $(RECEIVERHEADERDIR)/*.h)
RECEIVERSRCS := $(wildcard $(RECEIVERSRCDIR)/*.cpp)
RECEIVEROBJS := $(subst $(RECEIVERSRCDIR)/,$(OBJDIR)/receiver/,$(RECEIVERSRCS:.cpp=.o))

LDFLAGS += $(LIBFLAGS)

.PHONY: all clean distclean

all: $(BINDIR)/$(EXE) $(BINDIR)/$(EXE1)

$(BINDIR)/$(EXE): $(SENDEROBJS) $(OBJDIR)/messages.o $(OBJDIR)/err.o
	mkdir -p $(BINDIR)
	$(CXX) $? -o $@ $(LDFLAGS)

$(BINDIR)/$(EXE1): $(RECEIVEROBJS) $(OBJDIR)/messages.o $(OBJDIR)/err.o
	mkdir -p $(BINDIR)
	$(CXX) $? -o $@ $(LDFLAGS)

$(OBJDIR)/err.o: utils/err.cpp
	mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -Iutils $< -o $@

$(OBJDIR)/messages.o: messages/messages.cpp
	mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -Imessages $< -o $@

$(OBJDIR)/sender/%.o: $(SENDERSRCDIR)/%.cpp
	mkdir -p $(OBJDIR)/sender
	$(CXX) $(CXXFLAGS) -I$(SENDERHEADERDIR) $< -o $@

$(OBJDIR)/receiver/%.o: $(RECEIVERSRCDIR)/%.cpp
	mkdir -p $(OBJDIR)/receiver
	$(CXX) $(CXXFLAGS) -I$(RECEIVERHEADERDIR) $< -o $@

clean:
	rm -rf $(OBJDIR) $(BINDIR)/$(EXE) $(BINDIR)/$(EXE1)

distclean: clean
