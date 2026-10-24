# SPDX-License-Identifier: Apache-2.0
from PIL import Image, ImageDraw, ImageFont
from pathlib import Path
root=Path(__file__).resolve().parent.parent
W,H=1800,1720
im=Image.new('RGB',(W,H),'#f5f7fb'); d=ImageDraw.Draw(im)
font='/System/Library/Fonts/Supplemental/Arial.ttf'
def f(n): return ImageFont.truetype(font,n)
def text(x,y,s,n=25,c='#233047'): d.text((x,y),s,font=f(n),fill=c)
def box(x,y,w,h,title,lines=(),color='#ffffff'):
 d.rounded_rectangle((x,y,x+w,y+h),18,fill=color,outline='#ccd4e1',width=2)
 text(x+20,y+15,title,29)
 for i,s in enumerate(lines): text(x+20,y+58+i*33,s,24)
def line(pts,c='#64748b',width=5): d.line(pts,fill=c,width=width,joint='curve')
def dot(x,y,c): d.ellipse((x-7,y-7,x+7,y+7),fill=c)
blue='#2563eb'; red='#d43a40'; black='#334155'; green='#138467'
text(45,30,'One ESP-KVM, two interchangeable button accessories',42)
text(45,87,'PROPOSED WIRING • Power-cycle recovery after a hard freeze • Not installation-ready',25,'#a54b13')
box(45,145,520,235,'1. ESP expansion header: J1',[
 'GPIO46 = physical pin 36 → CONTROL',
 'GND = physical pin 39 → GROUND',
 'Push female Dupont sockets onto pins.',
 'No soldering to the ESP board.' ])
box(45,405,520,147,'Independent accessory power',[
 'Regulated 5 V supply, sized for servo.',
 'Its negative joins ESP ground.' ])
box(940,150,790,165,'2. Common removable accessory plug',[
 'Three circuits: CONTROL / +5 V / GROUND',
 'Use a keyed, latching plug with premade wires.',
 'Physical connector pin order is not finalized.' ])
# Distinct three circuits into plug; supply deliberately outside ESP
line([(565,232),(690,232),(690,205),(940,205)],blue)
text(710,169,'CONTROL',22,blue)
line([(565,307),(640,307),(640,355),(850,355),(850,280),(940,280)],black)
line([(565,500),(640,500),(640,355)],black); dot(640,355,black)
line([(565,460),(800,460),(800,243),(940,243)],red)
text(655,422,'+5 V from supply',22,red)
text(665,365,'Shared ground',22,black)
text(965,340,'ESP 3.3 V pin is NOT connected to servo power.',26,'#a54b13')
text(965,381,'The servo has 3 wires; this source harness uses',24)
text(965,413,'2 ESP header sockets + external supply wiring.',24)
text(965,475,'Choose A OR B. Power off the accessory before swapping.',25)
text(965,510,'Firmware must match the attached accessory.',25)
# A
box(45,600,825,655,'A. External servo finger — no Xbox soldering',color='#edf5ff')
box(80,675,340,125,'Accessory plug',[
 'CONTROL   +5 V   GROUND',
 'Three wires to servo' ])
box(525,675,300,125,'Positional servo',[
 '3.3 V-compatible signal',
 '5 V-rated selected model' ])
line([(420,720),(525,720)],blue);line([(420,745),(525,745)],red);line([(420,770),(525,770)],black)
# physical arm and button
box(80,875,745,190,'Xbox exterior',[
 'Bracket holds the servo beside the power button.',
 'The arm briefly presses, then retracts.',
 'No electrical cable goes from servo into Xbox.' ])
d.rounded_rectangle((150,1110,400,1205),10,fill='#334155')
text(178,1138,'SERVO',28,'white')
line([(400,1155),(545,1155),(545,1190)],green,14)
d.ellipse((520,1198,570,1248),fill='#138467')
text(590,1198,'Power button',24)
text(80,810,'Wire colors here identify circuits; verify actual servo pinout.',21)
# B
box(915,600,840,655,'B. Internal relay — electrically presses button',color='#eef8f3')
box(950,675,330,125,'Accessory plug',[
 'CONTROL   +5 V   GROUND',
 'Goes to driver electronics' ])
box(1350,675,365,125,'Isolated relay interface',[
 '5 V supply / 3.3 V control',
 'Final module still to select' ])
line([(1280,720),(1350,720)],blue);line([(1280,745),(1350,745)],red);line([(1280,770),(1350,770)],black)
text(955,823,'Isolation: only relay COM + NO contacts connect to Xbox.',23,green)
box(950,880,765,140,'Preferred: plug-in Xbox button adapter',[
 'Relay contacts → two-wire lead → verified inline adapter.',
 'Original button cable plugs through; Stellar stays installed.' ])
line([(1510,800),(1510,865),(1455,865),(1455,880)],green)
text(950,1045,'Alternative: two soldered leads across POWER switch',25)
# switch abstract inset
line([(1000,1145),(1110,1145)],green)
line([(1000,1205),(1240,1205),(1240,1145)],green)
d.ellipse((1100,1135,1120,1155),fill=green);d.ellipse((1230,1135,1250,1155),fill=green)
line([(1110,1145),(1210,1105)],black,5)
text(1300,1100,'Concept only:',23,'#a54b13')
text(1300,1134,'NOT an Xbox pad map.',23,'#a54b13')
text(1300,1168,'Actual solder points',23)
text(1300,1200,'are not yet verified.',23)
# footer
box(45,1290,1710,145,'3. AC power cycle is separate from the button accessory',[
 'Wall outlet → Shelly metering plug → Xbox AC cord only.',
 'KVM + accessory supply stay independently powered. Button press happens AFTER confirmed AC restoration.' ])
box(45,1465,1710,160,'What this diagram does not yet establish',[
 'GPIO46 must be checked against live settings. Existing audio GPIO22 / GPIO53 / GPIO27 stays installed.',
 'Exact revision-1.4 solder pads need board identification/continuity checks; do not solder using this sketch.',
 'Power-state sensing on GPIO48 is a separate circuit. Neither accessory proves the Xbox has booted.' ],color='#fff5e9')
text(45,1655,'Signal: blue     Accessory +5 V: red     Ground: dark gray     Isolated button contacts / physical action: green',23)
im.save(root/'roadmap-assets/xbox-power-accessories.png')
