TARGET = 1

MOC_DIR = 0__mocs

OBJECTS_DIR = 0__mocs

QT = core \
     network \
     widgets \
     svg

HEADERS = kernel/kernel.h \
          login/login.h \
          account/account.h \
          account/account_widgets/account_widgets.h \
          tables/holdem/holdem.h \
          tables/holdem/holdem_widgets/holdem_widgets.h \
          tables/stud/stud.h \
          tables/stud/stud_widgets.h \
          util_widgets/svg_button.h \
          rapidjson/document.h \
          rapidjson/writer.h \
          rapidjson/stringbuffer.h

SOURCES = main.cpp \
          kernel/kernel.cpp \
          login/login.cpp \
          account/account.cpp \
          tables/holdem/holdem.cpp \
          tables/holdem/holdem_parsers.cpp \
          tables/holdem/holdem_widgets/holdem_widgets.cpp \
          tables/stud/stud.cpp
