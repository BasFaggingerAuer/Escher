#Copyright 2012, Bas Fagginger Auer.
#
#This program is free software: you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation, either version 3 of the License, or
#(at your option) any later version.
#
#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with this program.  If not, see <http://www.gnu.org/licenses/>.
cmake_minimum_required(VERSION 2.6)
project(ESCHER)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ESCHER_SOURCE_DIR})

#set(CMAKE_CXX_FLAGS "-O2 -g -Wall -Wextra -Wshadow -ansi -pedantic")
set(CMAKE_CXX_FLAGS "-O3 -Wall -ffast-math -DNDEBUG -DLEAN")

find_package(SDL REQUIRED)

add_subdirectory(${ESCHER_SOURCE_DIR}/src/)

