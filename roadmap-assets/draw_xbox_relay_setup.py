# SPDX-License-Identifier: Apache-2.0
# Compose wiring guidance using existing supplier photos; no invented hardware photos.
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
root=Path(__file__).resolve().parent.parent
im=Image.new('RGB',(1800,2200),'#f3f6fb');d=ImageDraw.Draw(im)
font='/System/Library/Fonts/Supplemental/Arial.ttf'
def txt(x,y,s,n=25,c='#24324a'):d.text((x,y),s,font=ImageFont.truetype(font,n),fill=c)
def lines(x,y,ss,n=25,step=35):
 for i,s in enumerate(ss):txt(x,y+i*step,s,n)
def box(x,y,w,h,title,color='white'):
 d.rounded_rectangle((x,y,x+w,y+h),18,fill=color,outline='#bccbdd',width=2);txt(x+20,y+15,title,30)
def pic(path,x,y,w,h):
 a=Image.open(root/path).convert('RGB');a.thumbnail((w,h));im.paste(a,(x+(w-a.width)//2,y+(h-a.height)//2))
def arr(points,c='#456386'):
 d.line(points,fill=c,width=6);x,y=points[-1];px,py=points[-2]
 if x>px:d.polygon([(x,y),(x-16,y-9),(x-16,y+9)],fill=c)
 elif y>py:d.polygon([(x,y),(x-9,y-16),(x+9,y-16)],fill=c)
txt(40,25,'Xbox remote control: POWER + EJECT',45)
txt(40,88,'Codex agent → ESP API → two independent relays → real button contacts',30)
txt(40,135,'PROPOSED: firmware/API work, live GPIO audit and installer verification still required.',24,'#a04b15')
box(40,195,1720,110,'1. Agent-commanded buttons','#eaf3ff')
txt(65,250,'PRESS POWER or PRESS EJECT — either can request startup; neither is an ON/OFF state-set command.',26)
box(40,350,570,475,'2. Existing ESP-KVM header')
pic('docs/esp32-p4x-function-ev-board-isometric_v1.6.png',60,405,530,220)
lines(65,645,['J1-36 / GPIO46 → POWER signal','J1-37 / GPIO47 → EJECT signal','J1-39 / GND → control ground junction','Push-on female sockets; audio stays wired.'],24)
box(690,350,490,475,'3. POWER relay')
box(1270,350,490,475,'4. EJECT relay')
for x,gpio in [(690,'GPIO46'),(1270,'GPIO47')]:
 pic('roadmap-assets/product-photos/adafruit-4409.jpg',x+20,410,450,215)
 lines(x+25,645,['Adafruit #4409 + #3894 cable',f'White SIG ← {gpio}','Red V+ ← external 5 V','Black GND ← control ground'],23)
arr([(610,475),(690,475)])
arr([(610,540),(650,540),(650,325),(1515,325),(1515,350)])
box(40,880,570,370,'5. One independent 5 V supply','#eaf3ff')
pic('roadmap-assets/product-photos/adafruit-276.jpg',60,940,260,180)
pic('roadmap-assets/product-photos/adafruit-368.jpg',340,960,240,130)
lines(65,1140,['#276 supply + #368 jack, proposed','Insulated +5 V / GND split to both relays.','Supply negative also joins ESP GND.'],23)
arr([(325,880),(325,850),(935,850),(935,825)],'#d43a40')
arr([(935,850),(1515,850),(1515,825)],'#d43a40')
for x,label in [(690,'POWER'),(1270,'EJECT')]:
 box(x,880,490,370,f'6. {label} contact disconnect','#ecf8f1')
 pic('roadmap-assets/product-photos/adafruit-2880.jpg',x+20,945,450,160)
 lines(x+25,1130,['#2880 keyed pair, one per button','NO → button signal; COM → return','Label both halves; NC stays unused.'],23)
 arr([(x+245,825),(x+245,880)],'#138467')
box(40,1310,1720,400,'7. Installer connects both Xbox buttons')
pic('roadmap-assets/x3-power-eject-adapter-closeup.png',60,1370,420,280)
lines(510,1380,['Option A: plug-in pass-through adapter, like historical Xecuter X3 (photo).','Option B: professional soldered leads on verified front-button contacts.','Both: preserve original buttons/LEDs; insulate joints and add strain relief.','','POWER pair → across POWER switch     EJECT pair → across EJECT switch','Verify contacts, connector fit/orientation and each button independently.','No direct ESP GPIO, 5 V or control-ground wire to Xbox button contacts.'],25)
arr([(935,1250),(935,1310)],'#138467');arr([(1515,1250),(1515,1310)],'#138467')
box(40,1765,1720,215,'8. Hard-freeze recovery: Shelly switches Xbox AC only','#eaf3ff')
pic('roadmap-assets/product-photos/shelly-us-gen4.jpg',60,1820,190,140)
lines(290,1830,['Cut AC → confirm off / wait → restore AC → check fresh power state.', 'If Xbox remains off: default to one POWER pulse, then verify a fresh boot.', 'Keep ESP-KVM, relay supply and network powered independently.'],28)
box(40,2025,1720,110,'Button commands and confirmation','#fff4e8')
txt(65,2080,'EJECT is agent-commanded (not physical-only); Stellar eject-start may choose a different boot mode.',26)
txt(40,2160,'Photos: Adafruit / Shelly / historical X3 guide p.12; ESP reference: Espressif. Not to scale. No verified Xbox solder-pad map.',21)
im.save(root/'roadmap-assets/xbox-power-relay-setup.png')
