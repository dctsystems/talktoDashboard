

//Main MQTT Class
import org.eclipse.paho.client.mqttv3.*;

//System stuff 
import java.util.Vector;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;

//Drawing Stuff
import java.awt.Color;
import java.awt.Stroke;
import java.awt.BasicStroke;
import java.awt.Paint;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Rectangle;
import java.awt.Graphics2D;
import java.awt.image.BufferedImage;
import javax.imageio.ImageIO;
import java.io.File;


public class JavaDemo implements MqttCallback
{

  MqttClient client;
  File tmpFile;
  File realFile;

  public JavaDemo() {}

  // Gets a string representing the pid of this program - Java VM
  public static String getPid() throws IOException,InterruptedException {
     
    //Yes - this really is as ugly as it looks
    //Java really doesn't make this easy
    Vector<String> commands=new Vector<String>();
    commands.add("/bin/bash");
    commands.add("-c");
    commands.add("echo $PPID");
    ProcessBuilder pb=new ProcessBuilder(commands);
     
    Process pr=pb.start();
    pr.waitFor();
    if (pr.exitValue()==0) {
      BufferedReader outReader=new BufferedReader(new InputStreamReader(pr.getInputStream()));
      return outReader.readLine().trim();
    } else {
      System.out.println("Error while getting PID");
      return "";
    }
  }



  public void messageArrived(String topic, MqttMessage message) throws Exception
	{
	//System.out.println (topic + " " + new String (message.getPayload()));

	BufferedImage img = new BufferedImage(320, 180, BufferedImage.TYPE_INT_RGB);


	//Grab the graphics object off the image
	Graphics2D graphics = img.createGraphics();

	graphics.setPaint(Color.blue);
	graphics.fill(new Rectangle(0,0,320,180));
	graphics.setPaint(Color.white);
	graphics.draw(new Rectangle(0,0,319,179));

	Font font=new Font( "FreeSans", Font.PLAIN, 24 );
	graphics.setFont(font);
	FontMetrics metrics=graphics.getFontMetrics(font);
	int offset=metrics.getAscent();

	graphics.drawString("Topic:"+topic, 20,50+offset);
	graphics.drawString("Message:"+new String (message.getPayload()), 20,80+offset);


	ImageIO.write(img, "jpg", tmpFile);
	tmpFile.renameTo(realFile);
	}

  public void connectionLost (Throwable cause) {}
  public void deliveryComplete(IMqttDeliveryToken token) {}

  public static void main (String[] args) throws IOException,InterruptedException {
    new JavaDemo().startMQTT();
  }

  public void startMQTT() throws IOException,InterruptedException {
    try {

	String myPid=getPid();
	tmpFile=new File("/tmp/LIVE/X"+myPid+".jpg");
	realFile=new File("/tmp/LIVE/"+myPid+".jpg");


      client = new MqttClient("tcp://dashboard.local.:1883", MqttClient.generateClientId());
      client.setCallback(this);
      client.connect();
      client.subscribe("#");

      // We’ll now idle here sleeping, but your app can be busy
      // working here instead

      while (true)
		{
	      try { Thread.sleep (1000); } catch (InterruptedException e) {}
	      }
    }
    catch (MqttException e) { e.printStackTrace (); }
  }
}

