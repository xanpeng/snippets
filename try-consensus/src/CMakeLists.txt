SET(CXX_FLAGS
  -g
  -fpermissive
  )

SET(SRC_2PC test_2pc.cc util.cc worker.cc util.h worker.h tree.h)
SET(SRC_PAXOS test_paxos.cc util.cc worker.cc util.h worker.h tree.h)

ADD_EXECUTABLE(test_2pc ${SRC_2PC})
ADD_EXECUTABLE(test_paxos ${SRC_PAXOS})

# muduo_base depends on pthread, so order matters here
TARGET_LINK_LIBRARIES(test_2pc muduo_base pthread)
TARGET_LINK_LIBRARIES(test_paxos muduo_base pthread)
