// SPDX-License-Identifier: Apache-2.0
// Bounded B press through P4; observe USB output and automatic stale release.
import {mkdir,writeFile} from "node:fs/promises";
import {join} from "node:path";
const [main, dongle, evidence] = process.argv.slice(2);
if (!main || !dongle) throw new Error('Usage: node test-player1.mjs MAIN_ORIGIN S3_ORIGIN');
const url = new URL('/ws', main);url.protocol = url.protocol === 'https:' ? 'wss:' : 'ws:';
const ws = new WebSocket(url);
const sleep = ms => new Promise(resolve => setTimeout(resolve, ms));
const status = async () => {
  const response = await fetch(new URL('/status', dongle), {signal: AbortSignal.timeout(3000)});
  if (!response.ok) throw new Error(`S3 status HTTP ${response.status}`);
  return response.text();
};
async function frame(name) {
  if (!evidence) return;
  const response = await fetch(new URL('/api/v1/video/frame.jpg', main), {signal: AbortSignal.timeout(4000)});
  if (!response.ok || !response.headers.get('content-type')?.startsWith('image/jpeg'))
    throw new Error('Evidence JPEG unavailable');
  await mkdir(evidence,{recursive:true});
  await writeFile(join(evidence,name),Buffer.from(await response.arrayBuffer()));
}
const number = (text, name) => Number(text.match(new RegExp(`${name}: (\\d+)`))?.[1] ?? NaN);
let refresh;
const timer = setTimeout(() => {if(ws.readyState === WebSocket.OPEN) ws.send(new Uint8Array([5]));ws.close();}, 8000);
try {
  await new Promise((resolve, reject) => {
    ws.onopen = () => ws.send(new Uint8Array([7]));
    ws.onerror = () => reject(new Error('Control WebSocket failed'));
    ws.onclose = () => reject(new Error('Control session closed before takeover'));
    ws.onmessage = async ({data}) => {
      const bytes = new Uint8Array(await data.arrayBuffer());
      if (bytes[0] === 0x83 && bytes[1] === 1) resolve();
    };
  });
  const before = await status();
  ws.send(new Uint8Array([3,0,0x1b,0,0,0,0,0]));
  if (evidence) refresh=setInterval(() => ws.send(new Uint8Array([3,0,0x1b,0,0,0,0,0])),100);
  await sleep(evidence ? 700 : 120);
  const pressed = await status();
  console.log('DURING\n' + pressed);
  await frame('player1-b-pressed.jpg');
  clearInterval(refresh);refresh=undefined;
  // No refresh: the P4's lease must neutralize this without relying on close.
  await sleep(650);
  const released = await status();
  console.log('AFTER EXPIRY\n' + released);
  await frame('player1-released.jpg');
  if (!pressed.includes('held: B')) throw new Error('S3 did not submit the requested B state');
  if (!released.includes('held: -')) throw new Error('S3 did not return to neutral');
  if (!(number(released,'host consumed') > number(before,'host consumed')))
    throw new Error('USB host did not complete reports');
  console.log('PASS: P4 -> Wi-Fi -> S3 USB B state, host completions, and stale release');
} finally {
  clearTimeout(timer);
  clearInterval(refresh);
  if (ws.readyState === WebSocket.OPEN) ws.send(new Uint8Array([5]));
  ws.close();
}
