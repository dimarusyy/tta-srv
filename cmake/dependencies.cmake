#boost
set(Boost_NO_BOOST_CMAKE ON)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS system REQUIRED)

# doctest
find_package(doctest CONFIG REQUIRED)

#toml++
find_package(tomlplusplus CONFIG REQUIRED)

#spdlog
find_package(spdlog CONFIG REQUIRED)
