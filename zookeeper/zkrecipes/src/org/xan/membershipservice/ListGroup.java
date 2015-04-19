package org.xan.membershipservice;
import java.util.List;

import org.apache.zookeeper.KeeperException;


public class ListGroup extends ConnectionWatcher {
    public void list(String groupName) throws KeeperException, InterruptedException {
        String path = "/" + groupName;
        List<String> children = zk.getChildren(path, false);
        if (children.isEmpty()) {
            System.out.printf("No members in group %s\n", groupName);
            System.exit(1);
        }
        for (String child : children)
            System.out.println(child);
    }
    
    public static void main(String[] args) throws Exception {
        ListGroup listGroup = new ListGroup();
        listGroup.connect(args[0]);
        listGroup.list(args[1]);
        listGroup.close();
    }
}
