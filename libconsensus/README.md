## TOC

* Introduction
* How to build
* 2-Phase Commit
* Paxos
* consensus (original explanation from cmccabe)

### Introduction

Project originally forked from https://github.com/cmccabe/consensus-fun

### How to build

```
# cd consensus
# mkdir build && cd build
# cmake ..
# ./bin/test_2pc -v
```

### 2-Phase Commit

Single-proposer case is simple. Here talk about multiple-proposer case.  

Default to 5 nodes simulated by threads, and nodes 2,0,1 propose.  
If proposal is not accepted and committed, it will be sent again but delayed for random()%100 milliseconds to simulate real circumstance. 

```
# bin/test_2pc -v > test2pc.out 2>&1

# cat test2pc.out | grep "^0"
0: sent MMM_PROPOSE to node 1
0: sent MMM_PROPOSE to node 2
0: sent MMM_PROPOSE to node 3
0: sent MMM_PROPOSE to node 4
0: rejected proposal from 2
0: rejected proposal from 1
0: got ACK from 3
0: got NACK from 2
0: got ACK from 4
0: got NACK from 1
0: sending ABORT to everyone
0: received ABORT from 2
0: received ABORT from 1
0: sent MMM_PROPOSE to node 1
0: sent MMM_PROPOSE to node 2
0: sent MMM_PROPOSE to node 3
0: sent MMM_PROPOSE to node 4
0: got ACK from 1
0: got ACK from 2
0: got ACK from 3
0: got ACK from 4
0: sending COMMIT to everyone

# cat test2pc.out | grep "^1"
1: sent MMM_PROPOSE to node 0
1: sent MMM_PROPOSE to node 2
1: sent MMM_PROPOSE to node 3
1: sent MMM_PROPOSE to node 4
1: rejected proposal from 0
1: rejected proposal from 2
1: got NACK from 2
1: got NACK from 0
1: got NACK from 3
1: received ABORT from 0
1: received ABORT from 2
1: got NACK from 4
1: sending ABORT to everyone
1: accepted proposal from 0
1: ignoring do_propose

# cat test2pc.out | grep "^2"
2: sent MMM_PROPOSE to node 0
2: sent MMM_PROPOSE to node 1
2: sent MMM_PROPOSE to node 3
2: sent MMM_PROPOSE to node 4
2: rejected proposal from 0
2: rejected proposal from 1
2: got NACK from 0
2: received ABORT from 0
2: got NACK from 3
2: got NACK from 4
2: got NACK from 1
2: sending ABORT to everyone
2: received ABORT from 1
2: accepted proposal from 0
2: ignoring do_propose

# cat test2pc.out | grep "^3"
3: accepted proposal from 0
3: rejected proposal from 1
3: rejected proposal from 2
3: received ABORT from 2
3: received ABORT from 1
3: accepted proposal from 0

# cat test2pc.out | grep "^4"
4: accepted proposal from 0
4: rejected proposal from 2
4: rejected proposal from 1
4: received ABORT from 2
4: received ABORT from 1
4: accepted proposal from 0
```

### paxos

```
# bin/test_paxos -v > testpaxos.out 2>&1 

# cat testpaxos.out | grep "^0"
0: sent MMM_PROPOSE to node 1
0: sent MMM_PROPOSE to node 2
0: sent MMM_PROPOSE to node 3
0: sent MMM_PROPOSE to node 4
0: giving up on proposing because node 2 had a proposal with a higher sequence number
0: rejected proposal (mprop->leader = 1, mprop->seen_pseq = 0x101) from 1 (me->leader = -1, me->seen_pseq = 0x102)
0: got MMM_REJECT message from 1, but we are not proposing.
0: got MMM_REJECT message from 2, but we are not proposing.
0: got MMM_REJECT message from 3, but we are not proposing.
0: got MMM_REJECT message from 4, but we are not proposing.
0: accepted leader 2
0: sending commit ACK back to 2

# cat testpaxos.out | grep "^1"
1: sent MMM_PROPOSE to node 2
1: sent MMM_PROPOSE to node 3
1: sent MMM_PROPOSE to node 4
1: rejected proposal (mprop->leader = 0, mprop->seen_pseq = 0x100) from 0 (me->leader = -1, me->seen_pseq = 0x101)
1: giving up on proposing because node 2 had a proposal with a higher sequence number
1: got MMM_ACCEPT message from 4, but we are not proposing.
1: got MMM_REJECT message from 2, but we are not proposing.
1: got MMM_ACCEPT message from 3, but we are not proposing.
1: got MMM_REJECT message from 0, but we are not proposing.
1: accepted leader 2
1: sending commit ACK back to 2

# cat testpaxos.out | grep "^2"
2: sent MMM_PROPOSE to node 0
2: sent MMM_PROPOSE to node 1
2: sent MMM_PROPOSE to node 3
2: sent MMM_PROPOSE to node 4
2: rejected proposal (mprop->leader = 0, mprop->seen_pseq = 0x100) from 0 (me->leader = -1, me->seen_pseq = 0x102)
2: rejected proposal (mprop->leader = 1, mprop->seen_pseq = 0x101) from 1 (me->leader = -1, me->seen_pseq = 0x102)
2: got MMM_ACCEPT (prop_leader=1) from 1
2: got MMM_ACCEPT (prop_leader=1) from 4
2: got MMM_ACCEPT (prop_leader=0) from 0
2: sending COMMIT (prop_leader=2, prop_pseq=0x102) message to 0
2: sending COMMIT (prop_leader=2, prop_pseq=0x102) message to 1
2: sending COMMIT (prop_leader=2, prop_pseq=0x102) message to 3
2: sending COMMIT (prop_leader=2, prop_pseq=0x102) message to 4
2: got MMM_ACCEPT message from 3, but we are not proposing.
2: accepted leader 2
2: considering protocol terminated
2: got MMM_COMMIT_RESP from 4, but we are not commiting any more.

# cat testpaxos.out | grep "^3"
3: rejected proposal (mprop->leader = 0, mprop->seen_pseq = 0x100) from 0 (me->leader = -1, me->seen_pseq = 0x101)
3: accepted leader 2
3: sending commit ACK back to 2

# cat testpaxos.out | grep "^4"
4: rejected proposal (mprop->leader = 0, mprop->seen_pseq = 0x100) from 0 (me->leader = -1, me->seen_pseq = 0x102)
4: accepted leader 2
4: sending commit ACK back to 2
```

### consensus (original explanation from cmccabe)

This is a sample implementation of some distributed consensus protocols.

Currently implemented:
* Two-phase commit
* Paxos

In both of these tests, the nodes vote on which node should be the "leader" of
the cluster.  Nodes are represented by operating system threads.  One node
sending a message to another node is represented by putting the message into a
queue and posting a semaphore.

Nodes can send messages with a delay value.  This means that the messages will
be delivered after a timeout period has elapsed.  For example, a node could
send a delayed message to itself in order to implement a timeout.  Nodes can
also put delays on messages they send to other nodes in order to stress-test
the implementation a bit.

TODO:
* stress-test implementation by injecting message delays
* model and test node failure for Paxos

This is licensed under the Apache License 2.0.  See LICENSE for details.

Have fun.

Colin Patrick McCabe
cmccabe@alumni.cmu.edu
