find_all_sources(. linux_SRCS)
if(NOT TARGET_COMMON_LINUX)
	list(REMOVE_ITEM linux_SRCS ./DBusMessage.cpp)
	list(REMOVE_ITEM linux_SRCS ./DBusReserve.cpp)
	list(REMOVE_ITEM linux_SRCS ./DBusUtil.cpp)
	list(REMOVE_ITEM linux_SRCS ./HALManager.cpp)
endif(NOT TARGET_COMMON_LINUX)

list(REMOVE_ITEM linux_SRCS ./OMXClock.cpp ./OMXCore.cpp ./RBP.cpp)

add_sources(${linux_SRCS})
