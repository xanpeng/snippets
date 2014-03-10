package org.xan.namingservice;

public class NamingServiceSample {
    public static void Main(String[] args) {
        NamingService namingService = new NamingService("sampleNamingRoot", args[0]);
        boolean flag = namingService.registerName("foo");
        assert(flag == true);
        flag = namingService.registerName("foo");
        assert(flag == false);
        flag = namingService.unregisterName("foo");
        assert(flag == true);
        flag = namingService.unregisterName("foo");
        assert(flag == false);
    }
}
