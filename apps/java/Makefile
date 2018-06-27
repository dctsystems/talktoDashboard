all : JavaDemo.class
	java -cp mqtt-client-0.4.0.jar:. JavaDemo

JavaDemo.class : JavaDemo.java
	javac -cp mqtt-client-0.4.0.jar:jna-4.5.1.jar  JavaDemo.java

clean :
	rm *.class
