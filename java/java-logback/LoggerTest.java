import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

//
// put logback.xml into classpath
//
public class LoggerTest {
  private static final Logger logger = LoggerFactory.getLogger(LoggerTest.class);
  public static void main(String[] args) {
    logger.info("logging in LoggerTest");
  }
}

