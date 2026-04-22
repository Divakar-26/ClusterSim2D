all:
	g++ src/*.cpp \
	   src/glad.c \
	   imgui/imgui.cpp \
	   imgui/imgui_draw.cpp \
	   imgui/imgui_tables.cpp \
	   imgui/imgui_widgets.cpp \
	   imgui/backends/imgui_impl_sdl3.cpp \
	   imgui/backends/imgui_impl_opengl3.cpp \
	   -I/usr/local/include \
	   -Iinclude \
	   -Iimgui \
	   -Iimgui/backends \
	   -L/usr/local/lib \
	   -lSDL3 \
	   -lGL \
	   -o app
