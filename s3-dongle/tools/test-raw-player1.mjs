// SPDX-License-Identifier: Apache-2.0
// Run only in Controller Tool: checks raw-state refusal, expiry and disconnect.
const [main,dongle]=process.argv.slice(2);
if(!main||!dongle)throw new Error('Usage: node test-raw-player1.mjs MAIN S3');
const url=new URL('/ws',main);url.protocol=url.protocol==='https:'?'wss:':'ws:';
const ws=new WebSocket(url),sleep=ms=>new Promise(r=>setTimeout(r,ms));
const status=async()=>{const r=await fetch(new URL('/status',dongle),{signal:AbortSignal.timeout(3000)});if(!r.ok)throw new Error('S3 unavailable');return r.text();};
const neutral=new Uint8Array(20);neutral[1]=20;
const send=report=>ws.send(new Uint8Array([8,...report]));
const deadline=setTimeout(()=>{if(ws.readyState===WebSocket.OPEN)ws.send(new Uint8Array([5]));ws.close();},10000);
try{
 await new Promise((resolve,reject)=>{ws.onopen=()=>ws.send(new Uint8Array([7]));ws.onerror=()=>reject(new Error('Control failed'));ws.onclose=()=>reject(new Error('Closed before takeover'));ws.onmessage=async({data})=>{const b=new Uint8Array(await data.arrayBuffer());if(b[0]===0x83&&b[1]===1)resolve();};});
 send(neutral);await sleep(150);
 for(const [name,index,value] of [['output report',0,1],['wrong size header',1,6],['reserved bits',3,1]]){
  const bad=neutral.slice();bad[4]=255;bad[index]=value;send(bad);await sleep(120);
  if(!(await status()).includes('held: -'))throw new Error(`Accepted ${name}`);
 }
 for(const length of [19,21]){const bad=new Uint8Array(length);bad[1]=20;bad[4]=255;send(bad);await sleep(120);if(!(await status()).includes('held: -'))throw new Error(`Accepted ${length}-byte report`);}
 const held=neutral.slice();held[5]=255;held[10]=127;new DataView(held.buffer).setInt16(12,16384,true);
 send(held);await sleep(150);const pressed=await status();if(!pressed.includes('held: B'))throw new Error('Raw state not delivered');
 await sleep(650);if(!(await status()).includes('held: -'))throw new Error('Raw state did not expire');
 send(held);await sleep(150);ws.close();await sleep(250);
 if(!(await status()).includes('held: -'))throw new Error('Disconnect did not release raw state');
 console.log('PASS: 5 malformed reports rejected, full raw state delivered, stale expiry and disconnect release');
}finally{clearTimeout(deadline);if(ws.readyState===WebSocket.OPEN)ws.send(new Uint8Array([5]));ws.close();}
