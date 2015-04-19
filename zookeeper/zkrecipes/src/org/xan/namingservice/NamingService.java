package org.xan.namingservice;

import java.io.IOException;
import java.util.List;
import java.util.concurrent.CountDownLatch;

import org.apache.zookeeper.CreateMode;
import org.apache.zookeeper.KeeperException;
import org.apache.zookeeper.WatchedEvent;
import org.apache.zookeeper.Watcher;
import org.apache.zookeeper.Watcher.Event.KeeperState;
import org.apache.zookeeper.ZooDefs.Ids;
import org.apache.zookeeper.ZooKeeper;

/**
 * TODO：什么是命名服务？我不是很清楚，没有经历过实际场景。
 * 根据想象和资料（http://www.ibm.com/developerworks/cn/opensource/os-cn-zookeeper/index.html#major3），理解如下：
 * 1. 节点A想创建某个名字，但需要知道是否跟其他节点冲突，如冲突则创建失败，否则创建成功
 * 根据这个理解，实现十分简单的工程如下。 
 * @author xan
 */
public class NamingService implements Watcher {
    private static final int SESSION_TIMEOUT = 5000;
    private ZooKeeper zk;
    private CountDownLatch connectedSignal = new CountDownLatch(1);
    private String namingRoot;
    private String namingServer; // one of zookeeper servers, TODO: IP address parsing
    
    public NamingService(String namingRoot, String namingServer) {
        this.namingRoot = "/" + namingRoot;
        this.namingServer = namingServer;
    }
    
    public boolean registerName(String name) {
        try {
            connect(); // TODO: 设想registerName()的调用频率低，从而每次都执行对zookeeper的连接和断连
            String absolutePath = namingRoot + "/" + name;
            zk.create(absolutePath, null, Ids.OPEN_ACL_UNSAFE, CreateMode.PERSISTENT);
        } catch (IllegalArgumentException | InterruptedException | IOException e) {
            System.err.println(e.getMessage());
            return false;
        } catch (KeeperException e) {
            // special case:
            // zke.getCode() == KeeperException.NodeExistsException
            System.err.println(e.getMessage());
            return false;
        } finally {
            closeQuietly();
        }
        return true;
    }
    
    public boolean unregisterName(String name) {
        try {
            connect();
            String absolutePath = namingRoot + "/" + name;
            zk.delete(absolutePath, -1);
        } catch (IOException | InterruptedException e) {
            System.err.println(e.getMessage());
            return false;
        } catch (KeeperException e) {
            // special case
            // e.getCode() == KeeperException.NoNodeException
            System.err.println(e.getMessage());
            return false;
        } finally {
            closeQuietly();
        }
        return true;
    }
    
    public List<String> readAllNames() {
        List<String> names = null;
        try {
            connect();
            names = zk.getChildren(namingRoot, false);
        } catch (IOException | InterruptedException e) {
            System.err.println(e.getMessage());
            return null;
        } catch (KeeperException e) {
            System.err.println(e.getMessage());
            return null;
        } finally {
            closeQuietly();
        }
        return names;
    }
    
    private void connect() throws IOException, InterruptedException {
        zk = new ZooKeeper(namingServer, SESSION_TIMEOUT, this);
        connectedSignal.await();
    }
    
    private void closeQuietly() {
        try {
            zk.close();
        } catch (InterruptedException e) {
            // TODO: 特别的场景，是否会造成系统中无用的socket handle? 如何处理才最合适?
            System.err.println(e.getMessage());
        }
    }

    /**
     * TODO: 想办法删除这个函数（因为其public特性在这里碍眼）? 相反在connect()中等待SESSION_TIMEOUT再执行后续操作
     */
    @Override
    public void process(WatchedEvent event) {
        if (event.getState() == KeeperState.SyncConnected)
            connectedSignal.countDown();
    }
}
