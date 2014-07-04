package org.xan.membershipservice;
import java.io.IOException;
import java.util.concurrent.CountDownLatch;

import org.apache.zookeeper.WatchedEvent;
import org.apache.zookeeper.Watcher;
import org.apache.zookeeper.ZooKeeper;
import org.apache.zookeeper.Watcher.Event.KeeperState;


public class ConnectionWatcher implements Watcher {
    private static final int SESSION_TIMEOUT = 5000;
    private CountDownLatch connectedSignal = new CountDownLatch(1);
    protected ZooKeeper zk;
    
    public void connect(String hosts) throws IOException, InterruptedException {
        zk = new ZooKeeper(hosts, SESSION_TIMEOUT, this);
        connectedSignal.await();
    }
    
    @Override
    public void process(WatchedEvent event) {
        if (event.getState() == KeeperState.SyncConnected)
            connectedSignal.countDown();
    }
    
    public void close() throws InterruptedException {
        zk.close();
    }
    
}
