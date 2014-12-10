// jsch example, copied from http://stackoverflow.com/questions/2405885/any-good-jsch-examples
package com.lenovo.cloud.sim.tool;
 
import java.io.IOException;
import java.io.InputStream;
 
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
 
import com.jcraft.jsch.Channel;
import com.jcraft.jsch.ChannelExec;
import com.jcraft.jsch.JSch;
import com.jcraft.jsch.JSchException;
import com.jcraft.jsch.Session;
 
public class SSHManager {
  private static final Logger logger = LoggerFactory.getLogger(SSHManager.class);
  
  private JSch jschSSHChannel;
  private String strUserName;
  private String strPassword;
  private String strConnectionIP;
  private int intConnPort;
  private Session sessionConn;
  private int intTimeOut;
 
  public SSHManager(String userName, String password, String connectionIP,
      String knownHostsFileName) {
    doCommonConstructorActions(userName, password, connectionIP,
        knownHostsFileName);
    intConnPort = 22;
    intTimeOut = 60000;
  }
 
  public SSHManager(String userName, String password, String connectionIP,
      String knownHostsFileName, int connectionPort) {
    doCommonConstructorActions(userName, password, connectionIP,
        knownHostsFileName);
    intConnPort = connectionPort;
    intTimeOut = 60000;
  }
 
  public SSHManager(String userName, String password, String connectionIP,
      String knownHostsFileName, int connectionPort,
      int timeOutMilliseconds) {
    doCommonConstructorActions(userName, password, connectionIP,
        knownHostsFileName);
    intConnPort = connectionPort;
    intTimeOut = timeOutMilliseconds;
  }
 
  public String connect() {
    String errorMessage = null;
 
    try {
      sessionConn = jschSSHChannel.getSession(strUserName,
          strConnectionIP, intConnPort);
      // password-less login
      // sessionConn.setPassword(strPassword);
      // UNCOMMENT THIS FOR TESTING PURPOSES, BUT DO NOT USE IN PRODUCTION
      sessionConn.setConfig("StrictHostKeyChecking", "no");
      sessionConn.connect(intTimeOut);
    } catch (JSchException jschX) {
      errorMessage = jschX.getMessage();
    }
 
    return errorMessage;
  }
 
  public String sendCommand(String command) {
    StringBuilder outputBuffer = new StringBuilder();
 
    try {
      Channel channel = sessionConn.openChannel("exec");
      ((ChannelExec) channel).setCommand(command);
      channel.connect();
      InputStream commandOutput = channel.getInputStream();
      int readByte = commandOutput.read();
 
      while (readByte != 0xffffffff) {
        outputBuffer.append((char) readByte);
        readByte = commandOutput.read();
      }
 
      channel.disconnect();
    } catch (IOException ioX) {
      logger.warn(ioX.getMessage());
      return null;
    } catch (JSchException jschX) {
      logger.warn(jschX.getMessage());
      return null;
    }
 
    return outputBuffer.toString();
  }
 
  public void close() {
    sessionConn.disconnect();
  }
 
  private void doCommonConstructorActions(String userName, String password,
      String connectionIP, String knownHostsFileName) {
    jschSSHChannel = new JSch();
 
    try {
      jschSSHChannel.setKnownHosts(knownHostsFileName);
      jschSSHChannel.addIdentity(System.getProperty("user.home") + "/.ssh/id_rsa");
    } catch (JSchException jschX) {
      logger.error(jschX.getMessage());
    }
 
    strUserName = userName;
    strPassword = password;
    strConnectionIP = connectionIP;
  }
 
    public static void main(String[] args) {
    System.out.println("sendCommand");
 
    String command = "cat /etc/hosts";
    String userName = "root";
    String password = "Philly518";
    String connectionIP = "10.100.211.68";
    SSHManager instance = new SSHManager(userName, password, connectionIP, "");
    String errorMessage = instance.connect();
 
    if (errorMessage != null) {
      System.out.println(errorMessage);
      exit(-1);
    }
 
    // call sendCommand for each command and the output
    // (without prompts) is returned
    System.out.println(instance.sendCommand(command));
    // close only after all commands are sent
    instance.close();
  }
}
