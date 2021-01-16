#boost
set(Boost_NO_BOOST_CMAKE ON)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS system coroutine filesystem REQUIRED)

# doctest
find_package(doctest CONFIG REQUIRED)

#toml++
find_package(tomlplusplus CONFIG REQUIRED)

#sqlite_orm
find_package(SqliteOrm CONFIG REQUIRED)

#sqlitecpp
find_package(SQLiteCpp CONFIG REQUIRED)

#spdlog
find_package(spdlog CONFIG REQUIRED)

if(MSVC)
add_definitions(/MP)
add_definitions(/bigobj)
endif(MSVC)