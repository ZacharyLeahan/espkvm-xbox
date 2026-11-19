// SPDX-License-Identifier: Apache-2.0
import assert from 'node:assert/strict';
import {createRequire} from 'node:module';
import {createServer} from 'node:http';
import {once} from 'node:events';
import {mkdtemp, readFile, rm} from 'node:fs/promises';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {validate, report, run} from './run_xbox_scenario.mjs';
const require = createRequire(new URL('../web/package.json', import.meta.url));
const {WebSocketServer} = require('ws');
const base = {name: 'fixture', start_state: 'mock controller screen', steps: [
  {name: 'input', buttons: ['a', 'lt', 'rt', 'start'], hold_ms: 150},
  {name: 'evidence', expect: {signal: true, width: 720}, screenshot: true},
]};
validate(base);
for (const step of [
  {name: 'bad', buttons: ['power'], hold_ms: 100},
  {name: 'bad', buttons: ['a'], hold_ms: 9000},
  {name: 'bad', buttons: ['a'], player: 5, hold_ms: 100},
  {name: 'bad', expect: {signal: 'true'}},
  {name: 'bad', executable: 'arbitrary command'},
]) assert.throws(() => validate({...base, steps: [step]}));
assert.equal(report(['start'])[2], 16);
assert.equal(report(['a'])[4], 255);
assert.deepEqual([...report(['lt', 'rt']).slice(10, 12)], [255, 255]);

const root = await mkdtemp(join(tmpdir(), 'xbox-runner-'));
try {
  for (const mode of ['complete', 'mismatch', 'disconnect', 'ownership', 'deadline', 'no-ack', 'bad-jpeg']) {
    const packets = []; let requests = 0;
    const server = createServer((req, res) => {
      requests++;
      if (req.url === '/api/v1/video/status') {
        res.writeHead(200, {'content-type': 'application/json'});
        res.end(JSON.stringify({signal: true, width: mode === 'mismatch' ? 1280 : 720, height: 480, frameAgeMs: 7}));
      } else if (req.url === '/api/v1/video/frame.jpg') {
        res.writeHead(200, {'content-type': 'image/jpeg'});
        res.end(mode === 'bad-jpeg' ? Buffer.from('bad') : Buffer.from([255, 216, 255, 217]));
      } else { res.writeHead(404); res.end(); }
    });
    const wss = new WebSocketServer({server});
    wss.on('connection', socket => socket.on('message', bytes => {
      packets.push([...bytes]);
      if (bytes[0] === 7 && mode !== 'no-ack') socket.send(Buffer.from([0x83, 1]));
      if (bytes[0] === 8 && mode === 'disconnect') socket.close();
      if (bytes[0] === 8 && mode === 'ownership') socket.send(Buffer.from([0x83, 0]));
    }));
    server.listen(0, '127.0.0.1'); await once(server, 'listening');
    const output = join(root, mode);
    try {
      const result = await run({origin: `http://127.0.0.1:${server.address().port}`, plan: base,
        output, runs: 2, timeoutMs: ['deadline', 'no-ack'].includes(mode) ? 80 : 3000,
        buildId: 'mock-title', kvmBuildId: 'mock-kvm'});
      const events = (await readFile(join(output, 'events.jsonl'), 'utf8')).trim().split('\n').map(JSON.parse);
      assert.equal(result.outcome, mode === 'complete' ? 'INCONCLUSIVE' : mode === 'mismatch' ? 'FAILED_CHECK' : 'NEEDS_RECOVERY');
      if (mode === 'complete') {
        assert.equal(result.results.length, 2);
        assert.equal(events.filter(e => e.event === 'screenshot').length, 2);
        assert.ok(events.some(e => e.event === 'video' && e.video.frameAgeMs === 7));
      } else assert.equal(events.filter(e => e.event === 'run_started').length, mode === 'no-ack' ? 0 : 1);
      if (['complete', 'mismatch', 'deadline', 'bad-jpeg'].includes(mode)) assert.equal(packets.at(-1)[0], 5, 'release must follow input on completion/failure');
      if (mode === 'no-ack') assert.ok(packets.every(p => p[0] === 7), 'no input without control grant');
      if (mode === 'disconnect') assert.equal(requests, 0, 'do not continue after disconnect');
      assert.equal(packets.filter(p => p[0] === 7).length, 1, 'never reconnect/retry takeover');
      await assert.rejects(run({origin: '', plan: base, output}), /EEXIST/);
      console.log(`PASS: ${mode}`);
    } finally {
      for (const socket of wss.clients) socket.terminate();
      await new Promise(resolve => wss.close(resolve));
      await new Promise(resolve => server.close(resolve));
    }
  }
} finally { await rm(root, {recursive: true, force: true}); }
