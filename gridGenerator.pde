import processing.pdf.*;

String [] chars = { "Q", "F", "T", "I", "S", "H", "Y", "I", "N", "F", "U", "L", "W", "N", "S", "M", "T", "U", "I", "O", "A", "A", "E", "O", "E", "Q", "N", "F", "N", "U", "R", "H", "L", "O", "N", "T", "E", "O", "E", "R", "T", "A", "V", "E", "C", "E", "W", "W", "T", "V", "E", "P", "E", "S", "E", "L", "T", "T", "S", "W", "R", "T", "O", "B", "E", "R", "O", "K", "A", "Y", "E", "I", "G", "H", "T", "V", "H", "C", "P", "S", "E", "L", "E", "V", "E", "N", "E", "T", "K", "I", "F", "I", "V", "E", "I", "T", "F", "N", "R", "X"};

float xOffset = 260.5;
float yOffset = 260.5;

float xSpacing = 166;
float ySpacing = 166;

int rowWidth = 10;

int start = 0;
int stop = 0;

void setup() {
  size(2015, 2015, PDF, "output.pdf");
  background(0);
  PFont myFont = createFont("courbd.ttf", 1);
  textFont(myFont);
  textAlign(CENTER, CENTER);
  rectMode(CORNERS);
  textSize(160);
  noStroke();
}

void draw(){
  fill(255);
  for (int i= 0; i<chars.length; i++) {
    println(((i%rowWidth))*xSpacing+xOffset, ((i/rowWidth))*ySpacing+yOffset);
    text(chars[i], ((i%rowWidth))*xSpacing+xOffset, ((i/rowWidth))*ySpacing+yOffset);
  }
  println("DONE");
  exit();
}
