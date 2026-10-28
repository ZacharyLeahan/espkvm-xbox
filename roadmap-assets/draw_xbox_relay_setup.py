# SPDX-License-Identifier: Apache-2.0
# Compose wiring guidance using existing supplier photos; no invented hardware photos.
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
root=Path(__file__).resolve().parent.parent
im=Image.new('RGB',(1800,2390),'#f3f6fb');d=ImageDraw.Draw(im)
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
txt(40,25,'Existing ESP-KVM: wired POWER + EJECT',45)
txt(40,88,'XWiFi-style button control — isolated contacts; no extra Pico, XERC or IR',30)
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
 box(x,880,490,370,f'6. {label} contact pair','#ecf8f1')
 pic('roadmap-assets/product-photos/adafruit-2880.jpg',x+20,945,450,160)
 lines(x+25,1130,['Photo: #2880 BACKUP disconnect','Preferred: pair through 4-pin port','NO / COM only; NC stays unused.'],23)
 arr([(x+245,825),(x+245,880)],'#138467')
box(40,1310,1720,400,'7. Installer connects both Xbox buttons')
lines(65,1380,['SELECTED: solder two wires across POWER and two across EJECT on front button board.',
                  'Secure / insulate leads → #5319 sidewall socket <-> #5318 plug → round 4-wire cable.',
                  'External cable splits into POWER pair → POWER relay NO/COM; EJECT pair → EJECT relay NO/COM.',
                  'Installer assembles port + cable; owner connects relays / shared supply / existing ESP-KVM.',
                  '',
                  'BACKUP if port will not fit: two #2880 loose disconnects and a protected cable exit.',
                  'Keep factory buttons / LEDs. Verify actual pads and document connector orientation.',
                  'Button contacts only: no ESP GPIO, control ground or 5 V goes through the sidewall port.'],25)
arr([(935,1250),(935,1310)],'#138467');arr([(1515,1250),(1515,1310)],'#138467')
box(40,1765,1720,215,'8. Hard-freeze recovery: Shelly switches Xbox AC only','#eaf3ff')
pic('roadmap-assets/product-photos/shelly-us-gen4.jpg',60,1820,190,140)
lines(290,1830,['Cut AC → confirm off / wait → restore AC → check fresh power state.', 'If Xbox remains off: default to one POWER pulse, then verify a fresh boot.', 'Keep ESP-KVM, ONE relay supply and network powered independently.'],28)
box(40,2025,1720,155,'9. Where startup power comes from','#ecf8f1')
lines(65,2080,['Restored AC → Xbox PSU → standby-powered button controller → POWER press starts Xbox.',
               'The ESP does NOT power the Xbox or draw accessory power from its standby rail.',
               'Verify standby readiness before pressing; a failed standby supply means stop.'],26)
box(40,2220,1720,100,'Agent commands','#fff4e8')
txt(65,2270,'Codex can press either button. Default recovery uses POWER; qualify Stellar eject-start separately.',25)
txt(40,2350,'Photos: Adafruit / Shelly; ESP reference: Espressif. Sidewall shown in text; not to scale. Solder pads require verification.',21)
im.save(root/'roadmap-assets/xbox-power-relay-setup.png')
