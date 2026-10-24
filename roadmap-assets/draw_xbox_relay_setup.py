# SPDX-License-Identifier: Apache-2.0
# Layout real source photographs without generating or altering depicted hardware.
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
root=Path(__file__).resolve().parent.parent
im=Image.new('RGB',(2000,2420),'#f3f6fb');d=ImageDraw.Draw(im)
F='/System/Library/Fonts/Supplemental/Arial.ttf'
def t(x,y,s,n=26,c='#24324a'):d.text((x,y),s,font=ImageFont.truetype(F,n),fill=c)
def lines(x,y,ss,n=25,c='#24324a',step=34):
 for i,s in enumerate(ss):t(x,y+i*step,s,n,c)
def box(x,y,w,h,title,bg='white'):
 d.rounded_rectangle((x,y,x+w,y+h),18,fill=bg,outline='#c6d2e0',width=2);t(x+20,y+15,title,31)
def pic(p,x,y,w,h):
 if isinstance(p,int):p=f'roadmap-assets/product-photos/adafruit-{p}.jpg'
 a=Image.open(root/p).convert('RGBA');a.thumbnail((w,h),Image.Resampling.LANCZOS);im.paste(a,(x+(w-a.width)//2,y+(h-a.height)//2),a)
def ar(pts,label=None,c='#456386'):
 d.line(pts,fill=c,width=6,joint='curve');x,y=pts[-1];px,py=pts[-2]
 if x>px:d.polygon([(x,y),(x-17,y-9),(x-17,y+9)],fill=c)
 elif y>py:d.polygon([(x,y),(x-9,y-17),(x+9,y-17)],fill=c)
 elif y<py:d.polygon([(x,y),(x-9,y+17),(x+9,y+17)],fill=c)
t(45,25,'New setup: one relay + a plug-in Xbox button adapter',43)
t(45,83,'For a HARD FREEZE: cut Xbox AC → restore AC → briefly press POWER if still off.',28)
t(45,130,'PROPOSED • No servo • No interchangeable accessory port • Custom Xbox harness still needs building and verification',24,'#a04b15')
box(45,195,580,530,'1. Existing ESP-KVM')
pic('docs/esp32-p4x-function-ev-board-isometric_v1.6.png',65,250,540,285)
lines(70,555,['J1 pin 36 / GPIO46 → relay signal','J1 pin 39 / GND → control-side ground','Female sockets push onto header pins.','Audio GPIO22 / 53 / 27 stays connected.'],24)
box(705,195,580,530,'2. Dedicated control cable')
pic(3894,730,260,530,275)
lines(730,555,['Adafruit #3894 — builder-adapted lead','White socket → ESP GPIO46','Black socket → ESP GND + supply negative','Red socket → external +5 V, NOT ESP'],24)
box(1365,195,590,530,'3. Assembled non-latching relay')
pic(4409,1390,260,540,275)
lines(1390,555,['Adafruit #4409','Cable PH plug → relay input socket','Only COM + NO go toward Xbox.','NC stays unconnected.'],24)
ar([(625,435),(705,435)]);ar([(1285,435),(1365,435)])
# power line separate section
box(45,785,1240,395,'4. Relay power — stays on when Xbox AC is cut','#edf4ff')
pic(276,65,850,365,235);pic(368,475,865,245,180)
lines(760,875,['Regulated 5 V supply (#276 proposed)','→ barrel jack adapter #368','→ red wire of #3894','Negative → black wire + ESP ground'],25)
ar([(430,960),(475,960)]);ar([(720,960),(750,960)])
t(70,1110,'No relay power from Xbox. No need to use ESP 3.3 V or ESP 5 V.',25)
box(1365,785,590,395,'5. Two-wire contact disconnect','#edf8f1')
pic(2880,1390,850,535,215)
lines(1390,1075,['Adafruit #2880 keyed cable pair','COM + NO → two wires → Xbox adapter'],24)
ar([(1660,725),(1660,785)],c='#138467')
# meaningful power reference arrows heading signals
ar([(1050,785),(1050,755),(1330,755),(1330,665),(1365,665)],c='#d43a40')
# Xbox photos and exact topology
box(45,1240,1240,575,'6. Inside Xbox: insert adapter into the existing button cable')
pic('roadmap-assets/x3-power-eject-adapter-closeup.png',70,1320,550,430)
lines(660,1320,['LEFT: historical Xecuter X3 adapter','Photo shows how an inline adapter fits.','Our new passive harness is not built yet.'],24)
box(660,1450,595,95,'Xbox motherboard button header','#eef5ff')
box(660,1575,595,95,'NEW passive pass-through adapter','#eef8f1')
box(660,1700,595,85,'Existing cable → original front buttons','#eef5ff')
ar([(950,1545),(950,1575)]);ar([(950,1670),(950,1700)])
# contact branch routes clear
ar([(1660,1180),(1660,1300),(1310,1300),(1310,1625),(1255,1625)],c='#138467')
t(1390,1250,'Two isolated button wires',24,'#138467')
box(1365,1350,590,465,'Builder does the assembly','#fff4e8')
lines(1390,1420,['You unplug the original button cable,','insert the new adapter, then plug','the original cable into the adapter.','','No cutting/soldering original Xbox wires.','POWER and EJECT keep working.','','Candidate tap: J2G2 circuits 1 + 2.','Connector fit, pin orientation and','continuity MUST be verified first.'],24,step=33)
# AC
box(45,1875,1910,360,'7. Shelly controls ONLY the Xbox AC cord')
pic('roadmap-assets/product-photos/shelly-us-gen4.jpg',430,1950,270,230)
box(85,2010,270,100,'Wall / mains')
box(830,2010,400,100,'Xbox AC power cord')
box(1330,1950,585,230,'Keep outside the switched outlet','#edf4ff')
lines(1355,2020,['ESP-KVM power supply','Relay 5 V power supply','Router / automation host'],26)
ar([(355,2060),(430,2060)]);ar([(700,2060),(830,2060)])
t(760,2165,'Shelly Plug US Gen4 / S4PL-00116US',25)
box(45,2275,1910,100,'Isolation and confirmation','#fff4e8')
t(70,2325,'No new ESP-to-Xbox ground wire. Relay contacts only. Metering must be calibrated; ambiguous power means do not press.',25)
t(45,2390,'Photos: Adafruit, Shelly; board reference: Espressif; historical adapter: X3 guide p.12. Not to scale. Details: XBOX-POWER-RELAY-SETUP.md',19)
im.save(root/'roadmap-assets/xbox-power-relay-setup.png')
