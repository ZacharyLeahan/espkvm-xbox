// SPDX-License-Identifier: Apache-2.0
// Run only with Xbox Controller Tool's Control Test Page open on the console.
import {mkdir,writeFile,appendFile} from 'node:fs/promises';
import {join} from 'node:path';
const [main,dongle,out,slot]=process.argv.slice(2);
const player=Number(slot||1);
if(!Number.isInteger(player)||player<1||player>4)throw new Error("Player must be 1..4");
if(!main||!dongle||!out)throw new Error('Usage: node sweep-player1.mjs MAIN S3 NEW_EVIDENCE_DIR [PLAYER]');
await mkdir(out,{recursive:false});
const neutral=()=>{const b=new Uint8Array(20);b[1]=20;return b;};
const cases=[];
function add(name,modify=()=>{},kind='numeric'){const report=neutral();modify(report);cases.push({name,kind,report:[...report]});}
add('neutral-start');
const digital=['up','down','left','right','start','back','left-thumb','right-thumb'];
for(let bit=0;bit<8;bit++){add(digital[bit],r=>r[2]=1<<bit,'digital');add(`release-${digital[bit]}`);}
const pressure=['a','b','x','y','black','white'];
for(let i=0;i<6;i++)for(const value of [1,127,255])add(`${pressure[i]}-${value}`,r=>r[4+i]=value);
add('release-buttons');
for(let i=0;i<2;i++)for(const value of [0,1,32,64,127,192,254,255])add(`${i?'right':'left'}-trigger-${value}`,r=>r[10+i]=value);
add('release-triggers');
const axes=['left-x','left-y','right-x','right-y'];
for(let axis=0;axis<4;axis++)for(const value of [-32768,-24576,-16384,-8192,-1,0,1,8192,16384,24576,32767])
 add(`${axes[axis]}-${value}`,r=>new DataView(r.buffer).setInt16(12+axis*2,value,true));
add('release-axes');
for(let stick=0;stick<2;stick++)for(const radius of [16384,32767])for(let step=0;step<16;step++){
 const angle=step*Math.PI/8;
 add(`${stick?'right':'left'}-circle-${radius}-${step}`,r=>{
  const view=new DataView(r.buffer);view.setInt16(12+stick*4,Math.round(radius*Math.cos(angle)),true);view.setInt16(14+stick*4,Math.round(radius*Math.sin(angle)),true);
 });
}
add('both-triggers-and-diagonal-sticks',r=>{r[10]=64;r[11]=192;const v=new DataView(r.buffer);[8192,-16384,-24576,32767].forEach((n,i)=>v.setInt16(12+i*2,n,true));});
add('neutral-end');
await writeFile(join(out,'plan.json'),JSON.stringify(cases,null,2));
const url=new URL('/ws',main);url.protocol=url.protocol==='https:'?'wss:':'ws:';
const ws=new WebSocket(url);let current=neutral(),refresh;
const viewerAbort=new AbortController();
const sleep=ms=>new Promise(r=>setTimeout(r,ms));
const send=()=>{if(ws.readyState===WebSocket.OPEN)ws.send(new Uint8Array([8,player,...current]));};
const get=async(origin,path)=>{const r=await fetch(new URL(path,origin),{signal:AbortSignal.timeout(5000)});if(!r.ok)throw new Error(`${path}: HTTP ${r.status}`);return r;};
const deadline=setTimeout(()=>{clearInterval(refresh);if(ws.readyState===WebSocket.OPEN)ws.send(new Uint8Array([5]));ws.close();viewerAbort.abort();},480000);
try{
 await new Promise((resolve,reject)=>{ws.onopen=()=>ws.send(new Uint8Array([7]));ws.onerror=()=>reject(new Error('WebSocket failed'));ws.onclose=()=>reject(new Error('Disconnected before takeover'));ws.onmessage=async({data})=>{const b=new Uint8Array(await data.arrayBuffer());if(b[0]===0x83&&b[1]===1)resolve();};});
 const viewer=await fetch(new URL('/stream',main),{signal:viewerAbort.signal});
 if(!viewer.ok)throw new Error('Live viewer failed');
 (async()=>{const reader=viewer.body.getReader();while(!(await reader.read()).done){}})().catch(e=>{if(!viewerAbort.signal.aborted)console.error('Viewer stopped:',e.message);});
 refresh=setInterval(send,100);send();await sleep(1000);
 let previousConsumed=0;
 for(let index=0;index<cases.length;index++){
  if(ws.readyState!==WebSocket.OPEN)throw new Error("Control session disconnected during sweep");
  const item=cases[index];current=new Uint8Array(item.report);send();await sleep(650);
  const status=await(await get(dongle,'/status')).text();
  const consumed=Number(status.match(/host consumed: (\d+)/)?.[1]);
  if(!(consumed>previousConsumed))throw new Error(`USB host completions stalled at ${item.name}`);
  previousConsumed=consumed;
  const frame=await get(main,'/api/v1/video/frame.jpg');
  if(!frame.headers.get('content-type')?.startsWith('image/jpeg'))throw new Error('Not a JPEG');
  const file=`${String(index).padStart(3,'0')}-${item.name}.jpg`;
  await writeFile(join(out,file),Buffer.from(await frame.arrayBuffer()));
  await appendFile(join(out,'observations.jsonl'),JSON.stringify({index,...item,file,status,at:new Date().toISOString()})+'\n');
  if(index%10===0||index===cases.length-1)console.log(`${index+1}/${cases.length}: ${item.name}; USB consumed ${consumed}`);
 }
 current=neutral();send();clearInterval(refresh);refresh=undefined;await sleep(750);
 const status=await(await get(dongle,'/status')).text();
 if(!status.includes('held: -'))throw new Error('Final neutral release failed');
 await writeFile(join(out,'final-status.txt'),status);
 console.log(`CAPTURE COMPLETE: ${cases.length} states; compare Xbox display values before claiming PASS`);
}finally{clearTimeout(deadline);clearInterval(refresh);if(ws.readyState===WebSocket.OPEN)ws.send(new Uint8Array([5]));ws.close();viewerAbort.abort();}
