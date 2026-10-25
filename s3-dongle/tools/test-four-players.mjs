// SPDX-License-Identifier: Apache-2.0
// Run in a controller test app; never while playing a game.
import {mkdir,writeFile} from 'node:fs/promises';
const [main,out,...dongles]=process.argv.slice(2);
if(!main||!out||dongles.length!==4)throw new Error('Usage: node test-four-players.mjs MAIN NEW_EVIDENCE_DIR S3_P1 S3_P2 S3_P3 S3_P4');
await mkdir(out,{recursive:false});
const url=new URL('/ws',main);url.protocol=url.protocol==='https:'?'wss:':'ws:';
const ws=new WebSocket(url),sleep=ms=>new Promise(r=>setTimeout(r,ms)),evidence=[];
const neutral=()=>{const r=new Uint8Array(20);r[1]=20;return r;};
const held=button=>{const r=neutral();r[4+button]=255;return r;};
const send=(player,report)=>ws.send(new Uint8Array([8,player,...report]));
const statuses=()=>Promise.all(dongles.map(async origin=>{const r=await fetch(new URL('/status',origin),{signal:AbortSignal.timeout(3000)});if(!r.ok)throw new Error('S3 unavailable');return r.text();}));
async function check(name,expected){
 const actual=await statuses();
 actual.forEach((s,i)=>{
  if(!s.includes('USB host: configured')||!s.includes('XID open: yes'))throw new Error(`Player ${i+1}: USB unconfigured`);
  if(s.split('held: ')[1]?.trim()!==expected[i])throw new Error(`${name}: player ${i+1}, expected ${expected[i]}: ${s}`);
 });
 evidence.push({name,expected,actual,at:new Date().toISOString()});
 console.log(`PASS: ${name}`);return actual;
}
let refresh;
const deadline=setTimeout(()=>{clearInterval(refresh);if(ws.readyState===WebSocket.OPEN)ws.send(new Uint8Array([5]));ws.close();},30000);
try{
 await new Promise((resolve,reject)=>{ws.onopen=()=>ws.send(new Uint8Array([7]));ws.onerror=()=>reject(new Error('Control failed'));ws.onclose=()=>reject(new Error('Closed before takeover'));ws.onmessage=async({data})=>{const b=new Uint8Array(await data.arrayBuffer());if(b[0]===0x83&&b[1]===1)resolve();};});
 ws.send(new Uint8Array([5]));await sleep(200);
 const before=await check('neutral baseline',['-','-','-','-']);
 for(let p=1;p<=4;p++){
  send(p,held(p-1));await sleep(150);
  await check(`player ${p} isolated`,Array.from({length:4},(_,i)=>i===p-1?['A','B','X','Y'][i]:'-'));
  ws.send(new Uint8Array([5]));await sleep(150);
 }
 for(const p of [0,5,255])send(p,held(0));
 const bad=held(0);bad[3]=1;send(2,bad);
 await sleep(150);await check('invalid player and reserved bits rejected',['-','-','-','-']);
 const all=()=>{for(let p=1;p<=4;p++)send(p,held(p-1));};
 refresh=setInterval(all,100);all();await sleep(700);
 await check('four simultaneous distinct states',['A','B','X','Y']);
 clearInterval(refresh);refresh=setInterval(()=>send(1,held(0)),100);
 await sleep(750);await check('independent expiry while player 1 stays active',['A','-','-','-']);
 clearInterval(refresh);all();await sleep(150);
 ws.send(new Uint8Array([5]));await sleep(200);await check('release-all clears every slot',['-','-','-','-']);
 // Legacy 21-byte frames must still address only player 1.
 ws.send(new Uint8Array([8,...held(0)]));await sleep(150);await check('legacy player-1 compatibility',['A','-','-','-']);
 all();await sleep(150);ws.close();await sleep(250);
 const after=await check('disconnect clears every slot',['-','-','-','-']);
 after.forEach((s,i)=>{const count=t=>Number(t.match(/host consumed: (\d+)/)?.[1]);if(!(count(s)>count(before[i])))throw new Error(`Player ${i+1} USB completions stalled`);});
 console.log('PASS: Xbox host consumed reports from all four controllers. Verify console port selection separately.');
}finally{
 clearTimeout(deadline);clearInterval(refresh);
 if(ws.readyState===WebSocket.OPEN)ws.send(new Uint8Array([5]));ws.close();
 await writeFile(`${out}/routing.json`,JSON.stringify(evidence,null,2));
}
