# SPDX-License-Identifier: Apache-2.0
# Compose an explanatory diagram from unmodified supplier product photographs.
from PIL import Image, ImageDraw, ImageFont
from pathlib import Path
root=Path(__file__).resolve().parent.parent
assets=root/'roadmap-assets/product-photos'
im=Image.new('RGB',(1800,2130),'#f3f6fb');d=ImageDraw.Draw(im)
font='/System/Library/Fonts/Supplemental/Arial.ttf'
def text(x,y,s,n=25,c='#24324a'):d.text((x,y),s,font=ImageFont.truetype(font,n),fill=c)
def box(x,y,w,h,title,c='white'):
 d.rounded_rectangle((x,y,x+w,y+h),18,fill=c,outline='#cad3e0',width=2);text(x+18,y+13,title,29)
def photo(n,x,y,w,h):
 p=assets/f'adafruit-{n}.jpg' if isinstance(n,int) else root/n
 a=Image.open(p).convert('RGBA');a.thumbnail((w,h),Image.Resampling.LANCZOS)
 im.paste(a,(x+(w-a.width)//2,y+(h-a.height)//2),a)
def arrow(pts,c='#62718b',w=5):
 d.line(pts,fill=c,width=w,joint='curve');x,y=pts[-1]; px,py=pts[-2]
 if y>py:d.polygon([(x,y),(x-9,y-16),(x+9,y-16)],fill=c)
 elif x>px:d.polygon([(x,y),(x-16,y-9),(x-16,y+9)],fill=c)
def ls(x,y,lines,n=24,c='#24324a'):
 for i,s in enumerate(lines):text(x,y+i*31,s,n,c)
text(40,25,'The actual proposed parts — and how they connect',43)
text(40,82,'Real supplier photos • Existing ESP board reference render • Proposed, not yet hardware-tested',25)
box(40,135,680,410,'1. Your existing ESP-KVM')
photo('docs/esp32-p4x-function-ev-board-isometric_v1.6.png',55,195,335,300)
ls(400,220,['J1 expansion header','GPIO46 = pin 36','GND = pin 39','','Audio 22 / 53 / 27','stays connected.'])
text(65,500,'Use header labels; this picture is not a pin-location guide.',22)
box(780,135,975,410,'2. Female sockets push onto the ESP header')
photo(3894,805,200,400,250)
ls(1230,215,['Adafruit #3894','Female socket cable','','Builder uses its socket','leads for control + GND.','No ESP-board soldering.'])
text(805,485,'Custom harness assembly is needed; this is not a finished kit.',24,'#a34e18')
arrow([(720,330),(780,330)])
box(40,590,1715,480,'3. Independent 5 V supply + common accessory socket')
photo(276,60,660,320,240);photo(368,415,680,240,180);photo(4046,1080,665,360,240)
ls(65,918,['Adafruit #276','5 V / 2 A supply','2.1 mm center-positive'],23)
ls(400,918,['Adafruit #368','Barrel jack to','screw terminals'],23)
box(690,690,345,235,'Builder-made junction','#eef4ff')
ls(710,750,['White: ESP GPIO46','Black: ESP + supply GND','Red: external +5 V','','Red never goes to ESP.'],23)
arrow([(380,785),(415,785)]);arrow([(655,785),(690,785)]);arrow([(1035,785),(1080,785)])
ls(1460,725,['Adafruit #4046','3-pin PH socket','','This is the common','accessory connection.'],23)
text(695,960,'Supply powers ONE accessory at a time.',25)
text(695,1000,'No motor power from the ESP 3.3 V pin.',25,'#a34e18')
arrow([(1320,1070),(1320,1105),(450,1105),(450,1150)])
arrow([(1320,1105),(1350,1105),(1350,1150)])
box(40,1150,820,580,'A. Plug in the servo finger','#edf5ff')
photo(4326,80,1220,440,320)
ls(535,1260,['Adafruit #4326','TowerPro SG92R','','Matching PH plug','already attached.','Arm included.'],24)
ls(65,1560,['Mount beside Xbox power button; move arm briefly, then retract.','No wire from servo into the Xbox.','Custom mounting bracket / soft tip still needed.','3.3 V control and loaded power behavior need bench validation.'],23)
box(920,1150,835,580,'B. OR plug in the internal relay','#eef8f3')
photo(4336,945,1210,330,230);photo(4409,1350,1210,360,250)
arrow([(1275,1330),(1350,1330)])
ls(950,1470,['Adafruit #4336','PH plug-to-plug cable'],23)
ls(1355,1470,['Adafruit #4409','STEMMA non-latching relay'],23)
ls(945,1560,['COM + NO → two isolated wires → Xbox power-button circuit.','Use a verified inline button adapter, or soldered switch leads.','No ESP supply or GPIO connects directly to Xbox.','Exact Xbox 1.4 solder pads / adapter are NOT yet verified.'],23)
box(40,1770,1715,175,'How swapping works')
ls(65,1825,['The #4046 socket accepts the servo plug OR the #4336 relay cable.','PH is keyed/friction-retained, not the positive-lock SM latch shown in other connector catalogs.','Power accessory OFF before swapping; select the matching servo/relay firmware mode.'],25)
box(40,1980,1715,115,'Purpose: recover from a HARD FREEZE with a HARD POWER CYCLE','#fff3e5')
ls(65,2033,['Shelly cuts/restores Xbox AC. Then press power if needed. KVM and accessory supply stay powered.'],25)
text(40,2102,'Photos: Adafruit product pages (IDs shown). Board: local Espressif reference asset. Sources and BOM in XBOX-POWER-PARTS.md.',19)
im.save(root/'roadmap-assets/xbox-power-real-parts.png')
